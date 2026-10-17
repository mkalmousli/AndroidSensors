"""Tiny subprocess helpers shared by all steps."""
import subprocess
from pathlib import Path


def run(cmd: list[str] | str, cwd: Path | None = None, shell: bool = False) -> None:
    shown = cmd if isinstance(cmd, str) else " ".join(cmd)
    print(f"+ {shown}", flush=True)
    subprocess.run(cmd, cwd=cwd, shell=shell, check=True, executable="/bin/bash" if shell else None)


def capture(cmd: list[str], cwd: Path | None = None) -> str:
    return subprocess.run(cmd, cwd=cwd, check=True, capture_output=True, text=True).stdout


def in_aosp(cfg, script: str) -> None:
    """Run a shell snippet after sourcing envsetup + lunch of our product."""
    full = f"source build/envsetup.sh && lunch {cfg.lunch} && {script}"
    run(["bash", "-c", full], cwd=cfg.workdir)
