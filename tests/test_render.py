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
