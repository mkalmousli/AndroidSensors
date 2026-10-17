#include <cstdlib>

#include "gql_parser.h"

namespace andros {

bool Parser::Value_(Value* v) {
        const Token& t = Peek();
        if (t.type == Token::kNum) { *v = Value::Num(atof(t.text.c_str())); ++pos; return true; }
        if (t.type == Token::kStr) { *v = Value::Str(t.text); ++pos; return true; }
        if (t.type == Token::kName) {
            if (t.text == "true" || t.text == "false") *v = Value::Bool(t.text == "true");
            else if (t.text == "null") *v = Value();
            else *v = Value::Str(t.text);  // enum
            ++pos;
            return true;
        }
        if (Eat("$")) {
            const Value* var = Peek().type == Token::kName ? vars.Get(Peek().text) : nullptr;
            *v = var ? *var : Value();
            ++pos;
            return true;
        }
        if (Eat("[")) {
            *v = Value::Arr();
            while (!Eat("]")) {
                Value item;
                if (!Value_(&item)) return false;
                v->Push(std::move(item));
            }
            return true;
        }
        if (Eat("{")) {
            *v = Value::Obj();
            while (!Eat("}")) {
                std::string key = Peek().text;
                Value item;
                if (Peek().type != Token::kName) return Fail("expected field name");
                ++pos;
                if (!Eat(":") || !Value_(&item)) return Fail("bad object value");
                v->Set(key, std::move(item));
            }
            return true;
        }
        return Fail("unexpected value");
    }

}  // namespace andros
