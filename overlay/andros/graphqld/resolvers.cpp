#include "resolvers.h"

#include <cstring>

namespace andros {

int FindSensor(const std::string& name) {
    for (int i = 0; i < kSensorCount; ++i) {
        if (name == kSensors[i].name) return i;
    }
    return -1;
}

Value Resolvers::Describe(int idx) const {
    const SensorDef& def = kSensors[idx];
    Sample sample;
    store_->Read(idx, &sample);
    Value obj = Value::Obj();
    obj.Set("name", Value::Str(def.name));
    obj.Set("unit", Value::Str(def.unit));
    obj.Set("ts", Value::Num(double(sample.ts_ns)));
    Value values = Value::Arr();
    for (int i = 0; i < def.n; ++i) {
        values.Push(Value::Num(sample.v[i]));
        obj.Set(def.fields[i], Value::Num(sample.v[i]));
    }
    obj.Set("values", std::move(values));
    return obj;
}

bool Resolvers::operator()(const Operation& op, const Field& field, Value* out,
                           std::string* error) const {
    if (field.name == "__typename") { *out = Value::Str("Root"); return true; }
    return op.kind == "mutation" ? Mutate(field, out, error) : Query(field, out, error);
}

bool Resolvers::Query(const Field& field, Value* out, std::string* error) const {
    if (field.name == "sensors") {
        *out = Value::Arr();
        for (int i = 0; i < kSensorCount; ++i) out->Push(Describe(i));
        return true;
    }
    const Value* arg = field.Arg("name");
    int idx = field.name == "sensor" && arg ? FindSensor(arg->s) : FindSensor(field.name);
    if (idx < 0) { *error = "Unknown sensor field '" + field.name + "'"; return false; }
    *out = Describe(idx);
    return true;
}

bool Resolvers::SetByName(const std::string& name, const Field& field, Value* out,
                          std::string* error) const {
    int idx = FindSensor(name);
    if (idx < 0) { *error = "Unknown sensor '" + name + "'"; return false; }
    const SensorDef& def = kSensors[idx];
    Sample current;
    store_->Read(idx, &current);
    const Value* values = field.Arg("values");
    if (values) {  // generic form: positional values
        for (size_t i = 0; i < values->a.size() && i < size_t(def.n); ++i)
            current.v[i] = values->a[i].AsNum(current.v[i]);
    }
    for (int i = 0; i < def.n; ++i) {  // typed form: named arguments
        if (const Value* v = field.Arg(def.fields[i])) current.v[i] = v->AsNum(current.v[i]);
    }
    store_->Write(idx, current.v, def.n);
    *out = Describe(idx);
    out->Set("ok", Value::Bool(true));
    return true;
}

bool Resolvers::Mutate(const Field& field, Value* out, std::string* error) const {
    if (field.name == "setSensor") {
        const Value* name = field.Arg("name");
        if (!name) { *error = "setSensor requires 'name'"; return false; }
        return SetByName(name->s, field, out, error);
    }
    if (field.name.compare(0, 7, "trigger") == 0 && field.name.size() > 7) {
        std::string rest = field.name.substr(7);  // fire a detector / gesture: bump its counter
        rest[0] = char(tolower(rest[0]));
        int idx = FindSensor(rest);
        if (idx < 0) { *error = "Unknown sensor '" + rest + "'"; return false; }
        Sample current;
        store_->Read(idx, &current);
        current.v[0] += 1;
        store_->Write(idx, current.v, kSensors[idx].n);
        *out = Describe(idx);
        out->Set("ok", Value::Bool(true));
        return true;
    }
    if (field.name.compare(0, 3, "set") == 0 && field.name.size() > 3) {
        std::string rest = field.name.substr(3);
        rest[0] = char(tolower(rest[0]));
        return SetByName(rest, field, out, error);
    }
    *error = "Unknown mutation '" + field.name + "'";
    return false;
}

}  // namespace andros
