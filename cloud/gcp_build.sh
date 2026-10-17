#!/bin/bash
# One command from Cloud Shell: creates a big VM that builds Android and publishes a
# GitHub Release, then deletes itself.   GITHUB_TOKEN=... bash cloud/gcp_build.sh
set -euo pipefail
: "${GITHUB_TOKEN:?Set GITHUB_TOKEN (fine-grained PAT, Contents: read+write on the repo)}"
HERE=$(cd "$(dirname "$0")" && pwd)
PROJECT=${PROJECT:-$(gcloud config get-value project)}
ZONE=${ZONE:-us-central1-a}
MACHINE=${MACHINE:-n2-highmem-8}       # 8 vCPU / 64 GB; use n2-standard-32 if quota allows
DISK_GB=${DISK_GB:-500}
REPO=${REPO:-mkalmousli/AndroidSensors}
REPO_BRANCH=${REPO_BRANCH:-ccr-0a28e22a-43vywr}
SECRET=asp-github-token
NAME=asp-build-$(date +%Y%m%d-%H%M%S)

gcloud services enable compute.googleapis.com secretmanager.googleapis.com --project "$PROJECT"
if ! gcloud secrets describe "$SECRET" --project "$PROJECT" >/dev/null 2>&1; then
  gcloud secrets create "$SECRET" --replication-policy=automatic --project "$PROJECT"
fi
printf %s "$GITHUB_TOKEN" | gcloud secrets versions add "$SECRET" --data-file=- --project "$PROJECT"

NUM=$(gcloud projects describe "$PROJECT" --format='value(projectNumber)')
SA="${NUM}-compute@developer.gserviceaccount.com"
for role in roles/secretmanager.secretAccessor roles/compute.instanceAdmin.v1; do
  gcloud projects add-iam-policy-binding "$PROJECT" --member "serviceAccount:$SA" \
    --role "$role" --condition=None >/dev/null
done

gcloud compute instances create "$NAME" --project "$PROJECT" --zone "$ZONE" \
  --machine-type "$MACHINE" --boot-disk-size "${DISK_GB}GB" --boot-disk-type pd-balanced \
  --image-family ubuntu-2204-lts --image-project ubuntu-os-cloud \
  --service-account "$SA" --scopes cloud-platform \
  --max-run-duration "${MAX_HOURS:-14}h" --instance-termination-action DELETE \
  --metadata-from-file startup-script="$HERE/startup.sh" \
  --metadata "asp-repo=$REPO,asp-repo-branch=$REPO_BRANCH,asp-secret=$SECRET,asp-aosp-branch=${ASP_BRANCH:-android-14.0.0_r1}"

echo "Started $NAME. Watch:  gcloud compute instances tail-serial-port-output $NAME --zone $ZONE"
echo "Result appears at https://github.com/$REPO/releases when it finishes (VM deletes itself)."
