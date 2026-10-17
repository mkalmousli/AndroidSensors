# andros: the vendor overlay

Becomes `vendor/andros` in the AOSP tree and defines the product `andros_x86_64`
(`andros_x86_64.mk`, inheriting the stock `sdk_phone64_x86_64`).

```
graphqld ──writes──▶ /data/vendor/andros/sensors.store ◀──reads── sensors HAL / GNSS HAL / health HAL
   ▲ HTTP :8000                                                              │
GraphQL clients                                    SensorManager, LocationManager, BatteryManager
```

| Folder | What it is |
|---|---|
| [store/](store/README.md) | shared-memory sample store (one writer, many readers) |
| [graphqld/](graphqld/README.md) | the GraphQL daemon (HTTP + SSE), the only writer |
| [sensors_subhal/](sensors_subhal/README.md) | sensors multi-HAL sub-HAL for all motion/environment sensors |
| [gnss/](gnss/README.md) | `IGnss` HAL, serves `location` |
| [health/](health/README.md) | `IHealth` HAL, serves `battery` |
| [sepolicy/](sepolicy/README.md) | SELinux rules and file contexts |

Other files: `Android.bp` (header library for the generated `gen/` files), `AndroidProducts.mk`
(lunch choices). `gen/` is created by `asp patch` from the catalog in `src/asp/sensors.py`.
