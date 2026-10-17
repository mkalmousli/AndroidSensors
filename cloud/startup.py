#!/usr/bin/env python3
"""Runs on the build VM at boot (GCE startup-script, stdlib only).

Builds Android with `asp release`, publishes a GitHub Release (or the log on
failure) and always deletes this VM at the end so billing stops.
"""
import os
import subprocess
import sys
import time
import urllib.request
from pathlib import Path

MD = "http://metadata.google.internal/computeMetadata/v1/"
LOG = Path("/var/log/asp-build.log")
SRC = "/opt/src"


def md(path: str) -> str:
    req = urllib.request.Request(MD + path, headers={"Metadata-Flavor": "Google"})
    return urllib.request.urlopen(req).read().decode()


def say(line: str) -> None:
    for sink in (LOG, Path("/dev/console")):
        with sink.open("a") as f:
            f.write(line + "\n")


def run(cmd: list[str], env: dict | None = None, secret: str = "") -> None:
    say("+ " + " ".join(cmd).replace(secret, "***") if secret else "+ " + " ".join(cmd))
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                            env={**os.environ, **(env or {})})
    for line in proc.stdout:
        say(line.rstrip())
    if proc.wait():
        raise RuntimeError(f"command failed ({proc.returncode}): {cmd[0]}")


def build(attrs: dict, token: str) -> None:
    run(["apt-get", "update", "-qq"])
    run(["apt-get", "install", "-y", "-qq", "docker.io", "git"])
    url = f"https://x-access-token:{token}@github.com/{attrs['repo']}.git"
    run(["git", "clone", "--depth", "1", "--branch", attrs["ref"], url, SRC], secret=token)
    run(["docker", "build", "-t", "androidsensors", SRC])
    for d in ("/var/lib/aosp", "/var/lib/ccache"):
        os.makedirs(d, exist_ok=True)
    run(["docker", "run", "--rm", "-e", "ASP_GITHUB_TOKEN", "-e", f"ASP_REPO={attrs['repo']}",
         "-e", f"ASP_BRANCH={attrs['aosp']}", "-v", "/var/lib/aosp:/aosp",
         "-v", "/var/lib/ccache:/ccache", "androidsensors", "asp", "release"],
        env={"ASP_GITHUB_TOKEN": token})


def publish_log(attrs: dict, token: str) -> None:
    sys.path.insert(0, f"{SRC}/src")
    os.environ.update(ASP_GITHUB_TOKEN=token, ASP_REPO=attrs["repo"],
                      ASP_TAG=time.strftime("failed-%Y%m%d-%H%M%S", time.gmtime()))
    from asp.steps.publish import publish
    say("Log published: " + publish([LOG], "Build failed; see attached log."))


def main() -> None:
    name = md("instance/name")
    zone = md("instance/zone").rsplit("/", 1)[-1]
    attrs = {k: md(f"instance/attributes/asp-{k2}") for k, k2 in
             (("repo", "repo"), ("ref", "repo-ref"), ("secret", "secret"), ("aosp", "aosp-branch"))}
    token = ""
    try:
        token = subprocess.check_output(["gcloud", "secrets", "versions", "access", "latest",
                                         "--secret", attrs["secret"]], text=True).strip()
        build(attrs, token)
    except Exception as exc:  # noqa: BLE001 - last-resort handler, we must always clean up
        say(f"BUILD FAILED: {exc}")
        try:
            publish_log(attrs, token)
        except Exception as inner:  # noqa: BLE001
            say(f"could not publish log: {inner}")
    finally:
        run(["gcloud", "compute", "instances", "delete", name, "--zone", zone, "--quiet"])


if __name__ == "__main__":
    main()
