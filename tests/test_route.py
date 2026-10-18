from asp.route import mutation, read_gpx

GPX = """<gpx xmlns="http://www.topografix.com/GPX/1/1"><trk><trkseg>
<trkpt lat="52.0" lon="13.0"><ele>34.5</ele></trkpt><trkpt lat="52.1" lon="13.1"/></trkseg></trk></gpx>"""


def test_gpx_points_and_mutation(tmp_path):
    f = tmp_path / "a.gpx"
    f.write_text(GPX)
    pts = read_gpx(str(f))
    assert pts == [[52.0, 13.0, 34.5], [52.1, 13.1, 0.0]]
    q = mutation(pts, 2.5, True)
    assert "playRoute(points: [[52.0, 13.0, 34.5]" in q and "loop: true" in q
