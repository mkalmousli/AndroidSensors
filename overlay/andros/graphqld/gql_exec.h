// Executes a parsed operation: root fields come from a Resolver, nested
// selections are projected onto the returned value.
#pragma once
#include <functional>
#include <string>

#include "gql.h"

namespace andros {

// Resolve a root field. Return false and set *error on failure.
using Resolver = std::function<bool(const Operation&, const Field&, Value* out, std::string* error)>;

// Returns {"data": {...}, "errors": [{"message": ...}]} (errors only when present).
Value Execute(const Operation& op, const Resolver& resolve);

// Project `selection` onto `value`; appends problems to `errors`.
Value Project(const std::vector<Field>& selection, const Value& value, std::vector<std::string>* errors);

}  // namespace andros
