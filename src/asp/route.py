"""`asp route file.gpx`: replay a GPX track through the device's GraphQL API."""
import json
import urllib.request
import xml.etree.ElementTree as ET


def read_gpx(path: str) -> list[list[float]]:
    pts = []
    for el in ET.parse(path).iter():
        if el.tag.endswith("trkpt") or el.tag.endswith("rtept"):
            ele = next((c.text for c in el if c.tag.endswith("ele")), None)
            pts.append([float(el.get("lat")), float(el.get("lon")), float(ele or 0)])
    return pts


def mutation(points: list[list[float]], speed: float, loop: bool) -> str:
    args = f"points: {json.dumps(points)}, speed: {speed}, loop: {str(loop).lower()}"
    return "mutation { playRoute(" + args + ") { ok } }"


def play(path: str, speed: float, loop: bool, url: str = "http://localhost:8000/graphql") -> str:
    body = json.dumps({"query": mutation(read_gpx(path), speed, loop)}).encode()
    return urllib.request.urlopen(urllib.request.Request(url, body)).read().decode()
