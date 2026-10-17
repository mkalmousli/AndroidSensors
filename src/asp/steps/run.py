"""Boot the patched image in the AOSP-built emulator and publish its ports."""
import shutil
import threading

from ..config import Config
from ..sh import in_aosp, run

FWD_BASE = 18000


def _publish(port: int, local: int) -> None:
    """Re-expose a loopback-only port on all interfaces so Docker can publish it."""
    if shutil.which("socat"):
        listen = f"TCP-LISTEN:{port},fork,reuseaddr,bind=0.0.0.0"
        run(f"socat {listen} TCP:127.0.0.1:{local} &", shell=True)


def _expose(cfg: Config) -> None:
    run(["adb", "wait-for-device"])
    run(["adb", "forward", f"tcp:{FWD_BASE}", f"tcp:{cfg.http_port}"])
    _publish(cfg.http_port, FWD_BASE)


def start(cfg: Config) -> None:
    threading.Thread(target=_expose, args=(cfg,), daemon=True).start()
    flags = "-no-window -no-audio -no-snapshot -gpu swiftshader_indirect"
    in_aosp(cfg, f"emulator {flags} -ports 5554,{cfg.adb_port}")
