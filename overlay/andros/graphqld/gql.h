// GraphQL subset: query/mutation/subscription, nested selections, arguments,
// aliases and $variables. No fragments or directives.
#pragma once
#include <string>
#include <utility>
#include <vector>

#include "json.h"

namespace andros {

struct Field {
    std::string name;
    std::string alias;  // response key; defaults to name
    std::vector<std::pair<std::string, Value>> args;
    std::vector<Field> selection;

    const Value* Arg(const std::string& key) const;
};

struct Operation {
    std::string kind = "query";
    std::vector<Field> selection;
};

bool ParseOperation(const std::string& src, const Value& variables, Operation* op,
                    std::string* error);

// Token stream (exposed for tests).
struct Token {
    enum Type { kName, kNum, kStr, kPunct, kEnd } type = kEnd;
    std::string text;
};

bool Tokenize(const std::string& src, std::vector<Token>* out, std::string* error);

}  // namespace andros
