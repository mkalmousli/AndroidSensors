"""The rest of the standard Android sensor types (see sensors.py for the field meanings).

Uncalibrated sensors carry x, y, z plus three bias values. Detectors and gestures are triggers:
`mutation { triggerStepDetector { ok } }` fires one event ("oneshot" sensors disable after firing).
"""
from .catalog import Sensor


def _uncal(name, type_, unit, rng, res, init):
    return Sensor(name=name, hal="sensors", type=type_, mode="continuous",
                  fields=["x", "y", "z", "xBias", "yBias", "zBias"], unit=unit, max_range=rng,
                  resolution=res, power=0.5, min_delay_us=10000, init=init + [0, 0, 0])


def _trigger(name, type_, mode="oneshot", wakeup=True, feature=""):
    return Sensor(name=name, hal="sensors", type=type_, mode=mode, fields=["count"], init=[0],
                  max_range=1, resolution=1, power=0.1, wakeup=wakeup, feature=feature)


def _scalar(name, type_, unit, rng, res, init, delay=200000, mode="onchange", feature=""):
    return Sensor(name=name, hal="sensors", type=type_, mode=mode, fields=[unit_field(unit)],
                  unit=unit, max_range=rng, resolution=res, power=0.1, min_delay_us=delay,
                  init=[init], feature=feature)


def unit_field(unit):
    return {"deg": "degrees", "C": "celsius", "": "value"}.get(unit, "value")


def _quat(name, type_):
    return Sensor(name=name, hal="sensors", type=type_, mode="continuous", unit="quat",
                  fields=["x", "y", "z", "w"], max_range=1, resolution=1e-05, power=0.5,
                  min_delay_us=10000, init=[0, 0, 0, 1])


EXTRA = [
    _uncal("accelerometerUncalibrated", 35, "m/s^2", 39.2, 0.0012, [0, 9.81, 0]),
    _uncal("magnetometerUncalibrated", 14, "uT", 2000, 0.0625, [0, 25, -40]),
    _uncal("gyroscopeUncalibrated", 16, "rad/s", 34.9, 0.0011, [0, 0, 0]),
    _quat("gameRotationVector", 15),
    _quat("geomagneticRotationVector", 20),
    _trigger("significantMotion", 17),
    _trigger("stepDetector", 18, mode="special", wakeup=False, feature="stepdetector"),
    _trigger("tiltDetector", 22, mode="special"),
    _trigger("wakeGesture", 23),
    _trigger("glanceGesture", 24),
    _trigger("pickUpGesture", 25),
    _trigger("wristTiltGesture", 26, mode="special"),
    _trigger("stationaryDetect", 29, wakeup=False),
    _trigger("motionDetect", 30, wakeup=False),
    _scalar("deviceOrientation", 27, "", 3, 1, 0),
    _scalar("heartBeat", 31, "", 1, 1, 0, delay=0),
    _scalar("lowLatencyOffbodyDetect", 34, "", 1, 1, 1, delay=0),
    _scalar("hingeAngle", 36, "deg", 360, 1, 180, feature="hinge_angle"),
    _scalar("temperature", 7, "C", 100, 0.1, 30, mode="continuous"),
    Sensor(name="pose6dof", hal="sensors", type=28, mode="continuous", unit="pose",
           fields=["qx", "qy", "qz", "qw", "tx", "ty", "tz", "dqx", "dqy", "dqz", "dqw", "dtx",
                   "dty", "dtz", "seq"], max_range=1, resolution=1e-05, power=0.5,
           min_delay_us=10000, init=[0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0]),
]
