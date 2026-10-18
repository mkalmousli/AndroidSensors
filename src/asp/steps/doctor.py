"""List stock packages in the AOSP tree that our HALs replace (the ones our modules `overrides:`)."""
import re

from ..config import Config
from ..sh import capture

PATTERN = re.compile(r'name:\s*"([^"]*(gnss|health|sensors)[^"]*(ranchu|goldfish|multihal)[^"]*)"')


def doctor(cfg: Config) -> None:
    out = capture(["grep", "-rE", "--include=Android.bp", "name:", "device/generic/goldfish"],
                  cwd=cfg.workdir)
    names = sorted({m.group(1) for line in out.splitlines() if (m := PATTERN.search(line))})
    print("Candidate stock modules:\n  " + "\n  ".join(names))
