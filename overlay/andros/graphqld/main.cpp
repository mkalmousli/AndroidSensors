// graphqld: system daemon exposing the sensor store as a GraphQL API.
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <thread>

#include "handler.h"

using namespace andros;

static int Listen(int port) {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(uint16_t(port));
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof addr) != 0 || listen(fd, 16) != 0) {
        perror("graphqld: bind/listen");
        return -1;
    }
    return fd;
}

int main(int argc, char** argv) {
    int port = argc > 1 ? atoi(argv[1]) : 8000;
    const char* path = argc > 2 ? argv[2] : kDefaultPath;
    Store* store = Store::Open(path, /*writer=*/true);
    if (!store) { perror("graphqld: open store"); return 1; }
    int server = Listen(port);
    if (server < 0) return 1;
    Resolvers resolvers(store);
    for (;;) {
        int client = accept4(server, nullptr, nullptr, SOCK_CLOEXEC);
        if (client < 0) continue;
        std::thread([client, &resolvers, store] {
            Request req;
            if (ReadRequest(client, &req)) Handle(client, req, resolvers, store);
            close(client);
        }).detach();
    }
}
