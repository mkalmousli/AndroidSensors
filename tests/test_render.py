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
    assert "_DEVICE" not in mk  # empty fields keep the AOSP default
    h = (tmp_path / "device_gen.h").read_text()
    assert '"Acme Sensors"' in h and "kBatteryMah = 5000" in h


def test_device_codename_never_sets_product_device(tmp_path):
    from asp.device import Device
    from asp.render import render_all

    render_all(tmp_path, device=Device(device="x1"))
    mk = (tmp_path / "device.mk").read_text()
    assert "\nPRODUCT_DEVICE" not in mk and "PRODUCT_SYSTEM_DEVICE := x1" in mk


def test_every_device_field_is_rendered(tmp_path):
    import dataclasses

    from asp.device import Device
    from asp.render import render_all

    values = {f.name: f"v_{f.name}" for f in dataclasses.fields(Device) if f.type is str or f.type == "str"}
    values.update(battery_mah=1234, properties=(("ro.k", "v_prop"),))
    render_all(tmp_path, device=Device(**values))
    out = (tmp_path / "device.mk").read_text() + (tmp_path / "device_gen.h").read_text()
    missing = [k for k, v in values.items() if k != "properties" and str(v) not in out]
    assert not missing and "v_prop" in out, f"ignored fields: {missing}"


def test_locale_uses_underscore_for_aapt(tmp_path):
    from asp.device import Device
    render_all(tmp_path, device=Device(locale="de-DE"))
    assert "PRODUCT_LOCALES := de_DE" in (tmp_path / "device.mk").read_text()
