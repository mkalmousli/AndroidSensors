// Catalog-driven resolvers: every catalog entry becomes a query field and a
// set<Name> mutation, plus the generic sensors/sensor/setSensor fields.
#pragma once
#include <string>

#include "gql_exec.h"
#include "store.h"

namespace andros {

class Resolvers {
  public:
    explicit Resolvers(Store* store) : store_(store) {}
    bool operator()(const Operation& op, const Field& field, Value* out, std::string* error) const;

    Value Describe(int idx) const;  // {name, hal, unit, ts, values, <fields>...}

  private:
    bool Query(const Field& field, Value* out, std::string* error) const;
    bool Mutate(const Field& field, Value* out, std::string* error) const;
    bool SetByName(const std::string& name, const Field& field, Value* out, std::string* error) const;
    Store* store_;
};

int FindSensor(const std::string& name);  // -1 when unknown

}  // namespace andros
