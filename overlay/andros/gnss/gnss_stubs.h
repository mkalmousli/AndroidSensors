// Extensions the framework dereferences without a null check (Psds, Configuration):
// accept every call and do nothing - the position source needs no assistance data or tuning.
#pragma once
#include <aidl/android/hardware/gnss/BnGnssConfiguration.h>
#include <aidl/android/hardware/gnss/BnGnssPsds.h>

namespace andros {

namespace gnss = ::aidl::android::hardware::gnss;
using ndk::ScopedAStatus;

class PsdsStub : public gnss::BnGnssPsds {
  public:
    ScopedAStatus injectPsdsData(gnss::PsdsType, const std::vector<uint8_t>&) override {
        return ScopedAStatus::ok();
    }
    ScopedAStatus setCallback(const std::shared_ptr<gnss::IGnssPsdsCallback>&) override {
        return ScopedAStatus::ok();
    }
};

class ConfigurationStub : public gnss::BnGnssConfiguration {
  public:
    ScopedAStatus setSuplVersion(int) override { return ScopedAStatus::ok(); }
    ScopedAStatus setSuplMode(int) override { return ScopedAStatus::ok(); }
    ScopedAStatus setLppProfile(int) override { return ScopedAStatus::ok(); }
    ScopedAStatus setGlonassPositioningProtocol(int) override { return ScopedAStatus::ok(); }
    ScopedAStatus setEmergencySuplPdn(bool) override { return ScopedAStatus::ok(); }
    ScopedAStatus setEsExtensionSec(int) override { return ScopedAStatus::ok(); }
    ScopedAStatus setBlocklist(const std::vector<gnss::BlocklistedSource>&) override {
        return ScopedAStatus::ok();
    }
};

}  // namespace andros
