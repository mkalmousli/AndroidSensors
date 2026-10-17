# templates

Jinja2 templates rendered by `src/asp/render.py` from the catalog in `src/asp/sensors.py`:
`catalog_gen.h.j2` (C++ sensor table), `feature.xml.j2` (one `android.hardware.sensor.*` permission
file per feature), `andros_features.mk.j2` (product makefile that installs them).
