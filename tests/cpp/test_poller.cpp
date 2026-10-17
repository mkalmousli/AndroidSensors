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
    std::lock_guard<std::mutex> l(mu);
    int accel_n = 0, light_n = 0, flushes = 0;
    for (const Event& e : got) {
        if (e.sensorHandle == accel) { ++accel_n; CHECK(e.u.data[1] == 2.0f); }
        if (e.sensorHandle == light) ++light_n;
        if (e.sensorHandle == 0) ++flushes;
    }
    CHECK(accel_n >= 5);
    CHECK(light_n == 1);  // on-change: once at activation, value never changed
    CHECK(flushes == 1);
    puts(failures ? "FAILED" : "poller tests passed");
    return failures ? 1 : 0;
}
