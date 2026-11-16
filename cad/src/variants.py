"""NW3D-01 - varianty dilu pro konfigurator.
Vsechny dily jsou v souradnicich sestavy (stejne jako robot.build()).
Pouziva rozmery a pomocne funkce z robot.py."""
import math
import cadquery as cq
from shapely.geometry import LineString, Point, Polygon, box as sbox
from shapely import affinity
from shapely.ops import unary_union
import robot as R
from robot import (HW, HH, HD, WALL, FRONT, EYE_DX, EYE_Z, DISP_FACE, BRASS_TOP, PAD_H, CLAMP,
                   DISP_D, TAB_OUT, TAB_W, EYE_OPEN, INS_M3, INS_M3_D, CLR_M3, HEAD_M3,
                   NECK_ID, NECK_HOLE_R, NFL_HOLE_X, NFL_T, NECK_H,
                   BW0, BD0, BW1, BD1, BH, BCH, PLATE_T, BOSS_XY,
                   RING_OD, RING_ID, RING_T, RING_PIN_R, box, cyl_y, cyl_z, corner_fill, de_ears)

Z_HEAD = BH + NFL_T + NECK_H
Y_HEAD = -HD / 2

# --------------------------------------------------------------------------------------
#  Obrysy (tvary hlavy a tela)
# --------------------------------------------------------------------------------------
BODY_STYLES = {
    "hranate": ("chamfer", BCH, BCH),
    "oble":    ("round", 8.0, 6.0),      # (zaobleni dole, nahore)
    "kapsle":  ("round", 16.0, 12.0),
}


def rounded(w, d, rt, rb, n=10):
    """Obdelnik w x d se zaoblenymi rohy: rt nahore (+y), rb dole (-y)."""
    x, y = w / 2, d / 2
    pts = []
    for cx, cy, r, a0 in ((x - rb, -y + rb, rb, -90), (x - rt, y - rt, rt, 0),
                          (-x + rt, y - rt, rt, 90), (-x + rb, -y + rb, rb, 180)):
        for i in range(n + 1):
            a = math.radians(a0 + 90 * i / n)
            pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return pts


# Profil hlavy pri pohledu zepredu (x, z), z = 0 je spodek hlavy. Hlava je tento profil
# vytazeny do hloubky HD -> tiskne se celem dolu bez podper.
# Kazdy tvar musi obsahnout oci (x +-52, z 25..73), pusu (x +-46, z 10..27) a desku uvnitr.
HEAD_STYLES = ["hranata", "kulata", "kopule", "vejce"]


HEAD_SIZES = {"S": 0.92, "M": 1.0, "L": 1.10}   # vnejsi tvar; deska, oci a pusa zustavaji
HEAD_SIZE_FIX = {("kulata", "S"): 0.95}         # kulata hlava ma uzky spodek -> mensi zmenseni
BODY_SIZES = {"S": 0.90, "M": 1.0, "L": 1.10}
BODY_SIZE_NA = {("zvon", "S")}                   # maly zvon: nahore se nevejde stitek i priruba krku
MIN_EDGE = 2.0                                   # min. okraj od prstence oka / pusy k obrysu hlavy


def head_k(style, size):
    return HEAD_SIZE_FIX.get((style, size), HEAD_SIZES[size])


def body_k(style, size):
    return BODY_SIZES[size]


def mouth_fits(style, mouth, size):
    """Vejde se pusa na celo hlavy dane velikosti (s okrajem MIN_EDGE)?"""
    sh = mouth_shapes(mouth)
    if not sh:
        return True
    prof = head_profile(style, head_k(style, size))
    r = unary_union([s for _, s in sh])
    return prof.contains(r) and prof.exterior.distance(r) >= MIN_EDGE


def head_profile(style, k=1.0):
    p = _head_profile(style)
    return p if k == 1.0 else affinity.scale(p, k, k, origin=(0, 0))


