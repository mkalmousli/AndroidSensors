import pytest

from asp.catalog import Sensor, _validate, load


def test_catalog_loads_and_is_consistent():
    sensors = load()
    assert {s.hal for s in sensors} == {"sensors", "gnss", "health"}
    assert len({s.name for s in sensors}) == len(sensors)


def test_every_sensor_has_matching_init():
    for s in load():
        assert len(s.init) == len(s.fields)


def test_invalid_entries_rejected():
    bad = Sensor(name="x", hal="sensors", fields=["a"], init=[0], type=0)
    with pytest.raises(ValueError):
        _validate(bad)
    with pytest.raises(ValueError):
        _validate(Sensor(name="x", hal="nope", fields=["a"], init=[0]))


def test_every_standard_sensor_type_is_covered():
    from asp.catalog import load

    types = {s.type for s in load() if s.hal == "sensors"}
    # 1..31, 34..36 minus the two meta types (32 dynamic meta, 33 additional info)
    assert set(range(1, 32)) | {34, 35, 36} <= types


def test_triggers_and_wakeups():
    from asp.catalog import load

    by = {s.name: s for s in load()}
    assert by["proximity"].wakeup and by["significantMotion"].mode == "oneshot"
    assert by["stepDetector"].mode == "special" and len(by["pose6dof"].fields) == 15


def test_private_sensors_have_type_strings():
    from asp.catalog import load
    private = [s for s in load() if s.type >= 0x10000]
    assert {s.name for s in private} == {"activity", "floors"}
    assert all(s.type_string.startswith("android.sensor.") for s in private)
