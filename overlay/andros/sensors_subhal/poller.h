// Background thread that turns store samples into sensor events at the
// requested rates, honouring activate/batch/flush semantics.
#pragma once
#include <android/hardware/sensors/2.1/types.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "store.h"

namespace andros {

using ::android::hardware::sensors::V2_1::Event;

// Meta event telling SensorService a flush() finished.
Event FlushComplete(int32_t handle);

class Poller {
  public:
    using Sink = std::function<void(const std::vector<Event>&)>;
    Poller(Sink sink) : sink_(std::move(sink)) {}
    ~Poller();

    void Start();
    void Activate(int32_t handle, bool on);
    void SetPeriod(int32_t handle, int64_t period_ns);
    void Flush(int32_t handle);

  private:
    struct State {
        bool on = false;
        int64_t period_ns = 200000000;
        int64_t next_ns = 0;
        bool has_last = false;
        Sample last;
        bool flush = false;
    };
    void Loop();
    bool Emit(int32_t handle, State& st, std::vector<Event>* out);

    Sink sink_;
    std::mutex mu_;
    std::condition_variable cv_;
    std::map<int32_t, State> states_;
    Store* store_ = nullptr;
    std::thread thread_;
    std::atomic<bool> stop_{false};
};

}  // namespace andros
