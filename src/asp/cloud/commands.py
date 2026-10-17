"""`asp cloud ...` actions: build, status, logs, cleanup, fetch."""
import subprocess
from pathlib import Path

from ..config import Config
from . import setup, vm
from .tools import gh


def build(args) -> None:
    proj = setup.project()
    repo = args.repo or setup.github_repo()
    setup.enable_apis(proj)
    setup.store_token(proj, setup.github_token())
    account = setup.compute_account(proj)
    setup.grant_roles(proj, account)
    spec = vm.VmSpec(proj, account, repo, args.ref, args.aosp_branch or Config().branch,
                     args.zone, args.machine, args.disk_gb, args.max_hours, args.disk_type)
    name = vm.create(spec)
    print(f"\nStarted {name}. It builds, publishes to https://github.com/{repo}/releases and "
          f"deletes itself.\nFollow it: uv run asp cloud logs   |   Status: uv run asp cloud status")


def status(args) -> None:
    proj = setup.project()
    print("VMs:", vm.list_names(proj) or "none running")
    print(gh("release", "list", "--limit", "5"))


def logs(args) -> None:
    proj = setup.project()
    vms = vm.list_names(proj)
    if not vms:
        raise SystemExit("No build VM is running.")
    name, zone = vms[-1]
    subprocess.run(["gcloud", "compute", "instances", "tail-serial-port-output", name,
                    "--zone", zone, "--project", proj], check=False)


def cleanup(args) -> None:
    proj = setup.project()
    for name, zone in vm.list_names(proj):
        vm.delete(proj, name, zone)


def fetch(args) -> None:
    """Download the newest build-* release and rejoin the split image zip."""
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    tags = gh("release", "list", "--limit", "20", "--json", "tagName", "-q", ".[].tagName")
    tag = next((t for t in tags.split() if t.startswith("build-")), None)
    if not tag:
        raise SystemExit("No build-* release found.")
    gh("release", "download", tag, "--dir", str(out), "--clobber")
    parts = sorted(out.glob("*.zip.[0-9][0-9][0-9]"))
    joined = out / parts[0].name.rsplit(".", 1)[0]
    with joined.open("wb") as dest:
        for part in parts:
            dest.write(part.read_bytes())
    print(f"Wrote {joined}")
