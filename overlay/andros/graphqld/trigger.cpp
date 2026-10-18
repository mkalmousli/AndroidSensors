#include "resolvers.h"

namespace andros {

bool Resolvers::Trigger(const Field& field, Value* out, std::string* error) const {
    std::string rest = field.name.substr(7);  // fire a detector / gesture: bump its counter
    rest[0] = char(tolower(rest[0]));
    int idx = FindSensor(rest);
    if (idx < 0) { *error = "Unknown sensor '" + rest + "'"; return false; }
    if (store_->Flags(idx) & kDisabled) { *error = "Sensor '" + rest + "' is disabled in settings"; return false; }
    Sample current;
    store_->Read(idx, &current);
    current.v[0] += 1;
    store_->Write(idx, current.v, kSensors[idx].n);
    *out = Describe(idx);
    out->Set("ok", Value::Bool(true));
        return true;
}

}  // namespace andros
