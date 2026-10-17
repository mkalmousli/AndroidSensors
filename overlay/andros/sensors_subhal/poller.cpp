#include "poller.h"

#include <algorithm>
#include <cstdlib>

#include "sensor_table.h"

namespace andros {

using ::android::hardware::sensors::V2_1::SensorType;
namespace V1_0 = ::android::hardware::sensors::V1_0;

constexpr int64_t kIdleWaitNs = 50000000;
static const char* StorePath() {
    const char* env = getenv("ANDROS_STORE");  // overridable for host tests
    return env ? env : kDefaultPath;
}

Poller::~Poller() {
    stop_ = true;
    cv_.notify_all();
    if (thread_.joinable()) thread_.join();
}

void Poller::Start() { thread_ = std::thread([this] { Loop(); }); }

void Poller::Activate(int32_t handle, bool on) {
    std::lock_guard<std::mutex> lock(mu_);
    State& st = states_[handle];
    st.on = on;
    st.has_last = false;  // on-change sensors report once on activation
    st.next_ns = 0;
    cv_.notify_all();
}

void Poller::SetPeriod(int32_t handle, int64_t period_ns) {
    std::lock_guard<std::mutex> lock(mu_);
    states_[handle].period_ns = std::max<int64_t>(period_ns, 1000000);
}

void Poller::Flush(int32_t handle) {
    std::lock_guard<std::mutex> lock(mu_);
    states_[handle].flush = true;
    cv_.notify_all();
}

void Poller::Loop() {
    while (!stop_) {
        std::vector<Event> batch;
        std::unique_lock<std::mutex> lock(mu_);
        if (!store_) store_ = Store::Open(StorePath(), /*writer=*/false);
        int64_t now = NowNs(), wake = now + kIdleWaitNs;
        for (auto& [handle, st] : states_) {
            if (!st.on) continue;
            if (st.flush) {
                st.flush = false;
                Emit(handle, st, &batch);  // flush delivers the current value first
                batch.push_back(FlushComplete(handle));
            }
            if (st.next_ns <= now) {
                Emit(handle, st, &batch);
                st.next_ns = now + st.period_ns;
            }
            wake = std::min(wake, st.next_ns);
        }
        if (batch.empty()) {
            cv_.wait_for(lock, std::chrono::nanoseconds(std::max<int64_t>(wake - now, 1000000)));
            continue;
        }
        lock.unlock();
        sink_(batch);
    }
}

}  // namespace andros
