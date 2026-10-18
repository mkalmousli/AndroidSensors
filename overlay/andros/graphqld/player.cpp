#include "player.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <mutex>
#include <thread>

#include "resolvers.h"

namespace andros {

namespace {
std::mutex g_mu;
std::atomic<int> g_gen{0};  // bumped to cancel the running player
Motion g_motion;

void Put(Store* s, const char* name, std::initializer_list<double> v) {
    const int i = FindSensor(name);
    if (i < 0) return;
    double buf[kMaxValues] = {};
    int n = 0;
    for (double x : v) buf[n++] = x;
    s->Write(i, buf, n);
}

void Publish(Store* s, const Motion& m, double t) {
    Put(s, "location", {m.lat, m.lon, m.alt, m.speed, m.bearing, 4.0});
    Put(s, "stepCounter", {std::floor(m.steps)});
    Put(s, "pressure", {m.pressure_hpa});
    Put(s, "heartRate", {m.heart_bpm});
    const double bounce = m.cadence_hz > 0 ? 3.0 * std::sin(2 * 3.14159265 * m.cadence_hz * t) : 0;
    Put(s, "accelerometer", {0.3 * bounce, 0.2 * bounce, 9.81 + bounce});
    std::lock_guard<std::mutex> lock(g_mu);
    g_motion = m;
}
}  // namespace

void PlayRoute(Store* store, std::vector<Pt> pts, double speed, bool loop) {
    const int gen = ++g_gen;
    std::thread([=] {
        Route route(pts, speed, loop);
        double t = 0;
        while (g_gen == gen) {
            route.Advance(0.1);
            t += 0.1;
            Publish(store, route.Now(), t);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}

void StopRoute() { ++g_gen; }

Motion CurrentMotion() {
    std::lock_guard<std::mutex> lock(g_mu);
    return g_motion;
}

}  // namespace andros
