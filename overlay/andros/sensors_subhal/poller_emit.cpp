#include <algorithm>

#include "poller.h"
#include "sensor_table.h"

namespace andros {

constexpr int32_t kTypeStepCounter = 19;

using ::android::hardware::sensors::V2_1::SensorType;
namespace V1_0 = ::android::hardware::sensors::V1_0;

Event FlushComplete(int32_t handle) {
    Event ev{};
    ev.sensorType = SensorType::META_DATA;
    ev.u.meta.what = V1_0::MetaDataEventType::META_DATA_FLUSH_COMPLETE;
    ev.sensorHandle = handle;
    return ev;
}

bool Poller::Emit(int32_t handle, State& st, std::vector<Event>* out) {
    int idx = HandleToIndex(handle);
    const SensorDef& def = kSensors[idx];
    Sample sample;
    if (!store_ || !store_->Read(idx, &sample)) {  // daemon not up yet: use catalog defaults
        for (int i = 0; i < def.n; ++i) sample.v[i] = def.init[i];
    }
    bool changed = !st.has_last || !std::equal(sample.v, sample.v + def.n, st.last.v);
    const bool trigger = def.mode == Mode::kOneShot || def.mode == Mode::kSpecial;
    if (trigger && !st.has_last) {  // a trigger fires on a counter change, never on activation
        st.last = sample;
        st.has_last = true;
        return false;
    }
    if ((def.mode == Mode::kOnChange || trigger) && !changed) return false;
    Event ev{};
    ev.timestamp = NowNs();
    ev.sensorHandle = handle;
    ev.sensorType = static_cast<SensorType>(def.type);
    if (trigger) {
        ev.u.data[0] = 1.0f;  // scalar payload of detector / gesture events
    } else if (def.type == kTypeStepCounter) {
        ev.u.stepCount = static_cast<uint64_t>(sample.v[0]);  // uint64, not float
    } else {
        for (int i = 0; i < def.n; ++i) ev.u.data[i] = static_cast<float>(sample.v[i]);
    }
    out->push_back(ev);
    if (def.mode == Mode::kOneShot) st.on = false;  // one-shot sensors disable after firing
    st.last = sample;
    st.has_last = true;
    return true;
}

}  // namespace andros
