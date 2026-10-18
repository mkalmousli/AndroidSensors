"""End-to-end check: the sensors exist in the OS and follow GraphQL mutations."""
import json
import time
import urllib.request

from ..catalog import load
from ..config import Config
from ..device import DEVICE
from ..sh import capture


def gql(cfg: Config, query: str) -> dict:
    req = urllib.request.Request(
        f"http://127.0.0.1:{cfg.http_port}/graphql",
        data=json.dumps({"query": query}).encode(),
        headers={"Content-Type": "application/json"},
    )
    with urllib.request.urlopen(req, timeout=10) as resp:
        return json.load(resp)


def _check(ok: bool, label: str) -> bool:
    print(("PASS " if ok else "FAIL ") + label)
    return ok


def verify(cfg: Config) -> bool:
    results = []
    dump = capture(["adb", "shell", "dumpsys", "sensorservice"])
    for s in (s for s in load() if s.hal == "sensors"):
        listed = f"{DEVICE.sensor_vendor} {s.name}".lower() in dump.lower()
        results.append(_check(listed, f"sensorservice lists {s.name}"))
    gql(cfg, 'mutation { setAmbientTemperature(celsius: 31) { ok } }')
    time.sleep(1)
    out = gql(cfg, "{ ambientTemperature { celsius } }")
    results.append(_check(out["data"]["ambientTemperature"]["celsius"] == 31, "GraphQL round-trip"))
    gql(cfg, "mutation { setBattery(level: 42) { ok } }")
    time.sleep(2)
    battery = capture(["adb", "shell", "dumpsys", "battery"])
    results.append(_check("level: 42" in battery, "framework sees battery level 42"))
    return all(results)
