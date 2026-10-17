# sensors_subhal

Sub-HAL for the stock multihal sensors service (registered in `hals.conf`). Exposes every catalog
entry with `hal: "sensors"` as an Android sensor.

- `sensor_table` builds the HIDL `SensorInfo` list from the catalog
- `poller` turns store samples into events at the requested rates (activate, batch, flush)
- `andros_subhal` / `entry` implement and export the sub-HAL

Host-tested by `tests/cpp/test_poller.cpp` (`make test-poller`).
