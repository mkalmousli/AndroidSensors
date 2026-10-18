// Verifies the poller emits live store data, throttles on-change sensors and completes flushes.
#include <unistd.h>

#include <cstdlib>

#include <chrono>
#include <cstdio>
#include <mutex>
#include <thread>

#include "poller.h"
#include "sensor_table.h"

using namespace andros;

static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); ++failures; } } while (0)

int main() {
    const char* path = getenv("ANDROS_STORE");
    Store* store = Store::Open(path ? path : kDefaultPath, true);
    if (!store) { puts("skip: cannot create /data/vendor/andros/sensors.store"); return 0; }
    std::mutex mu;
    std::vector<Event> got;
    Poller poller([&](const std::vector<Event>& e) {
        std::lock_guard<std::mutex> l(mu);
        got.insert(got.end(), e.begin(), e.end());
    });
    poller.Start();
    int accel = -1, light = -1;
    for (size_t h = 1; h <= HalSensors().size(); ++h) {
        std::string n = kSensors[HandleToIndex(int32_t(h))].name;
        if (n == "accelerometer") accel = int(h);
        if (n == "light") light = int(h);
    }
    double v[3] = {1, 2, 3};
    store->Write(HandleToIndex(accel), v, 3);
    poller.SetPeriod(accel, 5000000);
    poller.Activate(accel, true);
    poller.Activate(light, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    poller.Flush(accel);
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    int sigmo = -1, steps = -1;
    for (size_t h = 1; h <= HalSensors().size(); ++h) {
        std::string n = kSensors[HandleToIndex(int32_t(h))].name;
        if (n == "significantMotion") sigmo = int(h);
        if (n == "stepCounter") steps = int(h);
    }
    double zero[1] = {0};
    store->Write(HandleToIndex(sigmo), zero, 1);  // the store file persists between runs
    store->Write(HandleToIndex(steps), zero, 1);
    poller.Activate(sigmo, true);
    poller.Activate(steps, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    double one[1] = {1}, step[1] = {42};
    store->Write(HandleToIndex(sigmo), one, 1);  // trigger fires on the counter change
    store->Write(HandleToIndex(steps), step, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    double two[1] = {2};
    store->Write(HandleToIndex(sigmo), two, 1);  // one-shot: disabled after the first event
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    std::lock_guard<std::mutex> l(mu);
    int sig_n = 0, step_n = 0;
    uint64_t last_step = 0;
    for (const Event& e : got) {
        if (e.sensorHandle == sigmo) { ++sig_n; CHECK(e.u.data[0] == 1.0f); }
        if (e.sensorHandle == steps) { ++step_n; last_step = e.u.stepCount; }
    }
    CHECK(sig_n == 1);   // not at activation, once on trigger, never again
    CHECK(last_step == 42);
    CHECK(step_n == 2);  // on-change: once at activation (0 steps) and once for 42
    int accel_n = 0, light_n = 0, flushes = 0;
    for (const Event& e : got) {
        if (e.sensorType == android::hardware::sensors::V2_1::SensorType::META_DATA) {
            ++flushes;  // flush-complete carries the flushed sensor's handle
            continue;
        }
        if (e.sensorHandle == accel) { ++accel_n; CHECK(e.u.data[1] == 2.0f); }
        if (e.sensorHandle == light) ++light_n;
    }
    CHECK(accel_n >= 5);
    CHECK(light_n == 1);  // on-change: once at activation, value never changed
    CHECK(flushes == 1);
    puts(failures ? "FAILED" : "poller tests passed");
    return failures ? 1 : 0;
}
