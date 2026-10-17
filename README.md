# AndroidSensors

Patches AOSP so every sensor is a **real system sensor** (HAL level, visible to `SensorManager`,
`LocationManager`, `BatteryManager`, `dumpsys`) and is **read and written through a GraphQL API served by
the OS itself**. No extra APKs, no helper apps, no third-party Docker images. Python + [uv] drive
everything; the whole pipeline runs in a Dockerfile of our own (plain Ubuntu).

```
App → framework → sensors / GNSS / health HAL ─┐   (read-only mmap)
                                               ├── /data/vendor/andros/sensors.store
GraphQL client ⇄ graphqld (vendor init service) ┘   (single writer, seqlock)
```

## Quick start
Needs a Linux host with Docker, `/dev/kvm`, ~300 GB disk and 32 GB RAM (first build takes hours).

```sh
make build     # repo sync + patch + build AOSP inside Docker (cached in volumes)
make run       # boot the emulator;  http://localhost:8000  (console)  adb: localhost:5555
make verify    # end-to-end check against the booted device
```

```graphql
query    { accelerometer { x y z }  location { lat lon alt }  battery { level charging }  sensors { name values } }
mutation { setLocation(lat: 52.52, lon: 13.4) { ok }  setAccelerometer(x: 0, y: 9.81, z: 0) { ok }
           setBattery(level: 42, charging: 1) { ok }  setSensor(name: "light", values: [800]) { ok } }
subscription { light { lux } }      # server-sent events: GET /graphql?query=subscription{light{lux}}
```
```sh
curl -s localhost:8000/graphql -d '{"query":"mutation { setLight(lux: 900) { ok } }"}'
adb shell dumpsys sensorservice | grep -i andros
```

## What gets patched
`asp patch` installs `overlay/andros` as `vendor/andros` in the AOSP tree and defines the product
`andros_x86_64` (inherits `sdk_phone64_x86_64`). **No stock source file is edited.**

| Piece | Where | Role |
|---|---|---|
| `store/` | static lib | shared-memory sample store, one writer, seqlock readers |
| `graphqld/` | vendor init service | GraphQL (HTTP + SSE), generated from the catalog |
| `sensors_subhal/` | multihal sub-HAL (`hals.conf`) | all motion/environment sensors → `SensorService` |
| `gnss/` | `IGnss/default` | `location` → framework GPS provider |
| `health/` | `IHealth/default` | `battery` → `BatteryService` |
| `sepolicy/`, `gen/` | policy, feature XMLs | `android.hardware.sensor.*` features, SELinux |

## Add or change a sensor
Edit `catalog/sensors.yaml` (name, Android type id, fields, range, rate…). The C++ table, GraphQL
fields (`<name>`, `set<Name>`), HAL registration and feature flags are all generated. `make quick`
rebuilds only our modules.

## Development (no Android needed)
`make test` runs the Python tests, the C++ GraphQL/store/poller tests (host-built), ruff and the
100-lines-per-file check. `uv run asp --help` lists pipeline steps (`sync patch build run verify doctor`).

## Status / honesty
Verified here: catalog rendering, patch step, GraphQL engine, store, poller (host unit tests).
**Not yet verified on a real AOSP build** (needs KVM + hours): the HIDL/AIDL HAL glue
(`sensors_subhal`, `gnss`, `health`), Soong module dependency names, SELinux rules and the
`ASP_REMOVE_PACKAGES` list for the pinned tag (`android-14.0.0_r1`, override with `ASP_BRANCH`).
Expect a few iterations on the first build; `asp doctor` lists the stock modules to replace.

[uv]: https://docs.astral.sh/uv/
