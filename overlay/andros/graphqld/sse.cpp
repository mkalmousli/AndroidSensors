#include <unistd.h>

#include <cstdint>

#include "handler.h"

namespace andros {

constexpr useconds_t kPollUs = 20000;
constexpr int kKeepAliveTicks = 750;  // ~15 s

void StreamSubscription(int fd, const Operation& op, const Resolvers& resolvers, Store* store) {
    const char* head =
        "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nCache-Control: no-cache\r\n"
        "Access-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n";
    if (!WriteAll(fd, head)) return;
    uint64_t seen = ~0ull;  // forces an initial event
    for (int idle = 0;; usleep(kPollUs), ++idle) {
        uint64_t now = store->Generation();
        if (now != seen) {
            seen = now;
            idle = 0;
            if (!WriteAll(fd, "data: " + Execute(op, resolvers).Dump() + "\n\n")) return;
        } else if (idle >= kKeepAliveTicks) {
            idle = 0;
            if (!WriteAll(fd, ": keep-alive\n\n")) return;
        }
    }
}

}  // namespace andros
