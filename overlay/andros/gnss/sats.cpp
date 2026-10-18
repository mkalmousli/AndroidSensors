#include "sats.h"

#include <cmath>

namespace andros {

std::vector<Sat> Satellites(int count, int64_t t_ms) {
    static const int kConstellations[4] = {1, 6, 3, 5};
    std::vector<Sat> out;
    for (int i = 0; i < count && i < 64; ++i) {
        Sat s;
        s.constellation = kConstellations[i % 4];
        s.svid = 1 + (i / 4) * 3 + (i % 4);
        s.azimuth_deg = float(std::fmod(i * 47.0 + t_ms / 1000.0 * 0.05, 360.0));
        s.elevation_deg = float(15 + (i * 17) % 65);
        s.cn0_dbhz = float(28 + (i * 5) % 17);
        out.push_back(s);
    }
    return out;
}

}  // namespace andros
