// IGnss implementation: every fix is the latest "location" sample in the store.
#pragma once
#include <aidl/android/hardware/gnss/BnGnss.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include "store.h"

namespace andros {

namespace gnss = ::aidl::android::hardware::gnss;
using ndk::ScopedAStatus;

class AndrosGnss : public gnss::BnGnss {
  public:
    ~AndrosGnss() override { StopFixes(); }

    ScopedAStatus setCallback(const std::shared_ptr<gnss::IGnssCallback>& cb) override;
    ScopedAStatus close() override;
    ScopedAStatus start() override;
    ScopedAStatus stop() override;
    ScopedAStatus injectTime(int64_t, int64_t, int) override { return ScopedAStatus::ok(); }
    ScopedAStatus injectLocation(const gnss::GnssLocation&) override { return ScopedAStatus::ok(); }
    ScopedAStatus injectBestLocation(const gnss::GnssLocation&) override { return ScopedAStatus::ok(); }
    ScopedAStatus deleteAidingData(GnssAidingData) override { return ScopedAStatus::ok(); }
    ScopedAStatus setPositionMode(const PositionModeOptions& o) override;

    // Optional extensions are reported as unavailable (see gnss_extensions.cpp).
    ScopedAStatus getExtensionPsds(std::shared_ptr<gnss::IGnssPsds>*) override;
    ScopedAStatus getExtensionGnssConfiguration(std::shared_ptr<gnss::IGnssConfiguration>*) override;
    ScopedAStatus getExtensionGnssMeasurement(std::shared_ptr<gnss::IGnssMeasurementInterface>*) override;
    ScopedAStatus getExtensionGnssPowerIndication(std::shared_ptr<gnss::IGnssPowerIndication>*) override;
    ScopedAStatus getExtensionGnssBatching(std::shared_ptr<gnss::IGnssBatching>*) override;
    ScopedAStatus getExtensionGnssGeofence(std::shared_ptr<gnss::IGnssGeofence>*) override;
    ScopedAStatus getExtensionGnssNavigationMessage(std::shared_ptr<gnss::IGnssNavigationMessageInterface>*) override;
    ScopedAStatus getExtensionAGnss(std::shared_ptr<gnss::IAGnss>*) override;
    ScopedAStatus getExtensionAGnssRil(std::shared_ptr<gnss::IAGnssRil>*) override;
    ScopedAStatus getExtensionGnssDebug(std::shared_ptr<gnss::IGnssDebug>*) override;
    ScopedAStatus getExtensionGnssVisibilityControl(std::shared_ptr<gnss::visibility_control::IGnssVisibilityControl>*) override;
    ScopedAStatus getExtensionGnssAntennaInfo(std::shared_ptr<gnss::IGnssAntennaInfo>*) override;
    ScopedAStatus getExtensionMeasurementCorrections(std::shared_ptr<gnss::measurement_corrections::IMeasurementCorrectionsInterface>*) override;

  private:
    void StopFixes();
    void FixLoop();
    gnss::GnssLocation CurrentFix();

    std::mutex mu_;
    std::shared_ptr<gnss::IGnssCallback> cb_;
    std::thread thread_;
    std::atomic<bool> running_{false};
    int interval_ms_ = 1000;
    Store* store_ = nullptr;
};

}  // namespace andros
