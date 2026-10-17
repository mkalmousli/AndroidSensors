import argparse

import pytest

from asp.cli import main
from asp.cloud import commands, setup, vm


class FakeGcloud:
    def __init__(self):
        self.calls, self.stdin = [], []

    def __call__(self, *args, stdin=None):
        self.calls.append(args)
        self.stdin.append(stdin)
        if args[:2] == ("projects", "describe"):
            return "123\n"
        if args[:2] == ("secrets", "describe"):
            raise SystemExit("not found")
        return ""


@pytest.fixture
def fake(monkeypatch):
    g = FakeGcloud()
    for mod in (setup, vm):
        monkeypatch.setattr(mod, "gcloud", g)
    monkeypatch.setattr(setup, "run", lambda *a, **k: "my-proj\n")
    monkeypatch.setattr(setup, "gh", lambda *a, **k: "owner/repo\n" if "view" in a else "ghp_tok\n")
    return g


def build_args(**over):
    base = {"repo": None, "ref": "main", "aosp_branch": "android-14.0.0_r1",
            "zone": "us-central1-a", "machine": "n2-highmem-8", "disk_gb": 500, "max_hours": 14}
    return argparse.Namespace(**{**base, **over})


def test_build_runs_full_setup_then_creates_vm(fake):
    commands.build(build_args())
    flat = [" ".join(c) for c in fake.calls]
    assert any("services enable" in c for c in flat)
    assert any(c.startswith("secrets create") for c in flat)  # created because describe failed
    assert "ghp_tok" in fake.stdin  # token goes to Secret Manager via stdin, never argv
    assert not any("ghp_tok" in c for c in flat)
    assert sum("add-iam-policy-binding" in c for c in flat) == 2
    create = next(c for c in flat if c.startswith("compute instances create"))
    assert "--max-run-duration 14h" in create and "asp-repo=owner/repo" in create
    assert "123-compute@developer.gserviceaccount.com" in create


def test_unset_project_is_a_clear_error(monkeypatch):
    monkeypatch.setattr(setup, "run", lambda *a, **k: "(unset)\n")
    with pytest.raises(SystemExit, match="gcloud config set project"):
        setup.project()


def test_cli_exposes_cloud_commands(capsys):
    with pytest.raises(SystemExit):
        main(["cloud", "--help"])
    out = capsys.readouterr().out
    assert all(word in out for word in ("build", "status", "logs", "cleanup", "fetch"))
