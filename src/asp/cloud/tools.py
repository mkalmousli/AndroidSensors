"""Thin wrappers around the gcloud and gh command line tools."""
import shutil
import subprocess

HINTS = {
    "gcloud": "install the Google Cloud CLI, then `gcloud auth login`",
    "gh": "install GitHub CLI, then `gh auth login`",
}


def run(tool: str, *args: str, stdin: str | None = None, quiet: bool = False) -> str:
    if not shutil.which(tool):
        raise SystemExit(f"`{tool}` not found: {HINTS.get(tool, 'install it')}")
    if not quiet:
        print("+", tool, *args, flush=True)
    proc = subprocess.run([tool, *args], input=stdin, text=True, capture_output=True, check=False)
    if proc.returncode:
        raise SystemExit(f"{tool} failed:\n{proc.stderr.strip() or proc.stdout.strip()}")
    return proc.stdout


def gcloud(*args: str, stdin: str | None = None) -> str:
    return run("gcloud", *args, stdin=stdin)


def gh(*args: str, quiet: bool = False) -> str:
    return run("gh", *args, quiet=quiet)
