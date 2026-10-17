"""Fail if any tracked source file exceeds the line limit."""
import sys
from pathlib import Path

LIMIT = 100
SUFFIXES = {".py", ".cpp", ".h", ".bp", ".mk", ".rc", ".j2", ".yaml", ".yml", ".xml", ".sh", ".conf"}
SKIP = {".venv", ".git", "build", "__pycache__", ".ruff_cache", ".pytest_cache", "uv.lock"}


def main(root: Path) -> int:
    bad = []
    for path in sorted(root.rglob("*")):
        if path.is_file() and path.suffix in SUFFIXES and not SKIP & set(path.parts):
            n = len(path.read_text().splitlines())
            if n > LIMIT:
                bad.append(f"{path.relative_to(root)}: {n} lines")
    print("\n".join(bad) or f"ok: all files <= {LIMIT} lines")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(Path(sys.argv[1] if len(sys.argv) > 1 else ".")))
