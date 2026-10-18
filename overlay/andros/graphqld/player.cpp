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

void Bump(Store* s, const char* name) {  // fire a detector: increment its counter
    const int i = FindSensor(name);
    Sample cur;
    if (i < 0 || !s->Read(i, &cur)) return;
    cur.v[0] += 1;
    s->Write(i, cur.v, 1);
}

int ActivityCode(const std::string& a) {
    static const char* kNames[] = {"still", "walking", "running", "cycling", "vehicle"};
    for (int i = 0; i < 5; ++i) if (a == kNames[i]) return i;
    return 0;
}

// Derived signals: activity, floors and the motion/step detectors.
void PublishDerived(Store* s, const Motion& m) {
    static double last_steps = 0;
    static int last_code = 0;
    const int code = ActivityCode(m.activity);
    if (code != last_code) {
        Put(s, "activity", {double(code), 100});
        if (code == 0) Bump(s, "stationaryDetect");
        else if (last_code == 0) { Bump(s, "motionDetect"); Bump(s, "significantMotion"); }
        last_code = code;
    }
    for (; std::floor(last_steps) < std::floor(m.steps); last_steps += 1) Bump(s, "stepDetector");
    Put(s, "floors", {m.floors});
}

void Publish(Store* s, const Motion& m, double t) {
    PublishDerived(s, m);
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
