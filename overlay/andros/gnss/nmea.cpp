#include "nmea.h"

#include <cmath>
#include <cstdio>
#include <ctime>

namespace andros {

namespace {
std::string Coord(double deg, bool lat) {
    const double a = std::fabs(deg);
    const int d = int(a);
    char buf[24];
    snprintf(buf, sizeof buf, lat ? "%02d%07.4f" : "%03d%07.4f", d, (a - d) * 60.0);
    return buf;
}

std::string Utc(int64_t ms, const char* fmt, bool frac) {
    time_t t = time_t(ms / 1000);
    struct tm tm;
    gmtime_r(&t, &tm);
    char buf[24];
    strftime(buf, sizeof buf, fmt, &tm);
    return frac ? std::string(buf) + ".00" : buf;
}

std::string Num(const char* fmt, double v) {
    char buf[32];
    snprintf(buf, sizeof buf, fmt, v);
    return buf;
}
}  // namespace

std::string NmeaWrap(const std::string& body) {
    unsigned char sum = 0;
    for (char c : body) sum ^= static_cast<unsigned char>(c);
    char tail[16];
    snprintf(tail, sizeof tail, "*%02X\r\n", sum);
    return "$" + body + tail;
}

std::string NmeaGga(const Fix& f) {
    return NmeaWrap("GPGGA," + Utc(f.unix_ms, "%H%M%S", true) + "," + Coord(f.lat, true) +
                    (f.lat >= 0 ? ",N," : ",S,") + Coord(f.lon, false) + (f.lon >= 0 ? ",E," : ",W,") +
                    "1," + std::to_string(f.sats) + "," + Num("%.1f", f.hdop) + "," +
                    Num("%.1f", f.alt) + ",M,0.0,M,,");
}

std::string NmeaRmc(const Fix& f) {
    return NmeaWrap("GPRMC," + Utc(f.unix_ms, "%H%M%S", true) + ",A," + Coord(f.lat, true) +
                    (f.lat >= 0 ? ",N," : ",S,") + Coord(f.lon, false) + (f.lon >= 0 ? ",E," : ",W,") +
                    Num("%.2f", f.speed_mps * 1.943844) + "," + Num("%.1f", f.bearing) + "," +
                    Utc(f.unix_ms, "%d%m%y", false) + ",,,A");
}

}  // namespace andros
