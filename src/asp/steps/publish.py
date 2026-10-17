"""Publish files as a GitHub Release (stdlib only; token from ASP_GITHUB_TOKEN)."""
import json
import os
import time
import urllib.request
from pathlib import Path

API = "https://api.github.com"
UPLOADS = "https://uploads.github.com"


def _call(req: urllib.request.Request, retries: int = 3) -> dict:
    for attempt in range(retries):
        try:
            with urllib.request.urlopen(req, timeout=600) as resp:
                return json.load(resp)
        except OSError:
            if attempt == retries - 1:
                raise
            time.sleep(5 * (attempt + 1))
    raise AssertionError("unreachable")


def _headers(token: str, **extra: str) -> dict:
    return {"Authorization": f"Bearer {token}", "Accept": "application/vnd.github+json", **extra}


def create_release(repo: str, tag: str, notes: str, token: str) -> dict:
    body = json.dumps({"tag_name": tag, "name": tag, "body": notes, "prerelease": True}).encode()
    req = urllib.request.Request(
        f"{API}/repos/{repo}/releases", data=body, headers=_headers(token), method="POST"
    )
    return _call(req)


def upload(repo: str, release_id: int, path: Path, token: str) -> None:
    url = f"{UPLOADS}/repos/{repo}/releases/{release_id}/assets?name={path.name}"
    with path.open("rb") as f:
        req = urllib.request.Request(
            url,
            data=f,
            method="POST",
            headers=_headers(
                token, **{"Content-Type": "application/octet-stream",
                          "Content-Length": str(path.stat().st_size)}
            ),
        )
        _call(req, retries=1)


def publish(files: list[Path], notes: str = "") -> str:
    token = os.environ["ASP_GITHUB_TOKEN"]
    repo = os.environ.get("ASP_REPO", "mkalmousli/AndroidSensors")
    tag = os.environ.get("ASP_TAG") or time.strftime("build-%Y%m%d-%H%M%S", time.gmtime())
    release = create_release(repo, tag, notes, token)
    for f in files:
        print(f"uploading {f.name} ({f.stat().st_size >> 20} MiB)", flush=True)
        for attempt in range(3):
            try:
                upload(repo, release["id"], f, token)
                break
            except OSError:
                if attempt == 2:
                    raise
                time.sleep(10)
    return release["html_url"]
