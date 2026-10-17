#!/bin/bash
# Runs on the GCE VM at boot (startup-script metadata). Builds Android, publishes a
# GitHub Release, then deletes this VM so billing stops. Progress: serial console.
set -uo pipefail
md() { curl -fs -H 'Metadata-Flavor: Google' "http://metadata.google.internal/computeMetadata/v1/$1"; }
attr() { md "instance/attributes/$1"; }
NAME=$(md instance/name)
ZONE=$(md instance/zone | awk -F/ '{print $NF}')
REPO=$(attr asp-repo); BRANCH=$(attr asp-repo-branch); SECRET=$(attr asp-secret)
LOG=/var/log/asp-build.log
exec > >(tee -a "$LOG" /dev/console) 2>&1
trap 'gcloud compute instances delete "$NAME" --zone "$ZONE" --quiet' EXIT

apt-get update -qq && apt-get install -y -qq docker.io git
TOKEN=$(gcloud secrets versions access latest --secret "$SECRET")
git clone --depth 1 --branch "$BRANCH" "https://x-access-token:${TOKEN}@github.com/${REPO}.git" /opt/src
docker build -t androidsensors /opt/src
mkdir -p /var/lib/aosp /var/lib/ccache

run_asp() {
  docker run --rm -e ASP_GITHUB_TOKEN="$TOKEN" -e ASP_REPO="$REPO" \
    -e ASP_BRANCH="$(attr asp-aosp-branch || true)" \
    -v /var/lib/aosp:/aosp -v /var/lib/ccache:/ccache -v "$LOG":/build.log androidsensors "$@"
}
if ! run_asp asp release; then
  echo "BUILD FAILED - publishing the log as a release"
  run_asp uv run --project /opt/asp python -c "
import os, time
from pathlib import Path
from asp.steps.publish import publish
os.environ['ASP_TAG'] = time.strftime('failed-%Y%m%d-%H%M%S', time.gmtime())
print(publish([Path('/build.log')], 'Build failed; see attached log.'))"
fi