def _head_profile(style):
    if style == "hranata":
        return Polygon([(x, z + HH / 2) for x, z in R.octagon(HW, HH, 7.0)])
    if style == "kulata":            # kruh s rovnym spodkem
        c = Point(0, 49).buffer(60, resolution=64)
        return c.intersection(sbox(-100, 0, 100, 200))
    if style == "kopule":            # hranaty spodek, nahore klenba
        dome = affinity.scale(Point(0, 0).buffer(1, resolution=64), 62, 42).buffer(0)
        dome = affinity.translate(dome, 0, 55)
        base = sbox(-62, 0, 62, 55).buffer(-6).buffer(6)
        return unary_union([base, dome]).intersection(sbox(-100, 0, 100, 200))
    if style == "vejce":             # vejce: dole siroke, nahore uzsi a vyssi
        pts = []
        for i in range(160):
            t = 2 * math.pi * i / 160
            pts.append((66 * math.cos(t) * (1 - 0.18 * math.sin(t)), 50 + 62 * math.sin(t)))
        return Polygon(pts).buffer(0).intersection(sbox(-100, 0, 100, 200))
    raise ValueError(style)


def head_top(style, k=1.0):
    """Vyska temene (z) uprostred - tam je otvor pro antenu."""
    return head_profile(style, k).bounds[3]


def head_posts(style, k=1.0):
    """Polohy 4 zavitovych vlozek pro zadni viko (x, z)."""
    if style == "hranata":
        bx, bz0, bz1 = HW * k / 2 - WALL - 8.0, WALL + 7.0, HH * k - WALL - 7.0
        return [(sx * bx, z) for sx in (-1, 1) for z in (bz0, bz1)]
    inner = head_profile(style, k).buffer(-WALL - 4.8)
    minx, minz, maxx, maxz = inner.bounds
    zc = (minz + maxz) / 2
    out = []
    for ang in (35, 145, 215, 325):
        a = math.radians(ang)
        ray = LineString([(0, zc), (200 * math.cos(a), zc + 200 * math.sin(a))])
        hit = ray.intersection(inner.exterior)
        p = hit if hit.geom_type == "Point" else min(hit.geoms, key=lambda g: g.distance(Point(0, zc)))
        out.append((p.x, max(p.y, WALL + 6.5)))
    return out


def ring_pts(poly):
    return list(poly.exterior.coords)[:-1]


def body_outline(style, w, d, top, inset=0.0):
    kind = BODY_STYLES[style]
    w, d = w - 2 * inset, d - 2 * inset
    if kind[0] == "chamfer":
        return R.octagon(w, d, kind[1] - (1 if inset else 0))
    r = kind[2] if top else kind[1]
    r = max(1.0, r - inset)
    return rounded(w, d, r, r)


# --------------------------------------------------------------------------------------
#  2D tvary (shapely) -> telesa
# --------------------------------------------------------------------------------------
def solid_xz(shape, y0, y1):
    """Plocha v rovine XZ (x, z) vytazena od y0 do y1 (y roste dozadu)."""
    geoms = getattr(shape, "geoms", [shape])
    out = None
    for g in geoms:
        if g.is_empty or g.area < 1e-6:
            continue
        ext = list(g.exterior.coords)[:-1]
        s = cq.Workplane("XZ").polyline(ext).close().extrude(-(y1 - y0))
        for hole in g.interiors:
            pts = list(hole.coords)[:-1]
            s = s.cut(cq.Workplane("XZ").polyline(pts).close().extrude(-(y1 - y0) - 2).translate((0, -1, 0)))
        out = s if out is None else out.union(s)
    return out.translate((0, y0, 0)) if out is not None else None


def solid_xy(shape, z0, z1):
    geoms = getattr(shape, "geoms", [shape])
    out = None
    for g in geoms:
        if g.is_empty or g.area < 1e-6:
            continue
        s = cq.Workplane("XY").polyline(list(g.exterior.coords)[:-1]).close().extrude(z1 - z0)
        for hole in g.interiors:
            s = s.cut(cq.Workplane("XY").polyline(list(hole.coords)[:-1]).close().extrude(z1 - z0 + 2)
                      .translate((0, 0, -1)))
        out = s if out is None else out.union(s)
    return out.translate((0, 0, z0)) if out is not None else None


# --------------------------------------------------------------------------------------
#  PUSA - vlozky v cele hlavy (zapustene 0.8 mm, tisknou se spolu s hlavou)
# --------------------------------------------------------------------------------------
MOUTH_D = 0.8
MOUTHS = ["pruh", "mrizka", "usmev", "zadna"]


