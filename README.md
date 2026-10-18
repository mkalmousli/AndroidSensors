# AndroidSensors

An Android 14 emulator image where every sensor (accelerometer, light, GPS, battery, ...) is a **real
system sensor**, and you read and set them through a **GraphQL API served by the OS itself**.
No helper apps. Apps, `SensorManager` and `dumpsys` see normal hardware.

```graphql
query    { accelerometer { x y z }  location { lat lon }  battery { level charging } }
mutation { setLocation(lat: 52.52, lon: 13.4) { ok }  setAccelerometer(x: 0, y: 9.81, z: 0) { ok } }
```

## 1. Use it (no clone, no build)

You need the [Android SDK](https://developer.android.com/studio) (the `emulator`) and a Linux machine with KVM,
and about 9 GB of free disk.

```sh
# 1. download and unpack the image into your SDK
U=https://github.com/mkalmousli/AndroidSensors/releases/latest/download
D=$ANDROID_HOME/system-images/android-34/andros
mkdir -p $D && curl -L $U/sdk-repo-linux-system-images-eng.root.zip.000 -o /tmp/andros.zip && unzip /tmp/andros.zip -d $D && rm /tmp/andros.zip

# 2. create the virtual device (avdmanager can't see a hand-unpacked image, so write two small files)
mkdir -p ~/.android/avd/andros.avd
printf 'path=%s\ntarget=android-34\n' ~/.android/avd/andros.avd > ~/.android/avd/andros.ini
cat > ~/.android/avd/andros.avd/config.ini <<EOF
AvdId=andros
abi.type=x86_64
hw.cpu.arch=x86_64
hw.ramSize=4096
hw.lcd.width=1080
hw.lcd.height=2400
hw.lcd.density=420
hw.gpu.enabled=yes
hw.gpu.mode=auto
disk.dataPartition.size=6G
image.sysdir.1=system-images/android-34/andros/x86_64/
tag.id=default
EOF

# 3. start it
emulator -avd andros
```

**Use the sensors:** open the settings page, or call the API (here: light sensor to 900 lux).
```sh
adb forward tcp:8000 tcp:8000
xdg-open http://localhost:8000/settings
curl localhost:8000/graphql -d '{"query":"mutation { setLight(lux: 900) { ok } }"}'
```

It is a normal emulator, so `scrcpy`, `adb` and your own Dockerfiles work as usual. If the download
is split into several parts (`.000`, `.001`, ...), join them first: `cat sdk-repo-*.zip.* > andros.zip`.

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

## How it works
`asp patch` copies [`overlay/andros`](overlay/andros/README.md) into the AOSP tree as `vendor/andros` and
defines the product `andros_x86_64`. **No stock source file is edited.**

```
graphqld ──writes──▶ /data/vendor/andros/sensors.store ◀──reads── sensors / GNSS / health HALs
   ▲ HTTP :8000                                                              │
GraphQL clients                                          SensorManager, LocationManager, BatteryManager
```

| Folder | Role |
|---|---|
| [`overlay/andros/store`](overlay/andros/store/README.md) | shared-memory sample store, one writer, seqlock readers |
| [`overlay/andros/graphqld`](overlay/andros/graphqld/README.md) | GraphQL daemon (HTTP + SSE) |
| [`overlay/andros/sensors_subhal`](overlay/andros/sensors_subhal/README.md) | all motion/environment sensors → `SensorService` |
| [`overlay/andros/gnss`](overlay/andros/gnss/README.md) | `location` → framework GPS provider |
| [`overlay/andros/health`](overlay/andros/health/README.md) | `battery` → `BatteryService` |
| [`overlay/andros/sepolicy`](overlay/andros/sepolicy/README.md) | SELinux policy |
| [`src/asp`](src/asp/README.md) | the `asp` pipeline (sync, patch, build, run, verify, cloud) |

## Add or change a sensor
Edit the list in `src/asp/sensors.py` (name, Android type id, fields, range, rate…). The C++ table, GraphQL
fields (`<name>`, `set<Name>`), HAL registration and feature flags are all generated. `make quick`
rebuilds only our modules.

All standard Android sensor types are covered (35 sensors plus location and battery). Detectors and
gestures are triggers: `mutation { triggerSignificantMotion { ok } }`.

**Settings page:** `http://localhost:8000/settings` (after `adb forward tcp:8000 tcp:8000`) shows status and metrics, a live
event stream, a searchable sensor table, and switches: data API on/off, each sensor on/off, and *dummy mode*
(the sensor produces synthetic values by itself). The same controls are GraphQL: `status`, `sensors(search:)`,
`setApiEnabled`, `setSensorEnabled`, `setSensorDummy`.

The device identity (brand, model, fingerprint, locale, timezone, battery size, vendor strings, extra
properties) is one dataclass in `src/asp/device.py` with dummy defaults. Put yours in git-ignored `src/asp/device_local.py` (copy `device_local.py.example`), run `asp patch`, rebuild.

## Development (no Android needed)
`make test` runs the Python tests, the C++ GraphQL/store/poller tests (host-built), ruff and the
100-lines-per-file check. `uv run asp --help` lists pipeline steps (`sync patch build run verify doctor`).

## Status
The image builds and boots on Android 14 (`android-14.0.0_r1`): the sensors, GNSS and health HALs run, the
stock ones are replaced, and the GraphQL API and settings page work. Not done yet: GNSS raw
measurements, ARM app translation, a native Android settings screen.

## License
GPL-3.0-or-later, see [LICENSE](LICENSE).

[uv]: https://docs.astral.sh/uv/
