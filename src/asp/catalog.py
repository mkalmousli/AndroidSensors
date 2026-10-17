"""Load and validate the declarative sensor catalog."""
from dataclasses import dataclass, field
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parents[2]
CATALOG_FILE = ROOT / "catalog" / "sensors.yaml"
HALS = ("sensors", "gnss", "health")
MODES = ("continuous", "onchange", "special")
MAX_VALUES = 6


@dataclass(frozen=True)
class Sensor:
    name: str
    hal: str
    fields: list[str]
    init: list[float]
    unit: str = ""
    type: int = 0
    mode: str = "continuous"
    max_range: float = 0.0
    resolution: float = 0.0
    power: float = 0.0
    min_delay_us: int = 0
    feature: str = ""
    index: int = field(default=0, compare=False)

    @property
    def cap(self) -> str:
        return self.name[0].upper() + self.name[1:]


def _validate(s: Sensor) -> None:
    if s.hal not in HALS:
        raise ValueError(f"{s.name}: unknown hal {s.hal!r}")
    if s.mode not in MODES:
        raise ValueError(f"{s.name}: unknown mode {s.mode!r}")
    if not 0 < len(s.fields) <= MAX_VALUES or len(s.init) != len(s.fields):
        raise ValueError(f"{s.name}: fields/init must match and be 1..{MAX_VALUES}")
    if s.hal == "sensors" and s.type <= 0:
        raise ValueError(f"{s.name}: sensors HAL entries need an Android type id")


def load(path: Path = CATALOG_FILE) -> list[Sensor]:
    raw = yaml.safe_load(path.read_text())["sensors"]
    sensors = [Sensor(index=i, **entry) for i, entry in enumerate(raw)]
    names = [s.name for s in sensors]
    if len(set(names)) != len(names):
        raise ValueError("duplicate sensor names in catalog")
    for s in sensors:
        _validate(s)
    return sensors
