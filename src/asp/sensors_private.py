"""Vendor-private sensors (type >= 0x10000) for signals Android has no standard sensor for.

Apps that want them list `SensorManager.getSensorList(Sensor.TYPE_ALL)` and match the string type.
Google Play services, when the user adds it, derives activity and floors from the standard sensors.
"""
from .catalog import Sensor

ACTIVITIES = "0 still, 1 walking, 2 running, 3 cycling, 4 vehicle"

PRIVATE = [
    Sensor(name="activity", hal="sensors", type=0x10001, type_string="android.sensor.activity",
           mode="onchange", fields=["code", "confidence"], unit=ACTIVITIES, init=[0, 100],
           max_range=4, resolution=1, power=0.1),
    Sensor(name="floors", hal="sensors", type=0x10002, type_string="android.sensor.floors",
           mode="onchange", fields=["climbed"], unit="floors", init=[0], max_range=1000,
           resolution=1, power=0.1),
]
