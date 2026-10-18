// GET /events[?q=text]: server-sent stream of every sensor update (name, values, ts).
#include <unistd.h>

#include "handler.h"
#include "metrics.h"

namespace andros {

void StreamEvents(int fd, const Request& req, const Resolvers& resolvers, Store* store) {
    const char* head =
        "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nCache-Control: no-cache\r\n"
        "Access-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n";
    if (!WriteAll(fd, head)) return;
    GetMetrics().streams++;
    const std::string q = QueryParam(req.query, "q");
    uint64_t seen[kSensorCount] = {};
    for (int idle = 0;; usleep(50000), ++idle) {
        for (int i = 0; i < kSensorCount; ++i) {
            const uint64_t w = store->Writes(i);
            if (w == seen[i]) continue;
            seen[i] = w;
            idle = 0;
            Value v = resolvers.Describe(i);
            if (!q.empty() && std::string(kSensors[i].name).find(q) == std::string::npos) continue;
            if (!WriteAll(fd, "data: " + v.Dump() + "\n\n")) { GetMetrics().streams--; return; }
        }
        if (idle >= 300 && idle % 300 == 0 && !WriteAll(fd, ": keep-alive\n\n")) {
            GetMetrics().streams--;
            return;
        }
    }
}

}  // namespace andros
