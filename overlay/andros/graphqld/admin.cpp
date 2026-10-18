// Settings API: status/metrics, searchable sensor list, API and per-sensor switches.
#include <algorithm>
#include <cctype>

#include "metrics.h"
#include "resolvers.h"

namespace andros {

Metrics& GetMetrics() {
    static Metrics m;
    if (!m.start_ns) m.start_ns = NowNs();
    return m;
}

namespace {
std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return s;
}

bool Matches(const SensorDef& d, const std::string& q) {
    if (q.empty()) return true;
    std::string hay = Lower(std::string(d.name) + " " + d.unit + " " +
                            (d.hal == Hal::kSensors ? "sensors" : d.hal == Hal::kGnss ? "gnss" : "health"));
    for (int i = 0; i < d.n; ++i) hay += std::string(" ") + Lower(d.fields[i]);
    return hay.find(Lower(q)) != std::string::npos;
}

bool Flag(const Field& f, const char* key, bool fallback) {
    const Value* v = f.Arg(key);
    return v ? v->b : fallback;
}
}  // namespace

bool Resolvers::Admin(const Operation& op, const Field& f, Value* out, std::string* error,
                      bool* handled) const {
    *handled = true;
    if (f.name == "sensors") {  // sensors(search: "acc") - substring over name, unit, hal, fields
        const Value* q = f.Arg("search");
        *out = Value::Arr();
        for (int i = 0; i < kSensorCount; ++i)
            if (Matches(kSensors[i], q ? q->s : "")) out->Push(Describe(i));
        return true;
    }
    if (f.name == "status") {
        Metrics& m = GetMetrics();
        int enabled = 0, dummy = 0;
        for (int i = 0; i < kSensorCount; ++i) {
            enabled += !(store_->Flags(i) & kDisabled);
            dummy += !!(store_->Flags(i) & kDummy);
        }
        *out = Value::Obj();
        out->Set("apiEnabled", Value::Bool(store_->ApiEnabled()));
        out->Set("uptimeSec", Value::Num(double(NowNs() - m.start_ns) / 1e9));
        out->Set("requests", Value::Num(double(m.requests)));
        out->Set("errors", Value::Num(double(m.errors)));
        out->Set("streams", Value::Num(double(m.streams)));
        out->Set("sensors", Value::Num(kSensorCount));
        out->Set("enabled", Value::Num(enabled));
        out->Set("dummy", Value::Num(dummy));
        out->Set("writes", Value::Num(double(store_->Generation())));
        return true;
    }
    if (op.kind != "mutation") { *handled = false; return false; }
    if (f.name == "setApiEnabled") {
        store_->SetApiEnabled(Flag(f, "enabled", true));
        *out = Value::Obj();
        out->Set("ok", Value::Bool(true));
        return true;
    }
    if (f.name == "setSensorEnabled" || f.name == "setSensorDummy") {  // omit name = all sensors
        const bool on_off = f.name == "setSensorEnabled";
        const Value* name = f.Arg("name");
        const Value* q = f.Arg("search");
        int changed = 0;
        for (int i = 0; i < kSensorCount; ++i) {
            if (name ? name->s != kSensors[i].name : !Matches(kSensors[i], q ? q->s : "")) continue;
            uint32_t flags = store_->Flags(i);
            const uint32_t bit = on_off ? kDisabled : kDummy;
            const bool set = on_off ? !Flag(f, "enabled", true) : Flag(f, "dummy", true);
            store_->SetFlags(i, set ? (flags | bit) : (flags & ~bit));
            ++changed;
        }
        if (name && !changed) { *error = "Unknown sensor '" + name->s + "'"; return false; }
        *out = Value::Obj();
        out->Set("ok", Value::Bool(true));
        out->Set("changed", Value::Num(changed));
        return true;
    }
    *handled = false;
    return false;
}

}  // namespace andros
