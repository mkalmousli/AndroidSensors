#include "json.h"

#include <cmath>
#include <cstdio>

namespace andros {

Value& Value::Set(const std::string& key, Value v) {
    kind = kObj;
    for (auto& kv : o) {
        if (kv.first == key) { kv.second = std::move(v); return kv.second; }
    }
    o.emplace_back(key, std::move(v));
    return o.back().second;
}

const Value* Value::Get(const std::string& key) const {
    for (const auto& kv : o) {
        if (kv.first == key) return &kv.second;
    }
    return nullptr;
}

double Value::AsNum(double fallback) const {
    if (kind == kNum) return n;
    if (kind == kBool) return b ? 1 : 0;
    return fallback;
}

static void Quote(const std::string& in, std::string* out) {
    out->push_back('"');
    for (unsigned char c : in) {
        if (c == '"' || c == '\\') { out->push_back('\\'); out->push_back(c); }
        else if (c == '\n') out->append("\\n");
        else if (c < 0x20) { char buf[8]; snprintf(buf, sizeof buf, "\\u%04x", c); out->append(buf); }
        else out->push_back(c);
    }
    out->push_back('"');
}

std::string Value::Dump() const {
    std::string r;
    switch (kind) {
        case kNull: return "null";
        case kBool: return b ? "true" : "false";
        case kNum: {
            if (!std::isfinite(n)) return "null";
            char buf[40];
            snprintf(buf, sizeof buf, "%.15g", n);
            return buf;
        }
        case kStr: Quote(s, &r); return r;
        case kArr:
            r = "[";
            for (size_t i = 0; i < a.size(); ++i) r += (i ? "," : "") + a[i].Dump();
            return r + "]";
        case kObj:
            r = "{";
            for (size_t i = 0; i < o.size(); ++i) {
                if (i) r += ",";
                Quote(o[i].first, &r);
                r += ":" + o[i].second.Dump();
            }
            return r + "}";
    }
    return r;
}

}  // namespace andros
