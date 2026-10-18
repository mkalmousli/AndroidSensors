// Optional IGnss extensions. Psds and Configuration are no-op stubs (the framework crashes on null);
// the rest return an error status so the framework treats them as unavailable (an ok status with a
// null interface makes it dereference null).
#include "andros_gnss.h"
#include "gnss_stubs.h"

namespace andros {

#define ANDROS_NO_EXTENSION(method, type)                         \
    ScopedAStatus AndrosGnss::method(std::shared_ptr<type>* out) { \
        *out = nullptr;                                            \
        return ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION); \
    }

ScopedAStatus AndrosGnss::getExtensionPsds(std::shared_ptr<gnss::IGnssPsds>* out) {
    *out = ndk::SharedRefBase::make<PsdsStub>();
    return ScopedAStatus::ok();
}

ScopedAStatus AndrosGnss::getExtensionGnssConfiguration(std::shared_ptr<gnss::IGnssConfiguration>* out) {
    *out = ndk::SharedRefBase::make<ConfigurationStub>();
    return ScopedAStatus::ok();
}

ANDROS_NO_EXTENSION(getExtensionGnssMeasurement, gnss::IGnssMeasurementInterface)
ANDROS_NO_EXTENSION(getExtensionGnssPowerIndication, gnss::IGnssPowerIndication)
ANDROS_NO_EXTENSION(getExtensionGnssBatching, gnss::IGnssBatching)
ANDROS_NO_EXTENSION(getExtensionGnssGeofence, gnss::IGnssGeofence)
ANDROS_NO_EXTENSION(getExtensionGnssNavigationMessage, gnss::IGnssNavigationMessageInterface)
ANDROS_NO_EXTENSION(getExtensionAGnss, gnss::IAGnss)
ANDROS_NO_EXTENSION(getExtensionAGnssRil, gnss::IAGnssRil)
ANDROS_NO_EXTENSION(getExtensionGnssDebug, gnss::IGnssDebug)
ANDROS_NO_EXTENSION(getExtensionGnssVisibilityControl,
                    gnss::visibility_control::IGnssVisibilityControl)
ANDROS_NO_EXTENSION(getExtensionGnssAntennaInfo, gnss::IGnssAntennaInfo)
ANDROS_NO_EXTENSION(getExtensionMeasurementCorrections,
                    gnss::measurement_corrections::IMeasurementCorrectionsInterface)

}  // namespace andros
