#include "gql_exec.h"

namespace andros {

Value Project(const std::vector<Field>& selection, const Value& value,
              std::vector<std::string>* errors) {
    if (selection.empty()) return value;
    if (value.kind == Value::kArr) {
        Value out = Value::Arr();
        for (const Value& item : value.a) out.Push(Project(selection, item, errors));
        return out;
    }
    if (value.kind != Value::kObj) return value;
    Value out = Value::Obj();
    for (const Field& f : selection) {
        if (f.name == "__typename") { out.Set(f.alias, Value::Str("Object")); continue; }
        const Value* child = value.Get(f.name);
        if (!child) { errors->push_back("Cannot query field '" + f.name + "'"); continue; }
        out.Set(f.alias, Project(f.selection, *child, errors));
    }
    return out;
}

Value Execute(const Operation& op, const Resolver& resolve) {
    Value data = Value::Obj();
    std::vector<std::string> errors;
    for (const Field& f : op.selection) {
        Value resolved;
        std::string error;
        if (!resolve(op, f, &resolved, &error)) {
            errors.push_back(error);
            data.Set(f.alias, Value());
            continue;
        }
        data.Set(f.alias, Project(f.selection, resolved, &errors));
    }
    Value result = Value::Obj();
    result.Set("data", std::move(data));
    if (!errors.empty()) {
        Value list = Value::Arr();
        for (const auto& message : errors) {
            Value e = Value::Obj();
            e.Set("message", Value::Str(message));
            list.Push(std::move(e));
        }
        result.Set("errors", std::move(list));
    }
    return result;
}

}  // namespace andros
