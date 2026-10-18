// Maps the HAL-visible part of the catalog to HIDL SensorInfo records.
#pragma once
#include <vector>

#include <android/hardware/sensors/2.1/types.h>

#include "catalog_gen.h"
#include "device_gen.h"

namespace andros {

using ::android::hardware::sensors::V2_1::SensorInfo;

// Catalog indices of entries served by the sensors HAL, in handle order.
const std::vector<int>& HalSensors();

// Handle <-> catalog index (handles are 1-based positions in HalSensors()).
int HandleToIndex(int32_t handle);
SensorInfo MakeInfo(int32_t handle);

}  // namespace andros
