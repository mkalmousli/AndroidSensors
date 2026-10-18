// NMEA 0183 sentences for map and fitness apps that read raw GNSS output.
#pragma once
#include <cstdint>
#include <string>

namespace andros {

struct Fix {
    double lat = 0, lon = 0, alt = 0;
    double speed_mps = 0, bearing = 0, hdop = 1.0;
    int sats = 0;
    int64_t unix_ms = 0;
};

std::string NmeaWrap(const std::string& body);  // "$" + body + "*HH\r\n"
std::string NmeaGga(const Fix& f);
std::string NmeaRmc(const Fix& f);

}  // namespace andros
