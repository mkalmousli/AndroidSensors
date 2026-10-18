#include "handler.h"

#include "graphiql.h"
#include "settings_html.h"

namespace andros {

static void Json(int fd, int status, const Value& v) { Respond(fd, status, "application/json", v.Dump()); }

static Value ErrorBody(const std::string& message) {
    Value e = Value::Obj(), list = Value::Arr(), body = Value::Obj();
    e.Set("message", Value::Str(message));
    list.Push(std::move(e));
    body.Set("errors", std::move(list));
    return body;
}

void Handle(int fd, const Request& req, const Resolvers& resolvers, Store* store) {
    if (req.method == "OPTIONS") return Respond(fd, 200, "text/plain", "");
    if (req.path == "/") return Respond(fd, 200, "text/html", kConsoleHtml);
    if (req.path == "/settings") return Respond(fd, 200, "text/html", kSettingsHtml);
    if (req.path == "/events") return StreamEvents(fd, req, resolvers, store);
    if (req.path == "/health") return Respond(fd, 200, "text/plain", "ok");
    if (req.path != "/graphql") return Respond(fd, 404, "text/plain", "not found");

    std::string query;
    Value variables = Value::Obj();
    if (req.method == "POST") {
        Value body;
        if (!ParseJson(req.body, &body)) return Json(fd, 400, ErrorBody("invalid JSON body"));
        if (const Value* q = body.Get("query")) query = q->s;
        if (const Value* v = body.Get("variables")) variables = *v;
    } else {
        query = QueryParam(req.query, "query");
    }
    Operation op;
    std::string error;
    if (!ParseOperation(query, variables, &op, &error)) return Json(fd, 400, ErrorBody(error));
    if (op.kind == "subscription") return StreamSubscription(fd, op, resolvers, store);
    Json(fd, 200, Execute(op, resolvers));
}

}  // namespace andros
