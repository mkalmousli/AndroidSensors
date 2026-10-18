// Route simulation: walks a polyline at a speed and derives every correlated signal
// (position, bearing, steps, gait acceleration, barometric pressure, heart rate, activity).
#pragma once
#include <string>
#include <vector>

namespace andros {

struct Pt { double lat, lon, alt; };

struct Motion {
    double lat = 0, lon = 0, alt = 0, speed = 0, bearing = 0;
    double steps = 0, cadence_hz = 0, pressure_hpa = 1013.25, heart_bpm = 65;
    double floors = 0, distance_m = 0;
    std::string activity = "still";
};

double HaversineM(const Pt& a, const Pt& b);
double BearingDeg(const Pt& a, const Pt& b);
double PressureAt(double alt_m);
const char* ActivityFor(double speed_mps);

class Route {
  public:
    Route(std::vector<Pt> pts, double speed_mps, bool loop);
    void Advance(double dt_s);       // moves along the route, updates Now()
    const Motion& Now() const { return m_; }
    bool Done() const { return done_; }

  private:
    void Place();
    std::vector<Pt> pts_;
    double speed_, along_ = 0, total_ = 0, climbed_ = 0;
    bool loop_, done_ = false;
    Motion m_;
};

}  // namespace andros
