#include <algorithm>

#include "poller.h"
#include "sensor_table.h"

namespace andros {

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
    if (def.mode == Mode::kOnChange && !changed) return false;
    Event ev{};
    ev.timestamp = NowNs();
    ev.sensorHandle = handle;
    ev.sensorType = static_cast<SensorType>(def.type);
    for (int i = 0; i < def.n; ++i) ev.u.data[i] = static_cast<float>(sample.v[i]);
    out->push_back(ev);
    st.last = sample;
    st.has_last = true;
    return true;
}

}  // namespace andros
