#pragma once
#include "http.h"
#include "resolvers.h"

namespace andros {

// Route one request on `fd` (regular queries, mutations and SSE subscriptions).
void Handle(int fd, const Request& req, const Resolvers& resolvers, Store* store);

// Stream a subscription as server-sent events until the client disconnects.
void StreamSubscription(int fd, const Operation& op, const Resolvers& resolvers, Store* store);

}  // namespace andros
