# src/asp

The `asp` command line (`uv run asp --help`): the pipeline that patches, builds, runs and publishes.

| Path | Role |
|---|---|
| `sensors.py` | **the sensor catalog** (Python data): edit it to add or change a sensor |
| `device.py` | **the device profile** (Python data): brand, model, fingerprint, locale, battery, vendor strings |
| `catalog.py` | `Sensor` model and validation |
| `render.py` | renders `templates/` from the catalog (C++ table, feature XMLs, makefile) |
| `config.py` | settings, overridable with `ASP_*` environment variables |
| `cli.py`, `sh.py` | entry point and subprocess helpers |
| [steps/](asp/steps/README.md) | pipeline steps |
| [cloud/](asp/cloud/README.md) | Google Cloud build launcher |
