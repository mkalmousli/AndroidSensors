// Host-built tests for the JSON/GraphQL engine and the shared store (no gtest needed).
#include <cstdio>
#include <cstdlib>
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

int main() {
    TestJson();
    TestParser();
    TestStoreAndResolvers();
    puts(failures ? "FAILED" : "all C++ tests passed");
    return failures ? 1 : 0;
}
