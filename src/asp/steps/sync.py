"""Fetch the pinned AOSP branch with repo."""
from ..config import Config
from ..sh import run


def sync(cfg: Config) -> None:
    cfg.workdir.mkdir(parents=True, exist_ok=True)
    if not (cfg.workdir / ".repo").exists():
        run(
            ["repo", "init", "-u", cfg.manifest_url, "-b", cfg.branch, "--depth=1"],
            cwd=cfg.workdir,
        )
    run(
        ["repo", "sync", "-c", "-j", str(cfg.sync_jobs), "--no-tags", "--no-clone-bundle"],
        cwd=cfg.workdir,
    )