def mouth_shapes(kind):
    """Vrati [(role, shapely tvar v rovine XZ)] - role 'mouthA' / 'mouthB'."""
    if kind == "pruh":
        z0, z1, x0, x1 = 13.0, 23.0, -42.0, 42.0
        band = sbox(x0, z0, x1, z1)
        pitch, w, lean = 10.0, 5.0, z1 - z0
        dark = []
        x = x0 - lean - pitch
        while x < x1 + pitch:
            dark.append(Polygon([(x, z0), (x + w, z0), (x + w + lean, z1), (x + lean, z1)]))
            x += pitch
        dark = unary_union(dark).intersection(band)
        return [("mouthA", band.difference(dark)), ("mouthB", dark)]
    if kind == "mrizka":
        slots = [LineString([(-33, z), (33, z)]).buffer(1.1, cap_style=1) for z in (12.0, 15.0, 18.0)]
        frame = sbox(-39, 7.5, 39, 21.5).difference(sbox(-37.5, 9.0, 37.5, 20.0))
        return [("mouthB", unary_union(slots)), ("mouthA", frame)]
    if kind == "usmev":
        cx, cz, r = 0.0, 41.0, 28.0
        arc = LineString([(cx + r * math.cos(math.radians(a)), cz + r * math.sin(math.radians(a)))
                          for a in range(228, 313, 2)])
        smile = arc.buffer(2.6, cap_style=1)
        dimples = unary_union([Point(sx * 23.2, 17.5).buffer(1.6, resolution=24) for sx in (-1, 1)])
        return [("mouthB", smile), ("mouthA", dimples)]
    return []


def mouth_parts(kind):
    """Vlozky pusy v souradnicich hlavy (y 0..0.8 od cela)."""
    return [(role, solid_xz(sh, 0.0, MOUTH_D)) for role, sh in mouth_shapes(kind)]


# --------------------------------------------------------------------------------------
#  HLAVA
# --------------------------------------------------------------------------------------
def head(style, mouth, k=1.0):
    prof = head_profile(style, k)
    outer = cq.Workplane("XZ").polyline(ring_pts(prof)).close().extrude(-HD)
    inner = (cq.Workplane("XZ").polyline(ring_pts(prof.buffer(-WALL, join_style=2))).close()
             .extrude(-(HD - FRONT + 1)).translate((0, FRONT, 0)))
    h = outer.cut(inner)
    top = head_top(style, k)
    zmid = (prof.bounds[1] + top) / 2

    posts = head_posts(style, k)
    for px, pz in posts:
        sx = 1 if px > 0 else -1
        sz = 1 if pz > zmid else -1
        h = h.union(corner_fill(outer, px - sx * 5, pz - sz * 5, sx, sz, "y", HD - 14, HD, HD - 14, steps=-10))
    for px, pz in posts:
        h = h.cut(cyl_y(INS_M3, HD - INS_M3_D, HD + 1, px, pz))

    post_top = DISP_FACE + BRASS_TOP + PAD_H - CLAMP
    h = h.cut(box(-7.5, 7.5, DISP_FACE - 0.6, FRONT + 1, WALL - 0.01, EYE_Z - 2))
    for ex in (-EYE_DX, EYE_DX):
        h = h.cut(cyl_y(DISP_D + 0.6, DISP_FACE, FRONT + 1, ex, EYE_Z))
        sgn = -1 if ex > 0 else 1
        xa = ex + sgn * (DISP_D / 2 - 2.0)
        xb = ex + sgn * (DISP_D / 2 + TAB_OUT + 0.5)
        h = h.cut(box(min(xa, xb), max(xa, xb), DISP_FACE, FRONT + 1, EYE_Z - TAB_W / 2, EYE_Z + TAB_W / 2))
        for wx, wz in de_ears(ex):
            h = h.union(cyl_y(8.0, FRONT - 0.5, post_top, wx, wz))
            h = h.cut(cyl_y(INS_M3, post_top - INS_M3_D, post_top + 1, wx, wz))
    for ex in (-EYE_DX, EYE_DX):
        h = h.cut(cyl_y(EYE_OPEN, -1, FRONT + 1, ex, EYE_Z))
        cone = (cq.Workplane("XZ").circle(EYE_OPEN / 2 + 1.2).workplane(offset=-1.2)
                .circle(EYE_OPEN / 2).loft().translate((ex, 0, EYE_Z)))
        h = h.cut(cone)
        h = h.cut(cyl_y(RING_OD + 0.4, -1, 0.8, ex, EYE_Z))

    # pusa: lozko pro vlozky
    shapes = mouth_shapes(mouth)
    if shapes:
        recess = unary_union([s for _, s in shapes])
        h = h.cut(solid_xz(recess, -0.01, MOUTH_D))

    # bocni zebra (jen hranata hlava - ostatni tvary maji oble boky)
    if style == "hranata":
        h = add_fins(h, k)
    yc = HD / 2
    h = h.cut(cyl_z(NECK_ID, -1, WALL + 1, 0, yc))
    for sy in (-1, 1):
        h = h.cut(cyl_z(CLR_M3, -1, WALL + 1, 0, yc + sy * NECK_HOLE_R))
    h = h.cut(cyl_z(5.3, top - WALL - 3, top + 1, 0, yc))
    return h


