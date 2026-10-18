"""Sensor model and validation; the data itself lives in sensors.py."""
from dataclasses import dataclass, field, replace
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HALS = ("sensors", "gnss", "health")
MODES = ("continuous", "onchange", "special", "oneshot")
MAX_VALUES = 15  # pose6dof is the widest


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
    wakeup: bool = False
    type_string: str = ""  # required for vendor-private types (>= 0x10000)
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
    if s.type >= 0x10000 and not s.type_string:
        raise ValueError(f"{s.name}: private sensor types need a type_string")


def load(sensors: list[Sensor] | None = None) -> list[Sensor]:
    """Return the validated catalog (index = position in the list)."""
    if sensors is None:
        from .sensors import SENSORS as sensors
    result = [replace(s, index=i) for i, s in enumerate(sensors)]
    if len({s.name for s in result}) != len(result):
        raise ValueError("duplicate sensor names in catalog")
    for s in result:
        _validate(s)
    return result
