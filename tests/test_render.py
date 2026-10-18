from asp.catalog import load
from asp.render import feature_names, render_all


def test_render_creates_expected_files(tmp_path):
    written = render_all(tmp_path)
    names = {p.name for p in written}
    assert "catalog_gen.h" in names and "andros_features.mk" in names
    header = (tmp_path / "catalog_gen.h").read_text()
    assert f"kSensorCount = {len(load())}" in header
    assert '"accelerometer"' in header and "Hal::kGnss" in header


def test_features_are_unique_and_declared(tmp_path):
    feats = feature_names(load())
    assert len(feats) == len(set(feats))
    render_all(tmp_path)
    for f in feats:
        assert f'name="{f}"' in (tmp_path / "features" / f"{f}.xml").read_text()


def test_device_profile_is_rendered(tmp_path):
    from asp.device import Device
    from asp.render import render_all

    d = Device(brand="Acme", model="Z1", device="", board="", fingerprint="acme/z1/z1:14/X/1:user/release-keys",
               sensor_vendor="Acme Sensors", battery_mah=5000, properties=(("ro.foo", "bar"),))
    render_all(tmp_path, device=d)
    mk = (tmp_path / "device.mk").read_text()
    assert "PRODUCT_BRAND := Acme" in mk and "BUILD_FINGERPRINT=acme/z1" in mk and "ro.foo=bar" in mk
    assert "PRODUCT_DEVICE" not in mk  # empty fields keep the AOSP default
    h = (tmp_path / "device_gen.h").read_text()
    assert '"Acme Sensors"' in h and "kBatteryMah = 5000" in h
