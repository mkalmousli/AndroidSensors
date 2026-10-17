"""Package the built system images as an emulator zip split into GitHub-sized parts."""
import hashlib
from pathlib import Path

from ..config import Config
from ..sh import in_aosp

PART_BYTES = 1_900_000_000  # GitHub release assets must stay under 2 GiB
CHUNK = 1 << 24


def find_zip(cfg: Config) -> Path:
    found = sorted((cfg.workdir / "out" / "target" / "product").glob("*/sdk-repo-*system-images*.zip"))
    if not found:
        raise SystemExit("No emulator image zip found; did `asp build` finish?")
    return found[-1]


def split(src: Path, out_dir: Path, part_bytes: int = PART_BYTES) -> list[Path]:
    """Write src as name.zip.000, .001, ... (rejoin with `cat name.zip.* > name.zip`)."""
    out_dir.mkdir(parents=True, exist_ok=True)
    parts = []
    with src.open("rb") as f:
        while True:
            part = out_dir / f"{src.name}.{len(parts):03d}"
            written = 0
            with part.open("wb") as o:
                while written < part_bytes and (chunk := f.read(min(CHUNK, part_bytes - written))):
                    o.write(chunk)
                    written += len(chunk)
            if written == 0:
                part.unlink()
                break
            parts.append(part)
    return parts


def checksums(files: list[Path], out: Path) -> Path:
    lines = []
    for f in files:
        h = hashlib.sha256()
        with f.open("rb") as fh:
            while chunk := fh.read(CHUNK):
                h.update(chunk)
        lines.append(f"{h.hexdigest()}  {f.name}")
    out.write_text("\n".join(lines) + "\n")
    return out


def package(cfg: Config) -> list[Path]:
    in_aosp(cfg, f"m -j{cfg.jobs} emu_img_zip")
    parts = split(find_zip(cfg), cfg.workdir / "dist")
    return parts + [checksums(parts, cfg.workdir / "dist" / "SHA256SUMS")]
