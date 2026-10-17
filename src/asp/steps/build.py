"""Build the patched system image and the emulator package."""
from ..config import Config
from ..sh import in_aosp

QUICK_MODULES = "andros_graphqld andros_sensors_subhal andros_health andros_gnss"


def build(cfg: Config, quick: bool = False) -> None:
    target = QUICK_MODULES if quick else ""
    in_aosp(cfg, f"m -j{cfg.jobs} {target}".strip())
