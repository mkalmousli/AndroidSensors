"""Device profile: the identity this system presents. Defaults describe the Andros ROM itself.

Put your own values in `src/asp/device_local.py` (git-ignored; copy `device_local.py.example`).
If it exists, its `DEVICE` replaces the dummy one. Then run `asp patch` and rebuild.

Every field is rendered (templates/device.mk.j2, device_gen.h.j2) and a test enforces that no
field is ignored. Empty strings mean "keep the build's own value".
"""
import importlib
from dataclasses import dataclass


@dataclass(frozen=True)
class Device:
    # --- identity (android.os.Build.*) ---
    brand: str = "andros"             # Build.BRAND
    manufacturer: str = "Andros"      # Build.MANUFACTURER
    model: str = "Andros Emulator" # Build.MODEL
    marketing_name: str = "Andros Emulator" # ro.product.marketname (Settings > About)
    device: str = "andros_x86_64"     # Build.DEVICE codename (set per partition, not PRODUCT_DEVICE)
    board: str = "andros"             # Build.BOARD
    # --- build strings ---
    fingerprint: str = ""             # Build.FINGERPRINT
    description: str = ""             # ro.build.description
    display_id: str = ""              # Build.DISPLAY
    # --- region ---
    locale: str = "en-US"             # default system locale (BCP-47)
    timezone: str = "UTC"             # persist.sys.timezone
    # --- what the HALs report ---
    sensor_vendor: str = "Andros"     # vendor string on every sensor
    gnss_name: str = "Andros GNSS"     # GNSS system-info name
    battery_mah: int = 4000           # battery capacity (health HAL)
    battery_technology: str = "Li-ion"
    # --- extra system properties, written verbatim as ro.* / persist.* ---
    properties: tuple[tuple[str, str], ...] = ()


def _load() -> Device:
    try:  # your private profile; never committed
        return importlib.import_module(".device_local", __package__).DEVICE
    except ModuleNotFoundError:
        return Device()


DEVICE = _load()
