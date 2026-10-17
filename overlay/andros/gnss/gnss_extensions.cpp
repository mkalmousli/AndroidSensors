// Optional IGnss extensions: none are implemented, so each reports "unavailable".
#include "andros_gnss.h"

namespace andros {

#define ANDROS_NO_EXTENSION(method, type)                         \
    ScopedAStatus AndrosGnss::method(std::shared_ptr<type>* out) { \
        *out = nullptr;                                            \
        return ScopedAStatus::ok();                                \
    }

ANDROS_NO_EXTENSION(getExtensionPsds, gnss::IGnssPsds)
ANDROS_NO_EXTENSION(getExtensionGnssConfiguration, gnss::IGnssConfiguration)
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
