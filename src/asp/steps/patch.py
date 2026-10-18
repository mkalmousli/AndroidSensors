"""Patch the AOSP tree: drop our overlay in as vendor/andros and rewire the product.

Nothing in the stock tree is edited. The overlay defines a new product
(andros_x86_64) that inherits the stock emulator product and then swaps the
stock sensors/GNSS/health HALs for ours. The step is idempotent.
"""
import shutil

from ..catalog import ROOT, load
from ..config import Config
from ..render import render_all

OVERLAY = ROOT / "overlay" / "andros"


def _copy_overlay(cfg: Config) -> None:
    if cfg.vendor_dir.exists():
        shutil.rmtree(cfg.vendor_dir)
    shutil.copytree(OVERLAY, cfg.vendor_dir)


def patch(cfg: Config) -> None:
    if not (cfg.workdir / "build" / "envsetup.sh").exists():
        raise SystemExit(f"No AOSP tree in {cfg.workdir}; run `asp sync` first.")
    _copy_overlay(cfg)
    render_all(cfg.vendor_dir / "gen", load())
    print(f"Patched {cfg.workdir}: vendor/andros installed, product {cfg.lunch} ready.")
