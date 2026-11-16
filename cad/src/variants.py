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
HEAD_STYLES = {
    "hranata": ("chamfer", 7.0),
    "obla":    ("round", 12.0, 12.0),
    "retro":   ("round", 17.0, 6.0),     # nahore vetsi zaobleni, dole mensi
}
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


def head_outline(style, inset=0.0):
    kind = HEAD_STYLES[style]
    w, d = HW - 2 * inset, HH - 2 * inset
    if kind[0] == "chamfer":
        return R.octagon(w, d, kind[1] - inset * 0.6)
    return rounded(w, d, max(1.0, kind[1] - inset), max(1.0, kind[2] - inset))


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
MOUTHS = ["pruh", "mrizka", "usmev", "led", "zadna"]


def mouth_shapes(kind):
    """Vrati [(role, shapely tvar v rovine XZ)] - role 'mouthA' / 'mouthB'."""
    if kind == "pruh":
        z0, z1, x0, x1 = 13.0, 23.0, -46.0, 46.0
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
        slots = [LineString([(-36, z), (36, z)]).buffer(1.25, cap_style=1) for z in (12.5, 16.0, 19.5, 23.0)]
        frame = sbox(-42, 9.5, 42, 26).difference(sbox(-40.5, 11.0, 40.5, 24.5))
        return [("mouthB", unary_union(slots)), ("mouthA", frame)]
    if kind == "usmev":
        cx, cz, r = 0.0, 44.0, 28.0
        arc = LineString([(cx + r * math.cos(math.radians(a)), cz + r * math.sin(math.radians(a)))
                          for a in range(228, 313, 2)])
        smile = arc.buffer(2.6, cap_style=1)
        dimples = unary_union([Point(sx * 23.2, 20.5).buffer(1.6, resolution=24) for sx in (-1, 1)])
        return [("mouthB", smile), ("mouthA", dimples)]
    if kind == "led":
        xs = [-36 + 9 * i for i in range(9)]
        a = [Point(x, 18).buffer(3.2, resolution=24) for i, x in enumerate(xs) if i % 2 == 0]
        b = [Point(x, 18).buffer(3.2, resolution=24) for i, x in enumerate(xs) if i % 2 == 1]
        return [("mouthA", unary_union(a)), ("mouthB", unary_union(b))]
    return []


def mouth_parts(kind):
    """Vlozky pusy v souradnicich hlavy (y 0..0.8 od cela)."""
    return [(role, solid_xz(sh, 0.0, MOUTH_D)) for role, sh in mouth_shapes(kind)]


# --------------------------------------------------------------------------------------
#  HLAVA
# --------------------------------------------------------------------------------------
def head(style, mouth):
    pts = [(x, z + HH / 2) for x, z in head_outline(style)]
    outer = cq.Workplane("XZ").polyline(pts).close().extrude(-HD)
    ipts = [(x, z + HH / 2) for x, z in head_outline(style, WALL)]
    inner = cq.Workplane("XZ").polyline(ipts).close().extrude(-(HD - FRONT + 1)).translate((0, FRONT, 0))
    h = outer.cut(inner)

    bx, bz0, bz1 = HW / 2 - WALL - 8.0, WALL + 7.0, HH - WALL - 7.0
    for sx in (-1, 1):
        for zc in (bz0, bz1):
            sz = -1 if zc < HH / 2 else 1
            h = h.union(corner_fill(outer, sx * bx - sx * 5, zc - sz * 5, sx, sz, "y", HD - 14, HD, HD - 14, steps=-10))
    for sx in (-1, 1):
        for zc in (bz0, bz1):
            h = h.cut(cyl_y(INS_M3, HD - INS_M3_D, HD + 1, sx * bx, zc))

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

    # bocni zebra
    FIN_OUT, FIN_Y0, FIN_Y1 = 7.0, 14.0, HD - 12.0
    run = FIN_OUT / 0.8
    w0 = HW / 2 - 0.5

    def fin_profile(y_front, y_back, sx):
        p = [(w0, y_front), (w0 + FIN_OUT, y_front + run), (w0 + FIN_OUT, y_back), (w0, y_back)]
        return [(sx * x, y) for x, y in p]

    for sx in (-1, 1):
        prof = fin_profile(FIN_Y0, FIN_Y1, sx)
        for i in range(5):
            zc = 38 + i * 6.5
            h = h.union(cq.Workplane("XY").polyline(prof).close().extrude(2.4).translate((0, 0, zc - 1.2)))
        front = [(sx * w0, FIN_Y0), (sx * (w0 + FIN_OUT), FIN_Y0 + run),
                 (sx * (w0 + FIN_OUT), FIN_Y0 + run + 3), (sx * w0, FIN_Y0 + 3)]
        h = h.union(cq.Workplane("XY").polyline(front).close().extrude(30.5).translate((0, 0, 36)))
        h = h.union(cq.Workplane("XY").polyline(fin_profile(FIN_Y1 - 3, FIN_Y1, sx)).close()
                    .extrude(30.5).translate((0, 0, 36)))

    yc = HD / 2
    h = h.cut(cyl_z(NECK_ID, -1, WALL + 1, 0, yc))
    for sy in (-1, 1):
        h = h.cut(cyl_z(CLR_M3, -1, WALL + 1, 0, yc + sy * NECK_HOLE_R))
    h = h.cut(cyl_z(5.3, HH - WALL - 1, HH + 1, 0, yc))
    return h


