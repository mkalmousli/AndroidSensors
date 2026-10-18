// NMEA formatting and the satellite generator (pure C++, no Android headers).
#include <cstdio>
#include <cstring>

#include "nmea.h"
#include "sats.h"

using namespace andros;

static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); ++failures; } } while (0)

int main() {
    // checksum of the NMEA spec's example body
    CHECK(NmeaWrap("GPGLL,4916.45,N,12311.12,W,225444,A") == "$GPGLL,4916.45,N,12311.12,W,225444,A*31\r\n");
    Fix f;
    f.lat = 48.1173; f.lon = 11.5167; f.alt = 545.4; f.sats = 8; f.speed_mps = 11.5;
    f.bearing = 84.4; f.unix_ms = 1100000000000LL;  // 2004-11-09 11:33:20 UTC
    std::string gga = NmeaGga(f), rmc = NmeaRmc(f);
    CHECK(gga.rfind("$GPGGA,113320.00,4807.0380,N,01131.0020,E,1,8,1.0,545.4,M,0.0,M,,*", 0) == 0);
    CHECK(rmc.rfind("$GPRMC,113320.00,A,4807.0380,N,01131.0020,E,22.35,84.4,091104,,,A*", 0) == 0);
    Fix s; s.lat = -33.5; s.lon = -70.25;
    CHECK(NmeaGga(s).find(",S,") != std::string::npos && NmeaGga(s).find(",W,") != std::string::npos);
    CHECK(Satellites(12, 0).size() == 12 && Satellites(100, 0).size() == 64);
    for (const Sat& x : Satellites(12, 123456)) CHECK(x.azimuth_deg >= 0 && x.azimuth_deg < 360);
    puts(failures ? "FAILED" : "nmea tests passed");
    return failures ? 1 : 0;
}
