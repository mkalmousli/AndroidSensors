"""The sensor catalog: edit this list to add or change a sensor.

hal:    "sensors" | "gnss" | "health"   which system HAL serves it
type:   Android SENSOR_TYPE_* id (sensors HAL only)
mode:   "continuous" | "onchange" | "special"
fields: GraphQL field names, one per value (max 6); init gives their start values
feature: android.hardware.sensor.<feature> declared in the image, if any
"""
from .catalog import Sensor

SENSORS = [
    Sensor(
        name="accelerometer", hal="sensors", type=1, mode="continuous", fields=["x", "y", "z"],
        unit="m/s^2", max_range=39.2, resolution=0.0012, power=0.5, min_delay_us=10000,
        init=[0, 9.81, 0], feature="accelerometer"
    ),
    Sensor(
        name="magnetometer", hal="sensors", type=2, mode="continuous", fields=["x", "y", "z"],
        unit="uT", max_range=2000, resolution=0.0625, power=0.5, min_delay_us=10000,
        init=[0, 25, -40], feature="compass"
    ),
    Sensor(
        name="orientation", hal="sensors", type=3, mode="continuous",
        fields=["azimuth", "pitch", "roll"], unit="deg", max_range=360, resolution=1e-05,
        power=0.5, min_delay_us=10000, init=[0, 0, 0]
    ),
    Sensor(
        name="gyroscope", hal="sensors", type=4, mode="continuous", fields=["x", "y", "z"],
        unit="rad/s", max_range=34.9, resolution=0.0012, power=0.5, min_delay_us=10000,
        init=[0, 0, 0], feature="gyroscope"
    ),
    Sensor(
        name="light", hal="sensors", type=5, mode="onchange", fields=["lux"], unit="lux",
        max_range=40000, resolution=1, power=0.1, min_delay_us=200000, init=[400], feature="light"
    ),
    Sensor(
        name="pressure", hal="sensors", type=6, mode="continuous", fields=["hpa"], unit="hPa",
        max_range=1100, resolution=0.01, power=0.1, min_delay_us=100000, init=[1013.25],
        feature="barometer"
    ),
    Sensor(
        name="proximity", hal="sensors", type=8, mode="onchange", fields=["cm"], unit="cm",
        max_range=5, resolution=5, power=0.1, min_delay_us=0, init=[5], feature="proximity"
    ),
    Sensor(
        name="gravity", hal="sensors", type=9, mode="continuous", fields=["x", "y", "z"],
        unit="m/s^2", max_range=19.6, resolution=0.0012, power=0.5, min_delay_us=10000,
        init=[0, 9.81, 0]
    ),
    Sensor(
        name="linearAcceleration", hal="sensors", type=10, mode="continuous",
        fields=["x", "y", "z"], unit="m/s^2", max_range=19.6, resolution=0.0012, power=0.5,
        min_delay_us=10000, init=[0, 0, 0]
    ),
    Sensor(
        name="rotationVector", hal="sensors", type=11, mode="continuous",
        fields=["x", "y", "z", "w"], unit="quat", max_range=1, resolution=1e-05, power=0.5,
        min_delay_us=10000, init=[0, 0, 0, 1]
    ),
    Sensor(
        name="humidity", hal="sensors", type=12, mode="onchange", fields=["percent"], unit="%",
        max_range=100, resolution=1, power=0.1, min_delay_us=200000, init=[45],
        feature="relative_humidity"
    ),
    Sensor(
        name="ambientTemperature", hal="sensors", type=13, mode="onchange", fields=["celsius"],
        unit="C", max_range=80, resolution=0.1, power=0.1, min_delay_us=200000, init=[22],
        feature="ambient_temperature"
    ),
    Sensor(
        name="heartRate", hal="sensors", type=21, mode="onchange", fields=["bpm"], unit="bpm",
        max_range=300, resolution=1, power=0.5, min_delay_us=200000, init=[70],
        feature="heartrate"
    ),
    Sensor(
        name="stepCounter", hal="sensors", type=19, mode="onchange", fields=["steps"],
        unit="steps", max_range=1000000, resolution=1, power=0.1, min_delay_us=0, init=[0],
        feature="stepcounter"
    ),
    Sensor(
        name="location", hal="gnss", fields=["lat", "lon", "alt", "speed", "bearing", "accuracy"],
        unit="deg/m", init=[0, 0, 0, 0, 0, 5]
    ),
    Sensor(
        name="battery", hal="health", fields=["level", "charging", "temperature", "voltage", "ac"],
        unit="%/C/mV", init=[100, 0, 25, 4200, 0]
    ),
]
