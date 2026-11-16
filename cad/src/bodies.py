"""NW3D-01 - dalsi tvary tela: valec, soudek, zvon, pasy.
Stejne rozhrani jako klasicke telo: nahore krk (vlozky M3 na +-21 mm, otvor pro kabel),
dole spodni viko na 4 vlozky M3, vzadu dole vystup USB kabelu, vpredu lozko pro stitek.
Tisknou se vzhuru nohama (horni plocha na podlozce) bez podper."""
import math
import numpy as np
import cadquery as cq
from shapely.geometry import Point, LineString, Polygon, box as sbox
from shapely import affinity
import robot as R
import variants as V
from robot import BH, WALL, INS_M3, INS_M3_D, CLR_M3, HEAD_M3, NECK_ID, NFL_HOLE_X, PLATE_T, box, cyl_z, cyl_y, corner_fill

# rezy (z, poloosa x, poloosa y) - telo je jejich hladky prechod
SHAPES = {
    "valec":  dict(sections=[(0, 46, 40), (BH, 46, 40)], facet=-32.0, badge_z=23.0),
    "soudek": dict(sections=[(0, 44, 36), (10, 50, 40), (30, 49, 39), (BH, 40, 34)], facet=-27.0, badge_z=23.0),
    "zvon":   dict(sections=[(0, 54, 42), (5, 51, 40), (20, 42, 34), (BH, 38, 32)], facet=-25.0, badge_z=19.0),
    "pasy":   dict(rect=(78, 60, 8.0), facet=None, badge_z=23.0),
}
STYLES = ["valec", "soudek", "zvon", "pasy"]
TRACK_W = 13.0


def spec(style, k=1.0):
    """Rozmery tela dane velikosti (k = meritko vnejsiho tvaru; krk, vlozky a stitek zustavaji)."""
    d = SHAPES[style]
    o = {"bh": BH * k, "badge_z": max(d["badge_z"] * k, 19.0), "facet": None}
    if "rect" in d:
        w, dd, r = d["rect"]
        o["rect"] = (w * k, dd * k, r)
    else:
        o["sections"] = [(z * k, rx * k, ry * k) for z, rx, ry in d["sections"]]
        o["facet"] = FACET_FIX.get((style, round(k, 2)), d["facet"] * k)
    return o


FACET_FIX = {("valec", 0.9): -27.0}   # (styl, k) -> y celni plochy, aby se vesel stitek


def _ellipse_loft(sections, inset=0.0, ztop=None):
    secs = list(sections)
    if ztop is not None:
        secs[-1] = (ztop, secs[-1][1], secs[-1][2])
    z0 = secs[0][0]
    w = cq.Workplane("XY").workplane(offset=z0).ellipse(secs[0][1] - inset, secs[0][2] - inset)
    for z, rx, ry in secs[1:]:
        w = w.workplane(offset=z - z0).ellipse(rx - inset, ry - inset)
        z0 = z
    return w.loft(ruled=len(secs) == 2)


def front_y(style, k=1.0):
    """y celni plochy (kde lezi stitek)."""
    d = spec(style, k)
    if d.get("facet") is not None:
        return d["facet"]
    return -d["rect"][1] / 2


def outline_at_bottom(style, k=1.0):
    """Obrys spodku tela (pro spodni viko a vlozky) jako shapely polygon."""
    d = spec(style, k)
    if "rect" in d:
        w, dd, r = d["rect"]
        return Polygon(V.rounded(w, dd, r, r))
    z, rx, ry = d["sections"][0]
    e = affinity.scale(Point(0, 0).buffer(1, resolution=64), rx, ry)
    if d["facet"] is not None:
        e = e.intersection(sbox(-200, d["facet"], 200, 200))
    return e


def bosses(style, k=1.0):
    """4 vlozky pro spodni viko (x, y) - na paprscich od stredu, kousek od steny."""
    inner = outline_at_bottom(style, k).buffer(-WALL - 4.6)
    out = []
    for ang in (35, 145, 215, 325):
        a = math.radians(ang)
        ray = LineString([(0, 0), (200 * math.cos(a), 200 * math.sin(a))])
        hit = ray.intersection(inner.exterior)
        p = hit if hit.geom_type == "Point" else min(hit.geoms, key=lambda g: g.distance(Point(0, 0)))
        out.append((p.x, p.y))
    return out