def add_fins(h, k=1.0):
    FIN_OUT, FIN_Y0, FIN_Y1 = 7.0, 14.0, HD - 12.0
    run = FIN_OUT / 0.8
    w0 = HW * k / 2 - 0.5
    dz = HH * (k - 1) / 2

    def fin_profile(y_front, y_back, sx):
        p = [(w0, y_front), (w0 + FIN_OUT, y_front + run), (w0 + FIN_OUT, y_back), (w0, y_back)]
        return [(sx * x, y) for x, y in p]

    for sx in (-1, 1):
        prof = fin_profile(FIN_Y0, FIN_Y1, sx)
        for i in range(5):
            zc = 38 + dz + i * 6.5
            h = h.union(cq.Workplane("XY").polyline(prof).close().extrude(2.4).translate((0, 0, zc - 1.2)))
        front = [(sx * w0, FIN_Y0), (sx * (w0 + FIN_OUT), FIN_Y0 + run),
                 (sx * (w0 + FIN_OUT), FIN_Y0 + run + 3), (sx * w0, FIN_Y0 + 3)]
        h = h.union(cq.Workplane("XY").polyline(front).close().extrude(30.5).translate((0, 0, 36 + dz)))
        h = h.union(cq.Workplane("XY").polyline(fin_profile(FIN_Y1 - 3, FIN_Y1, sx)).close()
                    .extrude(30.5).translate((0, 0, 36 + dz)))
    return h


def head_back(style, k=1.0):
    prof = head_profile(style, k)
    t = 3.0
    c = cq.Workplane("XY").polyline(ring_pts(prof)).close().extrude(t)
    try:
        c = c.faces(">Z").edges().chamfer(1.0)
    except Exception:
        pass
    for px, pz in head_posts(style, k):
        c = c.cut(cyl_z(CLR_M3, -1, t + 1, px, pz))
        c = c.cut(cyl_z(HEAD_M3, t - 1.6, t + 1, px, pz))
    zm = 42.0                                   # vetraci drazky a stitek kolem stredu desky
    for i in range(6):
        y = zm - 18 + i * 7
        c = c.cut(box(-30, 30, y - 1.5, y + 1.5, -1, t + 1))
    c = c.cut(box(-34, 34, zm + 24, zm + 34, t - 0.6, t + 1).cut(box(-32, 32, zm + 26, zm + 32, t - 1, t + 2)))
    return (c.rotate((0, 0, 0), (1, 0, 0), -90).rotate((0, 0, 0), (0, 1, 0), 180)
            .translate((0, Y_HEAD + HD, Z_HEAD)))


# --------------------------------------------------------------------------------------
#  TELO + spodni viko, jednotne lozko pro stitek se znakem
# --------------------------------------------------------------------------------------
BADGE_W, BADGE_H, BADGE_R = R.BADGE_W, R.BADGE_H, R.BADGE_R
BADGE_T, BADGE_POCKET, BADGE_CLR, BADGE_ZC = R.BADGE_T, R.BADGE_POCKET, R.BADGE_CLR, R.BADGE_ZC
SLOPE = math.atan(((BD0 - BD1) / 2) / BH)
Y_FACE = -(BD0 / 2 - (BD0 - BD1) / 2 * BADGE_ZC / BH)


def badge_outline(offset=0.0):
    return Polygon(rounded(BADGE_W + 2 * offset, BADGE_H + 2 * offset, BADGE_R + offset, BADGE_R + offset))


