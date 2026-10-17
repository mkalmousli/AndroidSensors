"""Create, list and delete the temporary build VM."""
import time
from dataclasses import dataclass
from pathlib import Path

from ..catalog import ROOT
from .setup import SECRET
from .tools import gcloud

STARTUP = ROOT / "cloud" / "startup.py"
PREFIX = "asp-build"


@dataclass(frozen=True)
class VmSpec:
    project: str
    account: str
    repo: str
    repo_ref: str = "main"
    aosp_branch: str = "android-14.0.0_r1"
    zone: str = "us-central1-a"
    machine: str = "n2-highmem-8"
    disk_gb: int = 500
    max_hours: int = 14
    disk_type: str = "pd-standard"


def create(spec: VmSpec) -> str:
    name = f"{PREFIX}-{time.strftime('%Y%m%d-%H%M%S')}"
    meta = (f"asp-repo={spec.repo},asp-repo-ref={spec.repo_ref},asp-secret={SECRET},"
            f"asp-aosp-branch={spec.aosp_branch}")
    gcloud("compute", "instances", "create", name, "--project", spec.project,
           "--zone", spec.zone, "--machine-type", spec.machine,
           "--boot-disk-size", f"{spec.disk_gb}GB", "--boot-disk-type", spec.disk_type,
           "--image-family", "ubuntu-2204-lts", "--image-project", "ubuntu-os-cloud",
           "--service-account", spec.account, "--scopes", "cloud-platform",
           "--max-run-duration", f"{spec.max_hours}h", "--instance-termination-action", "DELETE",
           "--metadata-from-file", f"startup-script={Path(STARTUP)}",
           "--metadata", meta)
    return name


def list_names(project: str) -> list[tuple[str, str]]:
    out = gcloud("compute", "instances", "list", "--project", project,
                 f"--filter=name~^{PREFIX}-", "--format=value(name,zone.basename())")
    return [tuple(line.split()) for line in out.splitlines() if line.strip()]


def delete(project: str, name: str, zone: str) -> None:
    gcloud("compute", "instances", "delete", name, "--zone", zone, "--project", project, "--quiet")
