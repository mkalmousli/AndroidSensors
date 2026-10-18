#include "route.h"

#include <algorithm>
#include <cmath>

namespace andros {

namespace {
constexpr double kPi = 3.14159265358979323846;
double Rad(double d) { return d * kPi / 180.0; }
}  // namespace

double HaversineM(const Pt& a, const Pt& b) {
    const double dlat = Rad(b.lat - a.lat), dlon = Rad(b.lon - a.lon);
    const double h = std::sin(dlat / 2) * std::sin(dlat / 2) +
                     std::cos(Rad(a.lat)) * std::cos(Rad(b.lat)) * std::sin(dlon / 2) * std::sin(dlon / 2);
    return 2 * 6371000.0 * std::asin(std::sqrt(h));
}

double BearingDeg(const Pt& a, const Pt& b) {
    const double dl = Rad(b.lon - a.lon);
    const double y = std::sin(dl) * std::cos(Rad(b.lat));
    const double x = std::cos(Rad(a.lat)) * std::sin(Rad(b.lat)) -
                     std::sin(Rad(a.lat)) * std::cos(Rad(b.lat)) * std::cos(dl);
    return std::fmod(std::atan2(y, x) * 180.0 / kPi + 360.0, 360.0);
}

double PressureAt(double alt_m) { return 1013.25 * std::pow(1.0 - alt_m / 44330.0, 5.255); }

const char* ActivityFor(double v) {
    if (v < 0.3) return "still";
    if (v < 2.2) return "walking";
    if (v < 4.0) return "running";
    if (v < 9.0) return "cycling";
    return "vehicle";
}

Route::Route(std::vector<Pt> pts, double speed_mps, bool loop)
    : pts_(std::move(pts)), speed_(speed_mps), loop_(loop) {
    for (size_t i = 1; i < pts_.size(); ++i) total_ += HaversineM(pts_[i - 1], pts_[i]);
    done_ = pts_.empty();
    if (!done_) Place();
}

void Route::Place() {  // position + bearing at distance `along_`
    double left = along_;
    for (size_t i = 1; i < pts_.size(); ++i) {
        const double seg = HaversineM(pts_[i - 1], pts_[i]);
        if (left <= seg || i + 1 == pts_.size()) {
            const double f = seg > 0 ? std::min(1.0, left / seg) : 0;
            const Pt &a = pts_[i - 1], &b = pts_[i];
            const double alt = a.alt + (b.alt - a.alt) * f;
            if (m_.alt != 0 && alt > m_.alt) climbed_ += alt - m_.alt;
            m_.lat = a.lat + (b.lat - a.lat) * f;
            m_.lon = a.lon + (b.lon - a.lon) * f;
            m_.alt = alt;
            m_.bearing = BearingDeg(a, b);
            return;
        }
        left -= seg;
    }
    m_.lat = pts_[0].lat; m_.lon = pts_[0].lon; m_.alt = pts_[0].alt;  // single point
}

void Route::Advance(double dt) {
    if (done_) { m_.speed = 0; m_.cadence_hz = 0; m_.activity = "still"; return; }
    along_ += speed_ * dt;
    if (along_ >= total_) {
        if (loop_ && total_ > 0) along_ = std::fmod(along_, total_);
        else { along_ = total_; done_ = true; }
    }
    Place();
    m_.speed = done_ ? 0 : speed_;
    m_.activity = ActivityFor(m_.speed);
    const bool on_foot = m_.speed >= 0.3 && m_.speed < 4.0;
    m_.cadence_hz = on_foot ? std::min(3.2, 1.2 + 0.5 * m_.speed) : 0;
    m_.steps += m_.cadence_hz * dt;
    m_.distance_m = along_;
    m_.pressure_hpa = PressureAt(m_.alt);
    m_.floors = std::floor(climbed_ / 3.0);
    m_.heart_bpm = m_.speed < 0.3 ? 65 : std::min(175.0, 80 + 14 * m_.speed);
}

}  // namespace andros
