#include "sensor_table.h"

namespace andros {

using ::android::hardware::sensors::V2_1::SensorType;

namespace {
// SensorFlagBits reporting mode occupies bits 1..3 of SensorInfo.flags.
uint32_t ModeFlags(Mode mode) {
    switch (mode) {
        case Mode::kOnChange: return 1u << 1;
        case Mode::kSpecial: return 3u << 1;
        case Mode::kOneShot: return 2u << 1;
        default: return 0;
    }
}
}  // namespace

const std::vector<int>& HalSensors() {
    static const std::vector<int> list = [] {
        std::vector<int> out;
        for (int i = 0; i < kSensorCount; ++i)
            if (kSensors[i].hal == Hal::kSensors) out.push_back(i);
        return out;
    }();
    return list;
}

int HandleToIndex(int32_t handle) {
    const auto& list = HalSensors();
    return handle >= 1 && size_t(handle) <= list.size() ? list[handle - 1] : -1;
}

SensorInfo MakeInfo(int32_t handle) {
    const SensorDef& def = kSensors[HandleToIndex(handle)];
    SensorInfo info{};
    info.sensorHandle = handle;
    info.name = std::string(gen::kSensorVendor) + " " + def.name;
    info.vendor = gen::kSensorVendor;
    info.version = 1;
    info.type = static_cast<SensorType>(def.type);
    info.typeAsString = def.type_string;
    info.maxRange = def.max_range;
    info.resolution = def.resolution;
    info.power = def.power;
    info.minDelay = def.min_delay_us;
    info.maxDelay = def.mode == Mode::kContinuous ? 1000000 : 0;
    info.fifoReservedEventCount = 0;
    info.fifoMaxEventCount = 0;
    info.flags = ModeFlags(def.mode) | (def.wakeup ? 1u : 0u);  // bit 0 = WAKE_UP
    return info;
}

}  // namespace andros
