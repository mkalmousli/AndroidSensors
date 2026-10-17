# tests

- Python (`pytest`): catalog, rendering, patch step, overlay structure, release packaging, cloud launcher
- `cpp/`: host-built tests for the GraphQL engine and the sensor poller, with Android header stubs in `cpp/stubs/`

`make test` runs both plus lint. CI (`.github/workflows/test.yml`) does the same on every push.
