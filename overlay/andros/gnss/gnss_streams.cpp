// Raw NMEA and satellite-status streams, derived from the same fix as gnssLocationCb.
#include "andros_gnss.h"
#include "nmea.h"
#include "sats.h"

namespace andros {

namespace {
constexpr int kSatCount = 12;
}

void AndrosGnss::EmitStreams(gnss::IGnssCallback& cb, const gnss::GnssLocation& loc) {
    if (nmea_) {
        Fix f;
        f.lat = loc.latitudeDegrees; f.lon = loc.longitudeDegrees; f.alt = loc.altitudeMeters;
        f.speed_mps = loc.speedMetersPerSec; f.bearing = loc.bearingDegrees;
        f.sats = kSatCount; f.unix_ms = loc.timestampMillis;
        cb.gnssNmeaCb(loc.timestampMillis, NmeaGga(f));
        cb.gnssNmeaCb(loc.timestampMillis, NmeaRmc(f));
    }
    if (sv_) {
        std::vector<gnss::IGnssCallback::GnssSvInfo> list;
        for (const Sat& s : Satellites(kSatCount, loc.timestampMillis)) {
            gnss::IGnssCallback::GnssSvInfo i{};
            i.svid = s.svid;
            i.constellation = static_cast<gnss::GnssConstellationType>(s.constellation);
            i.cN0Dbhz = s.cn0_dbhz;
            i.elevationDegrees = s.elevation_deg;
            i.azimuthDegrees = s.azimuth_deg;
            i.svFlag = 1 | 2 | 4;  // ephemeris, almanac, used in fix
            list.push_back(i);
        }
        cb.gnssSvStatusCb(list);
    }
}

}  // namespace andros
