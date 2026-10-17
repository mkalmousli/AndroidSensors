#include <cstdlib>
#include <cstring>

#include "gql_parser.h"

namespace andros {

const Value* Field::Arg(const std::string& key) const {
    for (const auto& kv : args) {
        if (kv.first == key) return &kv.second;
    }
    return nullptr;
}

bool ParseOperation(const std::string& src, const Value& variables, Operation* op,
                    std::string* error) {
    Parser p{{}, variables, 0, {}};
    if (!Tokenize(src, &p.toks, error)) return false;
    if (!p.Run(op)) { *error = p.err; return false; }
    return true;
}

}  // namespace andros
