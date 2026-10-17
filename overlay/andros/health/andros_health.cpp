#include "andros_health.h"

#include <cstdlib>

namespace andros {

using ::aidl::android::hardware::health::BatteryHealth;
using ::aidl::android::hardware::health::BatteryStatus;
using ::aidl::android::hardware::health::HealthInfo;

namespace {
constexpr int kBatteryIndex = [] {
    for (int i = 0; i < kSensorCount; ++i)
        if (kSensors[i].hal == Hal::kHealth) return i;
    return -1;
}();
constexpr int kCapacityUah = 3000000;  // advertised 3000 mAh pack
}  // namespace

Store* AndrosHealth::store() {
    if (!store_) store_ = Store::Open(kDefaultPath, /*writer=*/false);
    return store_;
}

void AndrosHealth::UpdateHealthInfo(HealthInfo* info) {
    Sample s;  // fields: level, charging, temperature, voltage, ac
    if (!store() || !store()->Read(kBatteryIndex, &s)) return;
    const int level = int(s.v[0]);
    const bool charging = s.v[1] > 0.5, ac = s.v[4] > 0.5;
    info->batteryPresent = true;
    info->batteryLevel = level;
    info->batteryStatus = !charging ? BatteryStatus::DISCHARGING
                                    : (level >= 100 ? BatteryStatus::FULL : BatteryStatus::CHARGING);
    info->batteryHealth = BatteryHealth::GOOD;
    info->batteryTemperatureTenthsCelsius = int(s.v[2] * 10);
    info->batteryVoltageMillivolts = int(s.v[3]);
    info->chargerAcOnline = charging && ac;
    info->chargerUsbOnline = charging && !ac;
    info->batteryChargeCounterUah = int(double(kCapacityUah) * level / 100);
    info->batteryFullChargeUah = kCapacityUah;
    info->batteryCurrentMicroamps = charging ? 500000 : -250000;
}

}  // namespace andros
