#include <android-base/logging.h>
#include <android/binder_interface_utils.h>
#include <health-impl/HalHealthLoop.h>

#include "andros_health.h"

using ::aidl::android::hardware::health::HalHealthLoop;

int main() {
    auto config = std::make_unique<healthd_config>();
    ::android::hardware::health::InitHealthdConfig(config.get());
    auto binder = ndk::SharedRefBase::make<andros::AndrosHealth>("default", std::move(config));
    HalHealthLoop loop(binder, binder);
    return loop.StartLoop();
}
