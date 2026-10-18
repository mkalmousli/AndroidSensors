// Deterministic, slowly moving satellite constellation for GnssSvStatus.
#pragma once
#include <cstdint>
#include <vector>

namespace andros {

struct Sat {
    int constellation;  // GnssConstellationType: 1 GPS, 3 GLONASS, 5 BEIDOU, 6 GALILEO
    int svid;
    float cn0_dbhz, elevation_deg, azimuth_deg;
};

std::vector<Sat> Satellites(int count, int64_t t_ms);

}  // namespace andros
