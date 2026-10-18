// GraphQL surface of the route player: playRoute / stopRoute mutations and the motion query.
#include "player.h"
#include "resolvers.h"

namespace andros {

bool RouteMutation(Store* store, const Field& field, Value* out, std::string* error) {
    if (field.name == "stopRoute") {
        StopRoute();
        *out = Value::Obj();
        out->Set("ok", Value::Bool(true));
        return true;
    }
    const Value* pts = field.Arg("points");  // [[lat, lon, alt?], ...]
    if (!pts || pts->a.empty()) { *error = "playRoute requires 'points': [[lat, lon, alt], ...]"; return false; }
    std::vector<Pt> route;
    for (const Value& p : pts->a) {
        if (p.a.size() < 2) { *error = "each point needs [lat, lon]"; return false; }
        route.push_back({p.a[0].AsNum(0), p.a[1].AsNum(0), p.a.size() > 2 ? p.a[2].AsNum(0) : 0});
    }
    const Value* speed = field.Arg("speed");
    const Value* loop = field.Arg("loop");
    PlayRoute(store, std::move(route), speed ? speed->AsNum(1.4) : 1.4, loop && loop->b);
    *out = Value::Obj();
    out->Set("ok", Value::Bool(true));
    return true;
}

Value MotionValue() {
    const Motion m = CurrentMotion();
    Value v = Value::Obj();
    v.Set("activity", Value::Str(m.activity));
    v.Set("steps", Value::Num(m.steps));
    v.Set("cadence", Value::Num(m.cadence_hz));
    v.Set("floors", Value::Num(m.floors));
    v.Set("distance", Value::Num(m.distance_m));
    v.Set("speed", Value::Num(m.speed));
    return v;
}

}  // namespace andros
