"""Runtime configuration, overridable through ASP_* environment variables."""
import os
from dataclasses import dataclass, field
from pathlib import Path


def _env(key: str, default: str) -> str:
    return os.environ.get(f"ASP_{key}", default)


@dataclass(frozen=True)
class Config:
    branch: str = field(default_factory=lambda: _env("BRANCH", "android-14.0.0_r1"))
    manifest_url: str = field(
        default_factory=lambda: _env(
            "MANIFEST_URL", "https://android.googlesource.com/platform/manifest"
        )
    )
    workdir: Path = field(default_factory=lambda: Path(_env("WORKDIR", "/aosp")))
    product: str = field(default_factory=lambda: _env("PRODUCT", "andros_x86_64"))
    variant: str = field(default_factory=lambda: _env("VARIANT", "userdebug"))
    jobs: int = field(default_factory=lambda: int(_env("JOBS", str(os.cpu_count() or 4))))
    http_port: int = field(default_factory=lambda: int(_env("HTTP_PORT", "8000")))
    adb_port: int = field(default_factory=lambda: int(_env("ADB_PORT", "5555")))
    sync_jobs: int = field(default_factory=lambda: int(_env("SYNC_JOBS", "8")))

    @property
    def vendor_dir(self) -> Path:
        return self.workdir / "vendor" / "andros"


    @property
    def lunch(self) -> str:
        return f"{self.product}-{self.variant}"
