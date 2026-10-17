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
