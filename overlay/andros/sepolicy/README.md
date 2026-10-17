# sepolicy

- `andros_graphqld.te`: domain for the daemon and the `andros_data_file` type
- `hals.te`: lets the stock sensors, GNSS and health HAL domains read the store
- `file_contexts`: labels `/vendor/bin/graphqld` and `/data/vendor/andros`

Not yet verified on a real boot: expect to adjust after the first `avc: denied` in `logcat`.
