// Settings API: search, per-sensor and global switches, dummy flag.
#include <cstdio>
#include <unistd.h>

#include "resolvers.h"

using namespace andros;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); ++failures; } } while (0)

static std::string Run(Resolvers& r, const std::string& q, const std::string& vars = "{}") {
    Value v;
    Operation op;
    std::string err;
    ParseJson(vars, &v);
    if (!ParseOperation(q, v, &op, &err)) return "ERR:" + err;
    return Execute(op, r).Dump();
}

static void TestSettings() {
    unlink("build/settings.store");
    Store* st = Store::Open("build/settings.store", true);
    Resolvers r(st);
    CHECK(Run(r, "{ sensors(search: \"accel\") { name } }").find("accelerometer") != std::string::npos);
    CHECK(Run(r, "{ sensors(search: \"accel\") { name } }").find("light") == std::string::npos);
    CHECK(Run(r, "{ status { apiEnabled sensors } }").find("\"apiEnabled\":true") != std::string::npos);
    Run(r, "mutation { setSensorEnabled(name: \"light\", enabled: false) { ok } }");
    CHECK(Run(r, "mutation { setLight(lux: 5) { ok } }").find("disabled") != std::string::npos);
    Run(r, "mutation { setSensorEnabled(name: \"light\", enabled: true) { ok } }");
    CHECK(Run(r, "mutation { setLight(lux: 5) { ok } }").find("\"ok\":true") != std::string::npos);
    CHECK(Run(r, "{ light { enabled dummy writes } }").find("\"writes\":1") != std::string::npos);
    Run(r, "mutation { setApiEnabled(enabled: false) { ok } }");
    CHECK(Run(r, "{ light { lux } }").find("disabled in settings") != std::string::npos);
    CHECK(Run(r, "{ status { apiEnabled } }").find("false") != std::string::npos);  // settings stay reachable
    Run(r, "mutation { setApiEnabled(enabled: true) { ok } }");
    CHECK(Run(r, "mutation { setSensorDummy(search: \"light\", dummy: true) { changed } }").find("\"changed\":1") != std::string::npos);
    CHECK(st->Flags(FindSensor("light")) & kDummy);
}

int main() {
    TestSettings();
    puts(failures ? "FAILED" : "settings tests passed");
    return failures ? 1 : 0;
}
