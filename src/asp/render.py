"""Render catalog-driven files (C++ table, feature XMLs, makefile) from templates."""
from pathlib import Path

from jinja2 import Environment, FileSystemLoader, StrictUndefined

from .catalog import ROOT, Sensor, load

TEMPLATES = ROOT / "templates"


def _env() -> Environment:
    return Environment(
        loader=FileSystemLoader(TEMPLATES),
        undefined=StrictUndefined,
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=True,
    )


def feature_names(sensors: list[Sensor]) -> list[str]:
    """Sensors HAL entries may declare an android.hardware.sensor.* feature."""
    return sorted({f"android.hardware.sensor.{s.feature}" for s in sensors if s.feature})


def render_all(out: Path, sensors: list[Sensor] | None = None) -> list[Path]:
    sensors = sensors if sensors is not None else load()
    env = _env()
    out.mkdir(parents=True, exist_ok=True)
    written = []

    header = out / "catalog_gen.h"
    header.write_text(env.get_template("catalog_gen.h.j2").render(sensors=sensors))
    written.append(header)

    features = feature_names(sensors)
    (out / "features").mkdir(exist_ok=True)
    for feat in features:
        path = out / "features" / f"{feat}.xml"
        path.write_text(env.get_template("feature.xml.j2").render(feature=feat))
        written.append(path)

    mk = out / "andros_features.mk"
    mk.write_text(env.get_template("andros_features.mk.j2").render(features=features))
    written.append(mk)
    return written
