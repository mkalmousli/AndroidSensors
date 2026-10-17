import json

from asp.steps import publish as pub
from asp.steps.package import checksums, split


def test_split_roundtrip(tmp_path):
    src = tmp_path / "img.zip"
    data = bytes(range(256)) * 100
    src.write_bytes(data)
    parts = split(src, tmp_path / "dist", part_bytes=10_000)
    assert [p.name for p in parts] == ["img.zip.000", "img.zip.001", "img.zip.002"]
    assert b"".join(p.read_bytes() for p in parts) == data


def test_checksums(tmp_path):
    f = tmp_path / "a"
    f.write_bytes(b"abc")
    line = checksums([f], tmp_path / "SUMS").read_text()
    assert line.startswith("ba7816bf") and line.strip().endswith("  a")


def test_publish_creates_release_and_uploads(tmp_path, monkeypatch):
    calls = []

    def fake_call(req, retries=3):
        calls.append((req.full_url, req.get_method()))
        return {"id": 7, "html_url": "https://example/release"}

    monkeypatch.setattr(pub, "_call", fake_call)
    monkeypatch.setenv("ASP_GITHUB_TOKEN", "t")
    monkeypatch.setenv("ASP_TAG", "build-x")
    f = tmp_path / "SHA256SUMS"
    f.write_text("x")
    assert pub.publish([f]) == "https://example/release"
    assert calls[0][0].endswith("/repos/mkalmousli/AndroidSensors/releases")
    assert calls[1][0].endswith("/releases/7/assets?name=SHA256SUMS")
    assert json.dumps(calls)