def on_chest(s, k=1.0):
    """Teleso postavene v rovine XZ (y = 0 je povrch tela) -> na sikmou celni stenu tela."""
    return s.rotate((0, 0, 0), (1, 0, 0), -math.degrees(SLOPE)).translate((0, Y_FACE * k, BADGE_ZC * k))


def body(style, k=1.0):
    bh = BH * k
    bo, to = body_outline(style, BW0 * k, BD0 * k, False), body_outline(style, BW1 * k, BD1 * k, True)
    outer = cq.Workplane("XY").polyline(bo).close().workplane(offset=bh).polyline(to).close().loft()
    bi = body_outline(style, BW0 * k, BD0 * k, False, WALL)
    ti = body_outline(style, BW1 * k, BD1 * k, True, WALL)
    inner = (cq.Workplane("XY").polyline(bi).close().workplane(offset=bh - WALL).polyline(ti).close()
             .loft().translate((0, 0, -0.01)))
    b = outer.cut(inner)
    try:
        b = b.faces(">Z").edges().chamfer(1.5)
    except Exception:
        pass
    bx, by = BOSS_XY[0] * k, BOSS_XY[1] * k
    for sx in (-1, 1):
        for sy in (-1, 1):
            b = b.union(corner_fill(outer, sx * (bx - 5), sy * (by - 5), sx, sy, "z", 0, 12, 12, steps=12))
    for sx in (-1, 1):
        for sy in (-1, 1):
            b = b.cut(cyl_z(INS_M3, -1, INS_M3_D, sx * bx, sy * by))
    for sx in (-1, 1):
        b = b.union(cyl_z(9.0, bh - WALL - 8, bh - 0.5, sx * NFL_HOLE_X, 0))
        b = b.cut(cyl_z(INS_M3, bh - INS_M3_D, bh + 1, sx * NFL_HOLE_X, 0))
    b = b.cut(cyl_z(NECK_ID, bh - WALL - 1, bh + 1, 0, 0))
    b = b.cut(box(-6, 6, BD0 * k / 2 - 8, BD0 * k / 2 + 2, -1, 8.0))
    b = b.cut(cyl_y(12, BD0 * k / 2 - 8, BD0 * k / 2 + 2, 0, 8.0))
    for sx in (-1, 1):
        for i in range(3):
            z = (22 + i * 6) * k
            x0, x1 = sx * (BW0 * k / 2 - 14), sx * (BW0 * k / 2 + 2)
            b = b.cut(box(min(x0, x1), max(x0, x1), -12, 12, z - 1.5, z + 1.5))
    # lozko pro stitek (v rovine XZ: x = sirka, z = vyska)
    pocket = solid_xz(badge_outline(BADGE_CLR), -1.0, BADGE_POCKET)
    b = b.cut(on_chest(pocket, k))
    return b


def bottom_plate(style, k=1.0):
    t = PLATE_T
    p = cq.Workplane("XY").polyline(body_outline(style, BW0 * k, BD0 * k, False)).close().extrude(t).translate((0, 0, -t))
    p = p.faces("<Z").edges().chamfer(1.0)
    bx, by = BOSS_XY[0] * k, BOSS_XY[1] * k
    for sx in (-1, 1):
        for sy in (-1, 1):
            p = p.cut(cyl_z(CLR_M3, -t - 1, 1, sx * bx, sy * by))
            p = p.cut(cyl_z(HEAD_M3, -t - 1, -t + 1.8, sx * bx, sy * by))
            p = p.cut(cyl_z(11.6, -t - 1, -t + 1.0, sx * (bx - 12), sy * (by - 2)))
    return p


# --------------------------------------------------------------------------------------
#  ZNAK NA HRUDI - stitek (zakladni barva) + znak (vlozka v hornich 0.6 mm)
# --------------------------------------------------------------------------------------
EMBLEMS = ["kolo", "blesk", "srdce", "hvezda", "prazdny"]
SYM_D = 0.6


