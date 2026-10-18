#include "andros_gnss.h"
#include "device_gen.h"

#include <chrono>

namespace andros {

using gnss::GnssLocation;

namespace {
constexpr int kLocationIndex = [] {
    for (int i = 0; i < kSensorCount; ++i)
        if (kSensors[i].hal == Hal::kGnss) return i;
    return -1;
}();
constexpr int kFlags = GnssLocation::HAS_LAT_LONG | GnssLocation::HAS_ALTITUDE |
                       GnssLocation::HAS_SPEED | GnssLocation::HAS_BEARING |
                       GnssLocation::HAS_HORIZONTAL_ACCURACY;
}  // namespace

ScopedAStatus AndrosGnss::setCallback(const std::shared_ptr<gnss::IGnssCallback>& cb) {
    std::lock_guard<std::mutex> lock(mu_);
    cb_ = cb;
    cb->gnssSetCapabilitiesCb(0);
    cb->gnssSetSystemInfoCb({2023, gen::kGnssName});
    return ScopedAStatus::ok();
}

ScopedAStatus AndrosGnss::close() {
    StopFixes();
    std::lock_guard<std::mutex> lock(mu_);
    cb_.reset();
    return ScopedAStatus::ok();
}

ScopedAStatus AndrosGnss::setPositionMode(const PositionModeOptions& o) {
    interval_ms_ = o.minIntervalMs > 0 ? o.minIntervalMs : 1000;
    return ScopedAStatus::ok();
}

ScopedAStatus AndrosGnss::start() {
    if (running_.exchange(true)) return ScopedAStatus::ok();
    if (cb_) cb_->gnssStatusCb(gnss::IGnssCallback::GnssStatusValue::ENGINE_ON);
    thread_ = std::thread([this] { FixLoop(); });
    return ScopedAStatus::ok();
}

ScopedAStatus AndrosGnss::stop() {
    StopFixes();
    return ScopedAStatus::ok();
}

void AndrosGnss::StopFixes() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

GnssLocation AndrosGnss::CurrentFix() {
    if (!store_) store_ = Store::Open(kDefaultPath, /*writer=*/false);
    Sample s;  // fields: lat, lon, alt, speed, bearing, accuracy
    if (!store_ || !store_->Read(kLocationIndex, &s)) s = Sample();
    GnssLocation loc{};
    loc.gnssLocationFlags = kFlags;
    loc.latitudeDegrees = s.v[0];
    loc.longitudeDegrees = s.v[1];
    loc.altitudeMeters = s.v[2];
    loc.speedMetersPerSec = s.v[3];
    loc.bearingDegrees = s.v[4];
    loc.horizontalAccuracyMeters = s.v[5];
    loc.timestampMillis = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now().time_since_epoch()).count();
    loc.elapsedRealtime.flags = gnss::ElapsedRealtime::HAS_TIMESTAMP_NS;
    loc.elapsedRealtime.timestampNs = NowNs();
    return loc;
}

void AndrosGnss::FixLoop() {
    while (running_) {
        std::shared_ptr<gnss::IGnssCallback> cb;
        { std::lock_guard<std::mutex> lock(mu_); cb = cb_; }
        if (cb) cb->gnssLocationCb(CurrentFix());
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
    }
}

}  // namespace andros
