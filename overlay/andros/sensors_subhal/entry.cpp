#include "andros_subhal.h"

// Entry point looked up by the multihal loader for every library in hals.conf.
extern "C" ::android::hardware::sensors::V2_1::implementation::ISensorsSubHal*
sensorsHalGetSubHal_2_1(uint32_t* version) {
    static andros::AndrosSubHal subhal;
    *version = SUB_HAL_2_1_VERSION;
    return &subhal;
}
