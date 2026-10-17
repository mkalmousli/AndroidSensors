#include <cstdlib>
#include <cstring>

#include "json.h"

namespace andros {
namespace {

struct Parser {
    const char* p;
    const char* end;

    void Ws() { while (p < end && strchr(" \t\r\n", *p)) ++p; }
    bool Lit(const char* word) {
        size_t n = strlen(word);
        if (size_t(end - p) < n || strncmp(p, word, n) != 0) return false;
        p += n;
        return true;
    }
    bool String(std::string* out) {
        if (p >= end || *p != '"') return false;
        for (++p; p < end && *p != '"'; ++p) {
            if (*p != '\\') { out->push_back(*p); continue; }
            if (++p >= end) return false;
            switch (*p) {
                case 'n': out->push_back('\n'); break;
                case 't': out->push_back('\t'); break;
                case 'r': out->push_back('\r'); break;
                case 'u':
                    if (end - p < 5) return false;
                    out->push_back(char(strtol(std::string(p + 1, 4).c_str(), nullptr, 16)));
                    p += 4;
                    break;
                default: out->push_back(*p);
            }
        }
        if (p >= end) return false;
        ++p;
        return true;
    }
    bool Val(Value* v) {
        Ws();
        if (p >= end) return false;
        if (*p == '"') { v->kind = Value::kStr; return String(&v->s); }
        if (*p == '{') return Obj(v);
        if (*p == '[') return Arr(v);
        if (Lit("true")) { *v = Value::Bool(true); return true; }
        if (Lit("false")) { *v = Value::Bool(false); return true; }
        if (Lit("null")) { *v = Value(); return true; }
        char* stop;
        double d = strtod(p, &stop);
        if (stop == p) return false;
        p = stop;
        *v = Value::Num(d);
        return true;
    }
    bool Arr(Value* v) {
        *v = Value::Arr();
        for (++p, Ws(); p < end && *p != ']';) {
            Value item;
            if (!Val(&item)) return false;
            v->Push(std::move(item));
            Ws();
            if (p < end && *p == ',') ++p;
            Ws();
        }
        if (p >= end) return false;
        ++p;
        return true;
    }
    bool Obj(Value* v) {
        *v = Value::Obj();
        for (++p, Ws(); p < end && *p != '}';) {
            std::string key;
            Value item;
            if (!String(&key)) return false;
            Ws();
            if (p >= end || *p++ != ':' || !Val(&item)) return false;
            v->Set(key, std::move(item));
            Ws();
            if (p < end && *p == ',') ++p;
            Ws();
        }
        if (p >= end) return false;
        ++p;
        return true;
    }
};

}  // namespace

bool ParseJson(const std::string& text, Value* out) {
    Parser parser{text.data(), text.data() + text.size()};
    return parser.Val(out);
}

}  // namespace andros
