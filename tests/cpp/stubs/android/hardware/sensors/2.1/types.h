// Host-only stand-in for the HIDL-generated types, just enough to compile and
// unit-test the poller and sensor table without an AOSP tree.
#pragma once
#include <cstdint>
#include <string>

namespace android::hardware::sensors {
namespace V1_0 {
enum class MetaDataEventType : int32_t { META_DATA_FLUSH_COMPLETE = 1 };
}
namespace V2_1 {
enum class SensorType : int32_t { META_DATA = 0 };
struct SensorInfo {
    int32_t sensorHandle; std::string name, vendor; int32_t version; SensorType type;
    std::string typeAsString; float maxRange, resolution, power; int32_t minDelay, maxDelay;
    uint32_t fifoReservedEventCount, fifoMaxEventCount; uint32_t flags;
};
struct Event {
    int64_t timestamp; int32_t sensorHandle; SensorType sensorType;
    union { float data[16]; struct { V1_0::MetaDataEventType what; int32_t sensor; } meta; } u;
};
}  // namespace V2_1
}  // namespace android::hardware::sensors
