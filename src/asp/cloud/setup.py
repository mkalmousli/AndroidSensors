"""One-time Google Cloud project preparation (idempotent)."""
from .tools import gcloud, gh, run

SECRET = "asp-github-token"
ROLES = ("roles/secretmanager.secretAccessor", "roles/compute.instanceAdmin.v1")


def project() -> str:
    name = run("gcloud", "config", "get-value", "project", quiet=True).strip()
    if not name or name == "(unset)":
        raise SystemExit("No project set. Run: gcloud config set project <PROJECT_ID>")
    return name


def github_repo() -> str:
    return gh("repo", "view", "--json", "nameWithOwner", "-q", ".nameWithOwner", quiet=True).strip()


def github_token() -> str:
    return gh("auth", "token", quiet=True).strip()


def enable_apis(proj: str) -> None:
    gcloud("services", "enable", "compute.googleapis.com", "secretmanager.googleapis.com",
           "--project", proj)


def store_token(proj: str, token: str) -> None:
    try:
        gcloud("secrets", "describe", SECRET, "--project", proj)
    except SystemExit:
        gcloud("secrets", "create", SECRET, "--replication-policy=automatic", "--project", proj)
    gcloud("secrets", "versions", "add", SECRET, "--data-file=-", "--project", proj, stdin=token)


def compute_account(proj: str) -> str:
    number = gcloud("projects", "describe", proj, "--format=value(projectNumber)").strip()
    return f"{number}-compute@developer.gserviceaccount.com"


def grant_roles(proj: str, account: str) -> None:
    for role in ROLES:
        gcloud("projects", "add-iam-policy-binding", proj, "--member", f"serviceAccount:{account}",
               "--role", role, "--condition=None")
