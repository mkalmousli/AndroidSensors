import pytest

from asp.config import Config
from asp.steps.patch import patch


@pytest.fixture
def cfg(tmp_path, monkeypatch):
    monkeypatch.setenv("ASP_WORKDIR", str(tmp_path))
    (tmp_path / "build").mkdir()
    (tmp_path / "build" / "envsetup.sh").write_text("")
    return Config()


def test_patch_requires_tree(tmp_path, monkeypatch):
    monkeypatch.setenv("ASP_WORKDIR", str(tmp_path))
    with pytest.raises(SystemExit):
        patch(Config())


def test_patch_installs_overlay_and_product(cfg):
    patch(cfg)
    v = cfg.vendor_dir
    assert (v / "graphqld" / "Android.bp").exists()
    assert (v / "gen" / "catalog_gen.h").exists()
    product = (v / "andros_x86_64.mk").read_text()
    assert "inherit-product" in product and "@REMOVE" not in product


def test_patch_is_idempotent(cfg):
    patch(cfg)
    patch(cfg)
    assert (cfg.vendor_dir / "gen" / "andros_features.mk").exists()