def emblem_shape(kind):
    """Znak v rovine XZ, stred v (0, 0), vejde se do ~44 x 26 mm."""
    if kind == "kolo":
        teeth = [sbox(-2.4, 0, 2.4, 13.0) for _ in range(10)]
        teeth = unary_union([affinity.rotate(t, i * 36, origin=(0, 0)) for i, t in enumerate(teeth)])
        g = unary_union([Point(0, 0).buffer(10.5, resolution=32), teeth]).difference(Point(0, 0).buffer(4.5))
        return g
    if kind == "blesk":
        return Polygon([(3, 13), (-7, -1), (-1, -1), (-4, -13), (8, 3), (1.5, 3), (5, 13)])
    if kind == "srdce":
        pts = []
        for i in range(120):
            t = 2 * math.pi * i / 120
            x = 16 * math.sin(t) ** 3
            y = 13 * math.cos(t) - 5 * math.cos(2 * t) - 2 * math.cos(3 * t) - math.cos(4 * t)
            pts.append((x * 0.78, y * 0.78 + 1.2))
        return Polygon(pts).buffer(0)
    if kind == "hvezda":
        pts = []
        for i in range(10):
            r = 13.0 if i % 2 == 0 else 5.4
            a = math.radians(90 + i * 36)
            pts.append((r * math.cos(a), r * math.sin(a) - 0.8))
        return Polygon(pts)
    return None


def badge_parts(kind):
    """[(role, teleso)] uz na hrudi tela. Stitek vycniva 0.6 mm nad povrch."""
    plate = solid_xz(badge_outline(), -(BADGE_T - BADGE_POCKET), BADGE_POCKET)
    sh = emblem_shape(kind)
    out = []
    if sh is not None:
        sym = solid_xz(sh, -(BADGE_T - BADGE_POCKET) - 0.01, -(BADGE_T - BADGE_POCKET) + SYM_D)
        sym = sym.intersect(plate)
        plate = plate.cut(sym)
        out.append(("symbol", on_chest(sym)))
    out.insert(0, ("badge", on_chest(plate)))
    return out


# --------------------------------------------------------------------------------------
#  OCI - prstence (tisknou se plochou dolu: z 0..0.8 je v lozku cela, z nahoru = ven)
# --------------------------------------------------------------------------------------
EYES = ["klasik", "nyty", "sestihran", "objektiv", "monokl", "slepe"]
BASE_T = 0.8


def _ring_base():
    return cq.Workplane("XY").circle(RING_OD / 2).circle(RING_ID / 2).extrude(BASE_T)


def eye_parts(kind):
    """[(role, teleso v plose)] - role: 'eyes' (prstenec), 'dark', 'iris', 'monokl'."""
    if kind == "klasik":
        return [("eyes", R.eye_ring())]
    if kind == "nyty":
        r = (cq.Workplane("XY").circle(RING_OD / 2).circle(RING_ID / 2).extrude(RING_T)
             .faces(">Z").edges().chamfer(0.8))
        for i in range(8):
            a = math.radians(22.5 + i * 45)
            x, y = RING_PIN_R * math.cos(a), RING_PIN_R * math.sin(a)
            r = r.union(cq.Workplane("XY").sphere(1.7).translate((x, y, RING_T - 0.4)))
        return [("eyes", r)]
    if kind == "sestihran":
        rc = RING_OD / 2                                     # hrot sestiuhelniku na obvodu lozka
        hexa = cq.Workplane("XY").polygon(6, 2 * rc).extrude(RING_T + 0.4 - BASE_T).translate((0, 0, BASE_T))
        hexa = hexa.faces(">Z").edges().chamfer(0.7)
        hexa = hexa.cut(cyl_z(RING_ID, -1, 10, 0, 0))
        r = _ring_base().union(hexa)
        for i in range(6):
            a = math.radians(i * 60)
            x, y = (rc - 4.2) * math.cos(a), (rc - 4.2) * math.sin(a)
            r = r.union(cq.Workplane("XY").circle(1.5).extrude(0.6).translate((x, y, RING_T + 0.4)))
        return [("eyes", r)]
    if kind == "objektiv":
        outer = (cq.Workplane("XY").circle(RING_OD / 2).circle(RING_OD / 2 - 4.0).extrude(2.6 - BASE_T)
                 .translate((0, 0, BASE_T)))
        for i in range(40):                                  # vroubkovani po obvodu
            a = i * 9.0
            k = (cq.Workplane("XY").rect(1.2, 2.4).extrude(3).translate((RING_OD / 2, 0, BASE_T + 0.6))
                 .rotate((0, 0, 0), (0, 0, 1), a))
            outer = outer.cut(k)
        inner = (cq.Workplane("XY").circle(RING_OD / 2 - 3.6).circle(RING_ID / 2).extrude(3.6 - BASE_T)
                 .translate((0, 0, BASE_T)).faces(">Z").edges().chamfer(1.0))
        return [("eyes", _ring_base().union(outer).union(inner))]
    if kind == "monokl":
        import monokl
        p = monokl.build()
        return [("monokl", p["prstenec_fialovy"]), ("dark", p["praskliny"])]
    if kind == "slepe":
        import cover
        p = cover.build()
        return [("eyes", p["prstenec"]), ("dark", p["displej_cerny"]), ("iris", p["duhovka_azur"])]
    raise ValueError(kind)


