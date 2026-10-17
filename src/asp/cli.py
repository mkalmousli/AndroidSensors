"""`asp` command line: sync -> patch -> build -> run -> verify."""
import argparse
import sys
from pathlib import Path

from .config import Config
from .render import render_all
from .steps.build import build
from .steps.doctor import doctor
from .steps.package import package
from .steps.patch import patch
from .steps.publish import publish
from .steps.run import start
from .steps.sync import sync
from .steps.verify import verify

STEPS = {"sync": sync, "patch": patch, "doctor": doctor, "run": start}


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(prog="asp", description=__doc__)
    sub = p.add_subparsers(dest="cmd", required=True)
    for name in STEPS:
        sub.add_parser(name)
    b = sub.add_parser("build")
    b.add_argument("--quick", action="store_true", help="rebuild only our modules")
    sub.add_parser("verify")
    sub.add_parser("all", help="sync, patch and build")
    sub.add_parser("release", help="sync, patch, build, package and publish a GitHub Release")
    r = sub.add_parser("render", help="render generated files only (no AOSP needed)")
    r.add_argument("--out", type=Path, default=Path("build/gen"))
    args = p.parse_args(argv)
    cfg = Config()

    if args.cmd == "render":
        render_all(args.out)
    elif args.cmd == "build":
        build(cfg, args.quick)
    elif args.cmd == "verify":
        return 0 if verify(cfg) else 1
    elif args.cmd == "release":
        for step in (sync, patch, build):
            step(cfg)
        print("Release:", publish(package(cfg)))
    elif args.cmd == "all":
        for step in (sync, patch):
            step(cfg)
        build(cfg)
    else:
        STEPS[args.cmd](cfg)
    return 0


if __name__ == "__main__":
    sys.exit(main())