def feet(style, k=1.0):
    """4 nozicky co nejdal od stredu (hlavne dopredu/dozadu), aspon 10.5 mm od sroubu spodniho vika."""
    ring = outline_at_bottom(style, k).buffer(-7.0)
    bs = bosses(style, k)
    best = None
    for a10 in range(300, 890, 5):
        a = math.radians(a10 / 10)
        hit = LineString([(0, 0), (200 * math.cos(a), 200 * math.sin(a))]).intersection(ring.exterior)
        p = hit if hit.geom_type == "Point" else min(hit.geoms, key=lambda g: g.distance(Point(0, 0)))
        if min(math.hypot(p.x - x, p.y - y) for x, y in bs) < 10.5:
            continue
        if best is None or min(p.x, p.y) > min(best):
            best = (p.x, p.y)
    x, y = best
    return [(sx * x, sy * y) for sx in (-1, 1) for sy in (-1, 1)]


def _outer(style, k=1.0):
    d = spec(style, k)
    BH = d["bh"]
    if "rect" in d:
        w, dd, r = d["rect"]
        return cq.Workplane("XY").polyline(V.rounded(w, dd, r, r)).close().workplane(offset=BH) \
            .polyline(V.rounded(w, dd, r, r)).close().loft()
    o = _ellipse_loft(d["sections"])
    if d["facet"] is not None:
        o = o.cut(box(-200, 200, -200, d["facet"], -10, BH + 10))
    return o


def _inner(style, k=1.0):
    d = spec(style, k)
    BH = d["bh"]
    if "rect" in d:
        w, dd, r = d["rect"]
        i = cq.Workplane("XY").polyline(V.rounded(w - 2 * WALL, dd - 2 * WALL, r - WALL, r - WALL)).close() \
            .workplane(offset=BH - WALL).polyline(V.rounded(w - 2 * WALL, dd - 2 * WALL, r - WALL, r - WALL)).close().loft()
    else:
        i = _ellipse_loft(d["sections"], WALL, ztop=BH - WALL)
        if d["facet"] is not None:
            i = i.cut(box(-200, 200, -200, d["facet"] + WALL, -10, BH + 10))
    return i.translate((0, 0, -0.01))


def badge_mount(style, k=1.0):
    """Matice 4x4: stitek z klasickeho tela (sikma stena) -> na celni plochu tohoto tela."""
    def rx(deg):
        a = math.radians(deg); c, s = math.cos(a), math.sin(a)
        return np.array([[1, 0, 0, 0], [0, c, -s, 0], [0, s, c, 0], [0, 0, 0, 1]])
    def tr(x, y, z):
        m = np.eye(4); m[:3, 3] = (x, y, z); return m
    klasik = tr(0, V.Y_FACE, V.BADGE_ZC) @ rx(-math.degrees(V.SLOPE))
    if style == "hranate":
        new = tr(0, V.Y_FACE * k, V.BADGE_ZC * k) @ rx(-math.degrees(V.SLOPE))
    else:
        new = tr(0, front_y(style, k), spec(style, k)["badge_z"])
    return new @ np.linalg.inv(klasik)


def any_body(style, k=1.0):
    return V.body(style, k) if style == "hranate" else body(style, k)


def any_bottom(style, k=1.0):
    return V.bottom_plate(style, k) if style == "hranate" else bottom_plate(style, k)


def body_height(style, k=1.0):
    return BH * k


def support(style, k=1.0):
    """Body, na kterych robot stoji (nozicky / pasy) - pro vypocet stability."""
    if style == "hranate":
        bx, by = R.BOSS_XY[0] * k, R.BOSS_XY[1] * k
        return [(sx * (bx - 12), sy * (by - 2)) for sx in (-1, 1) for sy in (-1, 1)]
    if style == "pasy":
        x = spec(style, k)["rect"][0] / 2 - 2.0 + TRACK_W
        y = 33 * k
        return [(-x, -y), (x, -y), (x, y), (-x, y)]
    return feet(style, k)


