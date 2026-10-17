// Internal recursive-descent parser state shared by gql_parser.cpp / gql_values.cpp.
#pragma once
#include <string>
#include <vector>

#include "gql.h"

namespace andros {

struct Parser {
    std::vector<Token> toks;
    const Value& vars;
    size_t pos = 0;
    std::string err;

    const Token& Peek() const { return toks[pos]; }
    bool Is(const char* punct) const {
        return Peek().type == Token::kPunct && Peek().text == punct;
    }
    bool Eat(const char* punct) {
        if (!Is(punct)) return false;
        ++pos;
        return true;
    }
    bool Fail(const std::string& msg) {
        if (err.empty()) err = msg + " near '" + Peek().text + "'";
        return false;
    }

    bool Value_(Value* v);

    bool Selection(std::vector<Field>* out) {
        while (!Eat("}")) {
            if (Peek().type != Token::kName) return Fail("expected field");
            Field f;
            f.name = f.alias = toks[pos++].text;
            if (Eat(":")) {
                if (Peek().type != Token::kName) return Fail("expected field after alias");
                f.name = toks[pos++].text;
            }
            if (Eat("(")) {
                while (!Eat(")")) {
                    if (Peek().type != Token::kName) return Fail("expected argument name");
                    std::string key = toks[pos++].text;
                    Value v;
                    if (!Eat(":") || !Value_(&v)) return Fail("bad argument");
                    f.args.emplace_back(key, std::move(v));
                }
            }
            if (Eat("{") && !Selection(&f.selection)) return false;
            out->push_back(std::move(f));
        }
        return true;
    }

    bool SkipVariableDefs() {  // ($a: Float!, $b: [Float!] = [1])
        for (int depth = 1; depth > 0 && Peek().type != Token::kEnd; ++pos) {
            if (Is("(")) ++depth;
            if (Is(")")) --depth;
        }
        return true;
    }

    bool Run(Operation* op) {
        if (Peek().type == Token::kName) {
            op->kind = toks[pos++].text;
            if (op->kind != "query" && op->kind != "mutation" && op->kind != "subscription")
                return Fail("unknown operation");
            if (Peek().type == Token::kName) ++pos;  // operation name
            if (Eat("(")) SkipVariableDefs();
        }
        if (!Eat("{")) return Fail("expected '{'");
        return Selection(&op->selection);
    }
};

}  // namespace andros
