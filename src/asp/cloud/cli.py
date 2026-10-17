"""Argument parsing for `asp cloud`."""
from . import commands

ACTIONS = {"status": commands.status, "logs": commands.logs, "cleanup": commands.cleanup}


def register(sub) -> None:
    cloud = sub.add_parser("cloud", help="build in Google Cloud and publish to GitHub")
    actions = cloud.add_subparsers(dest="action", required=True)
    b = actions.add_parser("build", help="create the VM (uses your gcloud and gh logins)")
    b.add_argument("--repo", help="owner/name (default: from the current gh repo)")
    b.add_argument("--ref", default="main", help="git branch the VM builds")
    b.add_argument("--aosp-branch", help="AOSP tag/branch (default: ASP_BRANCH)")
    b.add_argument("--zone", default="us-central1-a")
    b.add_argument("--machine", default="n2-highmem-8", help="e.g. n2-standard-32 if quota allows")
    b.add_argument("--disk-gb", type=int, default=500)
    b.add_argument("--disk-type", default="pd-standard",
                   help="pd-standard (HDD quota) or pd-balanced/pd-ssd (SSD quota, 250GB default)")
    b.add_argument("--max-hours", type=int, default=14, help="VM is deleted after this long")
    f = actions.add_parser("fetch", help="download and rejoin the newest built image")
    f.add_argument("--out", default="build/images")
    for name, help_ in (("status", "show VMs and releases"), ("logs", "follow the VM console"),
                        ("cleanup", "delete any leftover build VMs")):
        actions.add_parser(name, help=help_)


def run(args) -> None:
    {"build": commands.build, "fetch": commands.fetch, **ACTIONS}[args.action](args)
