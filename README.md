# AndroidSensors

An Android 14 emulator image where every sensor (accelerometer, light, GPS, battery, ...) is a **real
system sensor**, and you read and set them through a **GraphQL API served by the OS itself**.
No helper apps. Apps, `SensorManager` and `dumpsys` see normal hardware.

```graphql
query    { accelerometer { x y z }  location { lat lon }  battery { level charging } }
mutation { setLocation(lat: 52.52, lon: 13.4) { ok }  setAccelerometer(x: 0, y: 9.81, z: 0) { ok } }
```

## 1. Use it (no clone, no build)

You need the [Android SDK](https://developer.android.com/studio) (for `emulator` and `avdmanager`) and a
Linux machine with KVM.

**Download** the latest build. It comes in parts because GitHub limits files to 2 GB:
```sh
mkdir andros && cd andros
curl -s https://api.github.com/repos/mkalmousli/AndroidSensors/releases/latest \
  | grep -o 'https://[^"]*releases/download[^"]*' | xargs -n1 curl -LO
```

**Join** the parts and unpack them into your SDK:
```sh
cat sdk-repo-*.zip.* > andros.zip
unzip andros.zip -d $ANDROID_HOME/system-images/android-34/andros
```

**Create and start** the virtual device:
```sh
avdmanager create avd -n andros -k "system-images;android-34;andros;x86_64"
emulator -avd andros
```

**Use the sensors** through GraphQL (here: set the light sensor to 900 lux):
```sh
adb forward tcp:8000 tcp:8000
curl localhost:8000/graphql -d '{"query":"mutation { setLight(lux: 900) { ok } }"}'
```

It is a normal emulator, so `scrcpy`, `adb` and your own Dockerfiles work as usual.

## 2. Build it yourself

The build downloads AOSP and takes hours (8+ cores, 32+ GB RAM, ~400 GB disk). Pick one place to run it.

### On Google Cloud (easiest, a few dollars, runs unattended)
Needs `gcloud` and `gh` logged in (`gcloud auth login`, `gcloud config set project <id>`, `gh auth login`),
and [uv].
```sh
git clone https://github.com/mkalmousli/AndroidSensors && cd AndroidSensors
uv run asp cloud build      # starts a VM that builds, publishes a GitHub release, then deletes itself
uv run asp cloud logs       # follow it  (also: asp cloud status, asp cloud cleanup)
```
It takes about 8-12 hours and costs roughly $5-8. A 14 h limit deletes a stuck VM. If the build fails,
a `failed-*` release with the log is published instead. Options: `--machine`, `--zone`, `--disk-gb`,
`--disk-type` (default `pd-standard`; SSD types need more quota), `--ref`, `--aosp-branch`.

### On your own Linux machine (Docker)
Needs Docker and `/dev/kvm`.
```sh
git clone https://github.com/mkalmousli/AndroidSensors && cd AndroidSensors
make build     # sync + patch + build AOSP inside Docker (cached between runs)
make run       # boot it: console http://localhost:8000, adb localhost:5555
make verify    # end-to-end check
```

### On another cloud or server
Any Linux box with Docker works like the local option above. To also publish the result to GitHub:
```sh
docker build -t androidsensors .
docker run --rm -e ASP_GITHUB_TOKEN=<token with repo scope> -e ASP_REPO=<owner>/<repo> \
  -v aosp:/aosp -v ccache:/ccache androidsensors asp release
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
