// Host-built tests for the JSON/GraphQL engine and the shared store (no gtest needed).
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "resolvers.h"
#include "route.h"

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

static void TestJson() {
    Value v;
    CHECK(ParseJson(R"({"a":[1,2.5,"x\n"],"b":{"c":true}})", &v));
    CHECK(v.Get("a")->a.size() == 3);
    CHECK(v.Dump() == R"({"a":[1,2.5,"x\n"],"b":{"c":true}})");
    CHECK(!ParseJson("{\"a\":", &v));
}

static void TestParser() {
    Operation op;
    std::string err;
    CHECK(ParseOperation("mutation M($x: Float!) { setLight(lux: $x) { ok } }", [] {
        Value v; ParseJson(R"({"x":7})", &v); return v; }(), &op, &err));
    CHECK(op.kind == "mutation" && op.selection[0].Arg("lux")->n == 7);
    CHECK(!ParseOperation("{ a( }", Value::Obj(), &op, &err));
    CHECK(!ParseOperation("frobnicate { a }", Value::Obj(), &op, &err));
}

static void TestStoreAndResolvers() {
    char path[] = "/tmp/andros_testXXXXXX";
    close(mkstemp(path));
    Store* writer = Store::Open(path, true);
    Store* reader = Store::Open(path, false);
    CHECK(writer && reader);
    Resolvers r(writer);
    CHECK(Run(r, "{ light { lux } }") == R"({"data":{"light":{"lux":400}}})");
    Run(r, "mutation { setLight(lux: 123) { ok } }");
    Sample s;
    CHECK(reader->Read(FindSensor("light"), &s) && s.v[0] == 123 && s.ts_ns > 0);
    CHECK(Run(r, "{ nope }").find("errors") != std::string::npos);
    CHECK(Run(r, "{ a: accelerometer { y } }") == R"({"data":{"a":{"y":9.81}}})");
    unlink(path);
}

static void TestRoute() {
    Pt a{52.0, 13.0, 100}, b{52.0, 13.001, 100};
    double d = HaversineM(a, b);
    CHECK(d > 65 && d < 70);                               // ~68 m per 0.001 deg lon at 52N
    CHECK(BearingDeg(a, b) > 89 && BearingDeg(a, b) < 91);  // due east
    CHECK(PressureAt(0) > 1013 && PressureAt(100) < PressureAt(0));
    Route r({a, b, {52.0, 13.002, 130}}, 1.4, false);
    for (int i = 0; i < 200; ++i) r.Advance(1.0);           // 136 m total: reaches the end
    CHECK(r.Done() && r.Now().steps > 50);
    CHECK(r.Now().floors >= 9 && r.Now().activity == "still");  // climbed 30 m = 10 floors
    Route w({a, b}, 1.4, true);
    w.Advance(1.0);
    CHECK(std::string(w.Now().activity) == "walking" && w.Now().cadence_hz > 1.5);
    CHECK(std::string(ActivityFor(3.0)) == "running" && std::string(ActivityFor(20)) == "vehicle");
}

int main() {
    TestRoute();
    TestJson();
    TestParser();
    TestStoreAndResolvers();
    puts(failures ? "FAILED" : "all C++ tests passed");
    return failures ? 1 : 0;
}
