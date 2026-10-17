"""Structural checks on the overlay so a catalog change cannot silently break the build."""
from asp.catalog import ROOT, load

OVERLAY = ROOT / "overlay" / "andros"
MODULES = ["andros_graphqld", "andros_sensors_subhal", "andros_health", "andros_gnss"]


def test_every_product_package_has_a_soong_module():
    blobs = "".join(p.read_text() for p in OVERLAY.rglob("Android.bp"))
    for name in MODULES:
        assert f'name: "{name}"' in blobs


def test_product_installs_all_modules():
    product = (OVERLAY / "andros_x86_64.mk").read_text()
    for name in MODULES:
        assert name in product


def test_each_hal_has_a_catalog_entry():
    assert {s.hal for s in load()} >= {"sensors", "gnss", "health"}