def eye_place(s, ex, mirror=False):
    """Prstenec z plochy do sestavy (oko se stredem v ex)."""
    if mirror:
        s = s.mirror("YZ")
    return s.rotate((0, 0, 0), (1, 0, 0), 90).translate((ex, Y_HEAD + 0.8, Z_HEAD + EYE_Z))


# --------------------------------------------------------------------------------------
#  ANTENA - cep do hlavy (D5 x 7) + limec, ruzne spicky
# --------------------------------------------------------------------------------------
ANTENNAS = ["klasik", "kulicka", "dvojita", "radar"]


def _antenna_base():
    a = cyl_z(5.0, 0, 7, 0, 0)
    a = a.union(cyl_z(10.0, 7, 8, 0, 0))
    a = a.union(cq.Workplane("XY").circle(5.0).workplane(offset=3.6).circle(2.0).loft().translate((0, 0, 8)))
    return a


def antenna(kind):
    """Vraci (teleso v sestave, tisknout vzhuru nohama?)."""
    if kind == "klasik":
        a = R.antenna()
        flip = True
    elif kind == "kulicka":
        a = _antenna_base().union(cyl_z(4.0, 9.5, 24, 0, 0))
        # kapka: kuzel 45 st. pod koulí -> tiskne se nastojato bez podper
        a = a.union(cq.Workplane("XY").circle(2.0).workplane(offset=3.2).circle(4.4).loft().translate((0, 0, 23)))
        a = a.union(cq.Workplane("XY").sphere(5.0).translate((0, 0, 28.6)))
        flip = False
    elif kind == "dvojita":
        a = _antenna_base().union(cyl_z(4.6, 9.5, 13, 0, 0))
        for sx in (-1, 1):
            stem = cyl_z(3.0, 0, 15, 0, 0).union(
                cq.Workplane("XY").circle(1.5).workplane(offset=2.0).circle(3.4).loft().translate((0, 0, 14.5)))
            stem = stem.union(cq.Workplane("XY").sphere(3.6).translate((0, 0, 18.6)))
            a = a.union(stem.rotate((0, 0, 0), (0, 1, 0), sx * 22).translate((0, 0, 11.5)))
        flip = False
    elif kind == "radar":
        a = _antenna_base().union(cyl_z(4.0, 9.5, 20, 0, 0))
        # misa: spodek kuzel 45 st., nahore mělka prohlubeň + kolik uprostred
        dish = cq.Workplane("XY").circle(2.0).workplane(offset=6.0).circle(8.0).loft().translate((0, 0, 19))
        dish = dish.union(cyl_z(16.0, 25, 26.2, 0, 0))
        dish = dish.cut(cq.Workplane("XY").sphere(14.0).translate((0, 0, 26.2 + 12.6)))
        a = a.union(dish).union(cyl_z(2.0, 24, 31, 0, 0)).union(cq.Workplane("XY").sphere(1.6).translate((0, 0, 31)))
        flip = False
    else:
        raise ValueError(kind)
    if not flip:
        # "sukne": tenky disk pod cepem, aby antena pri tisku nastojato drzela na podlozce.
        # Po tisku se odstrihne (konec cepu je schovany v hlave).
        a = a.union(cyl_z(26.0, 0, 0.6, 0, 0)).union(
            cq.Workplane("XY").circle(4.0).workplane(offset=1.2).circle(2.5).loft().translate((0, 0, 0.6)))
    return a.translate((0, 0, Z_HEAD + HH - 7)), flip