def head_back(style):
    pts = head_outline(style)
    t = 3.0
    c = cq.Workplane("XY").polyline(pts).close().extrude(t)
    c = c.faces(">Z").edges().chamfer(1.0)
    bx, bz0, bz1 = HW / 2 - WALL - 8.0, WALL + 7.0, HH - WALL - 7.0
    for sx in (-1, 1):
        for zc in (bz0, bz1):
            y = zc - HH / 2
            c = c.cut(cyl_z(CLR_M3, -1, t + 1, sx * bx, y))
            c = c.cut(cyl_z(HEAD_M3, t - 1.6, t + 1, sx * bx, y))
    for i in range(6):
        y = -18 + i * 7
        c = c.cut(box(-30, 30, y - 1.5, y + 1.5, -1, t + 1))
    c = c.cut(box(-34, 34, 24, 34, t - 0.6, t + 1).cut(box(-32, 32, 26, 32, t - 1, t + 2)))
    # do sestavy (stejne jako robot.build)
    return (c.rotate((0, 0, 0), (1, 0, 0), -90).rotate((0, 0, 0), (0, 1, 0), 180)
            .translate((0, Y_HEAD + HD, Z_HEAD + HH / 2)))


# --------------------------------------------------------------------------------------
#  TELO + spodni viko, jednotne lozko pro stitek se znakem
# --------------------------------------------------------------------------------------
BADGE_W, BADGE_H, BADGE_R = 50.0, 32.0, 6.0
BADGE_T, BADGE_POCKET, BADGE_CLR, BADGE_ZC = 1.6, 1.0, 0.15, 23.0
SLOPE = math.atan(((BD0 - BD1) / 2) / BH)
Y_FACE = -(BD0 / 2 - (BD0 - BD1) / 2 * BADGE_ZC / BH)


def badge_outline(offset=0.0):
    return Polygon(rounded(BADGE_W + 2 * offset, BADGE_H + 2 * offset, BADGE_R + offset, BADGE_R + offset))


def on_chest(s):
    """Teleso postavene v rovine XZ (y = 0 je povrch tela) -> na sikmou celni stenu tela."""
    return s.rotate((0, 0, 0), (1, 0, 0), -math.degrees(SLOPE)).translate((0, Y_FACE, BADGE_ZC))


def body(style):
    bo, to = body_outline(style, BW0, BD0, False), body_outline(style, BW1, BD1, True)
    outer = cq.Workplane("XY").polyline(bo).close().workplane(offset=BH).polyline(to).close().loft()
    bi = body_outline(style, BW0, BD0, False, WALL)
    ti = body_outline(style, BW1, BD1, True, WALL)
    inner = (cq.Workplane("XY").polyline(bi).close().workplane(offset=BH - WALL).polyline(ti).close()
             .loft().translate((0, 0, -0.01)))
    b = outer.cut(inner)
    try:
        b = b.faces(">Z").edges().chamfer(1.5)
    except Exception:
        pass
    bx, by = BOSS_XY
    for sx in (-1, 1):
        for sy in (-1, 1):
            b = b.union(corner_fill(outer, sx * (bx - 5), sy * (by - 5), sx, sy, "z", 0, 12, 12, steps=12))
    for sx in (-1, 1):
        for sy in (-1, 1):
            b = b.cut(cyl_z(INS_M3, -1, INS_M3_D, sx * bx, sy * by))
    for sx in (-1, 1):
        b = b.union(cyl_z(9.0, BH - WALL - 8, BH - 0.5, sx * NFL_HOLE_X, 0))
        b = b.cut(cyl_z(INS_M3, BH - INS_M3_D, BH + 1, sx * NFL_HOLE_X, 0))
    b = b.cut(cyl_z(NECK_ID, BH - WALL - 1, BH + 1, 0, 0))
    b = b.cut(box(-6, 6, BD0 / 2 - 8, BD0 / 2 + 2, -1, 8.0))
    b = b.cut(cyl_y(12, BD0 / 2 - 8, BD0 / 2 + 2, 0, 8.0))
    for sx in (-1, 1):
        for i in range(3):
            z = 22 + i * 6
            x0, x1 = sx * (BW0 / 2 - 14), sx * (BW0 / 2 + 2)
            b = b.cut(box(min(x0, x1), max(x0, x1), -12, 12, z - 1.5, z + 1.5))
    # lozko pro stitek (v rovine XZ: x = sirka, z = vyska)
    pocket = solid_xz(badge_outline(BADGE_CLR), -1.0, BADGE_POCKET)
    b = b.cut(on_chest(pocket))
    return b


def bottom_plate(style):
    t = PLATE_T
    p = cq.Workplane("XY").polyline(body_outline(style, BW0, BD0, False)).close().extrude(t).translate((0, 0, -t))
    p = p.faces("<Z").edges().chamfer(1.0)
    bx, by = BOSS_XY
    for sx in (-1, 1):
        for sy in (-1, 1):
            p = p.cut(cyl_z(CLR_M3, -t - 1, 1, sx * bx, sy * by))
            p = p.cut(cyl_z(HEAD_M3, -t - 1, -t + 1.8, sx * bx, sy * by))
            p = p.cut(cyl_z(11.6, -t - 1, -t + 1.0, sx * (bx - 12), sy * (by - 2)))
    return p


# --------------------------------------------------------------------------------------
#  ZNAK NA HRUDI - stitek (zakladni barva) + znak (vlozka v hornich 0.6 mm)
# --------------------------------------------------------------------------------------
EMBLEMS = ["logo", "kolo", "blesk", "srdce", "hvezda", "prazdny"]
SYM_D = 0.6


def emblem_shape(kind):
    """Znak v rovine XZ, stred v (0, 0), vejde se do ~44 x 26 mm."""
    if kind == "logo":
        return R.logo_polygon(40.0, 0.0)
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
    return a.translate((0, 0, Z_HEAD + HH - 7)), flip