def body(style, k=1.0):
    d = spec(style, k)
    BH = d["bh"]
    outer = _outer(style, k)
    b = outer.cut(_inner(style, k))
    try:
        b = b.faces(">Z").edges().chamfer(1.5)
    except Exception:
        pass
    for x, y in bosses(style, k):
        sx = 1 if x > 0 else -1
        sy = 1 if y > 0 else -1
        b = b.union(corner_fill(outer, x - sx * 5, y - sy * 5, sx, sy, "z", 0, 12, 12, steps=12))
    for x, y in bosses(style, k):
        b = b.cut(cyl_z(INS_M3, -1, INS_M3_D, x, y))
    # krk
    for sx in (-1, 1):
        b = b.union(cyl_z(9.0, BH - WALL - 8, BH - 0.5, sx * NFL_HOLE_X, 0))
        b = b.cut(cyl_z(INS_M3, BH - INS_M3_D, BH + 1, sx * NFL_HOLE_X, 0))
    b = b.cut(cyl_z(NECK_ID, BH - WALL - 1, BH + 1, 0, 0))
    # vystup kabelu vzadu dole
    yb = outline_at_bottom(style, k).bounds[3]
    b = b.cut(box(-6, 6, yb - 10, yb + 2, -1, 8.0))
    b = b.cut(cyl_y(12, yb - 10, yb + 2, 0, 8.0))
    # vetraci drazky na bocich (pasy maji boky zakryte)
    if style != "pasy":
        for sx in (-1, 1):
            for i in range(3):
                z = (22 + i * 6) * k
                b = b.cut(box(min(sx * 30, sx * 70), max(sx * 30, sx * 70), -10, 10, z - 1.5, z + 1.5))
    # lozko pro stitek (svisla celni plocha)
    yf, zc = front_y(style, k), d["badge_z"]
    pocket = V.solid_xz(V.badge_outline(V.BADGE_CLR), -1.0, V.BADGE_POCKET).translate((0, yf, zc))
    b = b.cut(pocket)
    if style == "pasy":
        b = b.union(tracks(k))
    return b


def tracks(k=1.0):
    """Pasy po stranach: stadion pres celou vysku (tiskne se od podlozky), kola na vnejsi strane."""
    d = spec("pasy", k)
    BH = d["bh"]
    w = d["rect"][0] / 2
    out = None
    for sx in (-1, 1):
        x0 = sx * (w - 2.0)
        t = (cq.Workplane("YZ").center(0, (BH - 3) / 2).slot2D(66 * k, BH + 3, 0).extrude(TRACK_W * sx)
             .translate((x0, 0, 0)))
        # drazky behounu na vnejsi strane (svisle - tisknou se bez podper)
        for i in range(-int(6 * k), int(6 * k) + 1):
            g = box(-0.8, 0.8, i * 5 - 0.9, i * 5 + 0.9, -4, BH + 1).translate((x0 + sx * TRACK_W, 0, 0))
            t = t.cut(g)
        for yy in (-18 * k, 0, 18 * k):
            hub = cq.Workplane("YZ").center(yy, (BH - 3) / 2).circle(8.5).circle(3).extrude(1.6 * sx) \
                .translate((x0 + sx * TRACK_W, 0, 0))
            t = t.union(hub)
        out = t if out is None else out.union(t)
    return out


def bottom_plate(style, k=1.0):
    t = PLATE_T
    poly = outline_at_bottom(style, k)
    pts = list(poly.simplify(0.05).exterior.coords)[:-1]
    p = cq.Workplane("XY").polyline(pts).close().extrude(t).translate((0, 0, -t))
    try:
        p = p.faces("<Z").edges().chamfer(1.0)
    except Exception:
        pass
    for x, y in bosses(style, k):
        p = p.cut(cyl_z(CLR_M3, -t - 1, 1, x, y))
        p = p.cut(cyl_z(HEAD_M3, -t - 1, -t + 1.8, x, y))
    if style != "pasy":     # pasy stoji na pasech
        for x, y in feet(style, k):
            p = p.cut(cyl_z(11.6, -t - 1, -t + 1.0, x, y))
    return p
