// Minimal JSON value used for GraphQL data, arguments and HTTP bodies.
#pragma once
#include <string>
#include <utility>
#include <vector>

namespace andros {

struct Value {
    enum Kind { kNull, kBool, kNum, kStr, kArr, kObj } kind = kNull;
    bool b = false;
    double n = 0;
    std::string s;
    std::vector<Value> a;
    std::vector<std::pair<std::string, Value>> o;  // insertion ordered

    static Value Num(double v) { Value x; x.kind = kNum; x.n = v; return x; }
    static Value Bool(bool v) { Value x; x.kind = kBool; x.b = v; return x; }
    static Value Str(std::string v) { Value x; x.kind = kStr; x.s = std::move(v); return x; }
    static Value Arr() { Value x; x.kind = kArr; return x; }
    static Value Obj() { Value x; x.kind = kObj; return x; }

    Value& Set(const std::string& key, Value v);
    const Value* Get(const std::string& key) const;
    void Push(Value v) { a.push_back(std::move(v)); }
    double AsNum(double fallback = 0) const;
    std::string Dump() const;
};

bool ParseJson(const std::string& text, Value* out);

}  // namespace andros
