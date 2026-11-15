"""NW3D-01 - stolni robot s ocima z desky Waveshare ESP32-S3-DualEye-LCD-1.28
Parametricky model v CadQuery. Jednotky: mm.
Souradnice sestavy: X = sirka (vpravo), Y = hloubka (dozadu, predek je -Y), Z = nahoru.
"""
import math
import cadquery as cq

# ---------------- TOLERANCE A SPOJOVACI MATERIAL ----------------
TOL = 0.25            # vule pro lozka
INS_M3 = 4.25         # otvor pro vlozku M3 (Rotorama OD 4.5: doporuceno 4.2-4.3)
INS_M3_D = 7.5        # hloubka otvoru pro vlozku M3 (sroub M3x8 nedosedne na dno)
INS_M2 = 3.2          # otvor pro vlozku M2 kratkou (overit v tabulce vyrobce)
CLR_M3 = 3.4          # pruchozi otvor M3
HEAD_M3 = 6.2         # zapusteni hlavy M3

# ---------------- DISPLEJ ER-TFTM1.28-1 (vykres buydisplay) ----------------
PCB_W, PCB_H, PCB_T = 44.0, 36.0, 1.5
LCD_T = 3.2                          # tloustka LCD nad DPS (4.7 celkem - 1.5 DPS)
AA_CX, AA_CY = 21.3, 18.0           # stred LCD od leveho/spodniho okraje DPS (pohled zepredu)
HOLES = [(6.3, 3.0), (6.3, 33.0), (36.3, 3.0), (36.3, 33.0)]  # otvory D2.0, roztec 30 x 30
EYE_OPEN = 33.0                      # prumer vyrezu oka (AA = 32.4)

# ---------------- HLAVA ----------------
HW, HH, HD = 124.0, 84.0, 64.0       # sirka, vyska, hloubka
HCH = 7.0                            # zkoseni podelnych hran
WALL = 2.4
FRONT = 3.6                          # tloustka celni steny
EYE_DX, EYE_Z = 28.5, 49.0           # stred oka: x = +-28.5 (rozteč desky 57 mm), z = 52

# ---------------- WAVESHARE ESP32-S3-DualEye-LCD-1.28 ----------------
DISP_D = 36.5                        # prumer kulateho modulu (cerny ramecek displeje)
# mosazne zavity M2 na zadni strane; (u, v) vuci stredu oka, u = smerem ven od osy, v = nahoru
DE_HOLES = [(-(28.5 - 18.10), 11.36), ((39.00 - 28.5), -11.29)]
BRASS_TOP = 8.0                      # cela displeje -> vrchol mosazneho zavitu (zmereno 8 mm)
PAD_H = 1.0                          # podlozky na drzaku (odsazeni drzaku nad soucastkami)
BR_T = 4.0                           # tloustka drzaku
CLAMP = 0.5                          # o kolik jsou sloupky nizsi -> drzak displej pritlaci
EAR_R = 24.0                         # vzdalenost ok drzaku od stredu oka (po diagonale)
POCKET = 1.0                         # hloubka luzka pro ramecek displeje v celni stene
GLASS_T = 2.2                        # sklo displeje nad DPS (zmereno)
PCB_T_DE = 1.6                       # tloustka DPS desky DualEye
TAB_OUT = 2.5                        # vystupek pripojeni displeje: celkova sirka 39 mm (zmereno) - 36.5
TAB_W = 18.0                         # sirka vybrani pro vystupek (s rezervou)
DISP_FACE = FRONT - POCKET           # y cela displeje (od predni plochy hlavy)

# ---------------- KRK ----------------
NECK_OD, NECK_ID, NECK_H = 34.0, 18.0, 8.0
NFL_W, NFL_D, NFL_T = 56.0, 50.0, 4.0
NECK_HOLE_R = 13.0                   # vlozky M3 v horni casti krku (0, +-13)
NFL_HOLE_X = 21.0                    # srouby spodni priruby (+-21, 0)

# ---------------- TELO ----------------
BW0, BD0 = 100.0, 70.0               # spodni obrys
BW1, BD1 = 76.0, 56.0                # horni obrys
BH = 46.0
BCH = 6.0
PLATE_T = 3.0
BOSS_XY = (38.0, 22.0)               # vlozky pro spodni viko

# ---------------- LOGO NA HRUDI (samostatny dil, vlepuje se) ----------------
LOGO_FILE = __import__("os").path.join(__import__("os").path.dirname(__file__), "logo.png")
LOGO_W = 46.0                        # sirka loga
LOGO_T = 1.6                         # tloustka vytisteneho loga
LOGO_POCKET = 1.0                    # hloubka luzka v tele (logo vycniva 0.6 mm)
LOGO_CLR = 0.15                      # vule kolem loga v luzku
LOGO_ZC = 23.0                       # vyska stredu loga na tele

# ---------------- ESP32-S3-DevKitC-1 ----------------
ESP_L, ESP_W = 63.0, 25.5            # priblizne, deska lezi obracene (piny nahoru)
PLAT_H = 6.0                         # vyska podstavce pod plechovym krytem modulu


def octagon(w, d, ch):
    """Body osmiuhelniku (obdelnik se zkosenymi rohy) v rovine."""
    x, y = w / 2, d / 2
    return [(-x + ch, -y), (x - ch, -y), (x, -y + ch), (x, y - ch),
            (x - ch, y), (-x + ch, y), (-x, y - ch), (-x, -y + ch)]


def box(x0, x1, y0, y1, z0, z1):
    return cq.Workplane("XY").box(x1 - x0, y1 - y0, z1 - z0, centered=False).translate((x0, y0, z0))


def cyl_y(d, y0, y1, x, z):
    """Valec v ose Y."""
    return cq.Workplane("XZ").circle(d / 2).extrude(-(y1 - y0)).translate((x, y0, z))


def cyl_z(d, z0, z1, x, y):
    return cq.Workplane("XY").circle(d / 2).extrude(z1 - z0).translate((x, y, z0))


def corner_fill(env, a0, b0, sa, sb, axis, c0, c1, taper_from, steps=10, k=0.75):
    """Rohova vyztuha: plny blok od (a0,b0) smerem k rohu (znamenka sa, sb) v rovine kolme na 'axis',
    v rozsahu c0..c1 podel osy. Od taper_from (smer dle znamenka steps) se hladce zuzuje do rohu:
    vnitrni hrany ustupuji o k mm na 1 mm vysky (k = 0.75 -> plocha cca 53 st. od vodorovne),
    takze se tiskne bez podper (zadne schody / previsy)."""
    BIG, H = 200.0, 40.0
    d = 1 if steps > 0 else -1

    def rect(a, b):
        A = (a, a + sa * BIG); B = (b, b + sb * BIG)
        return [(A[0], B[0]), (A[1], B[0]), (A[1], B[1]), (A[0], B[1])]

    if axis == "z":
        A0, A1 = sorted((a0, a0 + sa * BIG)); B0, B1 = sorted((b0, b0 + sb * BIG))
        f = box(A0, A1, B0, B1, min(c0, c1), max(c0, c1))
        wp = cq.Workplane("XY", origin=(0, 0, taper_from))
        off = d * H
    else:  # axis == "y": a = x, b = z
        A0, A1 = sorted((a0, a0 + sa * BIG)); B0, B1 = sorted((b0, b0 + sb * BIG))
        f = box(A0, A1, min(c0, c1), max(c0, c1), B0, B1)
        wp = cq.Workplane("XZ", origin=(0, taper_from, 0))
        off = -d * H                                   # normala roviny XZ miri do -Y
    taper = (wp.polyline(rect(a0, b0)).close()
             .workplane(offset=off).polyline(rect(a0 + sa * k * H, b0 + sb * k * H)).close()
             .loft(ruled=True))
    return f.union(taper).intersect(env)


def pcb_to_world(eye_x, px, pz):
    """Prevede souradnici na DPS (zleva/zdola, pri pohledu zepredu) na X,Z sestavy."""
    return eye_x - AA_CX + px, EYE_Z - AA_CY + pz


# =====================================================================
#  HLAVA (tiskne se celem dolu)
# =====================================================================
_LOGO_CACHE = {}


def logo_polygon(width, offset):
    """Obrys loga z PNG (azurove tahy) jako shapely polygon v mm, vycentrovany."""
    key = (width, offset)
    if key in _LOGO_CACHE:
        return _LOGO_CACHE[key]
    import numpy as np
    from skimage import io, measure
    from shapely.geometry import Polygon
    im = io.imread(LOGO_FILE)[..., :3].astype(float)
    mask = (im[..., 0] < 140) & (im[..., 1] > 150) & (im[..., 2] > 150)
    mask = np.pad(mask, 2)
    cs = measure.find_contours(mask.astype(float), 0.5)
    polys = sorted([Polygon(c[:, ::-1]).buffer(0) for c in cs if len(c) > 20], key=lambda p: -p.area)
    shape = polys[0]
    for hole in polys[1:]:
        shape = shape.difference(hole)
    minx, miny, maxx, maxy = shape.bounds
    k = width / (maxx - minx)
    from shapely import affinity
    shape = affinity.translate(shape, -(minx + maxx) / 2, -(miny + maxy) / 2)
    shape = affinity.scale(shape, k, -k, origin=(0, 0))      # obraz ma osu y dolu
    shape = shape.buffer(offset, join_style=1).simplify(0.03)
    _LOGO_CACHE[key] = shape
    return shape


def logo_solid(width, offset, y0, y1):
    """Logo jako teleso v rovine XZ (x = sirka, z = vyska), od y0 do y1."""
    shape = logo_polygon(width, offset)
    geoms = getattr(shape, "geoms", [shape])
    solid = None
    for g in geoms:
        ext = list(g.exterior.coords)[:-1]
        sld = cq.Workplane("XZ").polyline(ext).close().extrude(-(y1 - y0))
        for interior in g.interiors:
            pts = list(interior.coords)[:-1]
            sld = sld.cut(cq.Workplane("XZ").polyline(pts).close().extrude(-(y1 - y0) - 2).translate((0, -1, 0)))
        solid = sld if solid is None else solid.union(sld)
    return solid.translate((0, y0, 0))


def head_envelope():
    # profil v rovine XZ, vytazeny do +Y
    pts = [(x, z + HH / 2) for x, z in octagon(HW, HH, HCH)]
    return cq.Workplane("XZ").polyline(pts).close().extrude(-HD)


def stripe_band():
    """Pasy vystrazneho pruhu pod ocima: vrati (cely pas, cerne pruhy, zlute pruhy)."""
    z0, z1, x0, x1, depth = 13.0, 23.0, -46.0, 46.0, 0.8
    band = box(x0, x1, 0, depth, z0, z1)
    pitch, w, lean = 10.0, 5.0, (z1 - z0)
    dark = None
    x = x0 - lean - pitch
    while x < x1 + pitch:
        p = (cq.Workplane("XZ")
             .polyline([(x, z0), (x + w, z0), (x + w + lean, z1), (x + lean, z1)]).close()
             .extrude(-depth))
        dark = p if dark is None else dark.union(p)
        x += pitch
    dark = dark.intersect(band)
    yellow = band.cut(dark)
    return band, dark, yellow


def de_world(ex, u, v):
    """Lokalni souradnice oka (u ven od osy, v nahoru) -> X, Z sestavy."""
    sgn = 1 if ex > 0 else -1
    return ex + sgn * u, EYE_Z + v


def de_ears(ex):
    """Pozice dvou ok drzaku (pokracovani diagonaly mezi zavity)."""
    out = []
    for u, v in DE_HOLES:
        r = math.hypot(u, v)
        out.append(de_world(ex, u / r * EAR_R, v / r * EAR_R))
    return out


def head():
    outer = head_envelope()
    inner_pts = [(x, z + HH / 2) for x, z in octagon(HW - 2 * WALL, HH - 2 * WALL, HCH - WALL * 0.6)]
    inner = cq.Workplane("XZ").polyline(inner_pts).close().extrude(-(HD - FRONT + 1)).translate((0, FRONT, 0))
    h = outer.cut(inner)

    # rohove sloupky pro zadni viko (vlozky M3 zezadu)
    bx, bz0, bz1 = HW / 2 - WALL - 8.0, WALL + 7.0, HH - WALL - 7.0
    for sx in (-1, 1):
        for zc in (bz0, bz1):
            sz = -1 if zc < HH / 2 else 1
            h = h.union(corner_fill(outer, sx * bx - sx * 5, zc - sz * 5, sx, sz, "y", HD - 14, HD, HD - 14, steps=-10))
    for sx in (-1, 1):
        for zc in (bz0, bz1):
            h = h.cut(cyl_y(INS_M3, HD - INS_M3_D, HD + 1, sx * bx, zc))

    # dvojdeska DualEye: lůžko pro ramecek displeje + sloupky pro drzak (vlozky M3)
    post_top = DISP_FACE + BRASS_TOP + PAD_H - CLAMP
    # kanal pro konektor USB-C (stredni mustek desky, kabel miri dolu)
    h = h.cut(box(-7.5, 7.5, DISP_FACE - 0.6, FRONT + 1, WALL - 0.01, EYE_Z - 2))
    for ex in (-EYE_DX, EYE_DX):
        h = h.cut(cyl_y(DISP_D + 0.6, DISP_FACE, FRONT + 1, ex, EYE_Z))
        # vybrani pro vystupek (pripajeni displeje) na vnitrni strane oka
        sgn = -1 if ex > 0 else 1                       # smer k ose robota
        xa = ex + sgn * (DISP_D / 2 - 2.0)
        xb = ex + sgn * (DISP_D / 2 + TAB_OUT + 0.5)
        h = h.cut(box(min(xa, xb), max(xa, xb), DISP_FACE, FRONT + 1, EYE_Z - TAB_W / 2, EYE_Z + TAB_W / 2))
        for wx, wz in de_ears(ex):
            h = h.union(cyl_y(8.0, FRONT - 0.5, post_top, wx, wz))
            h = h.cut(cyl_y(INS_M3, post_top - INS_M3_D, post_top + 1, wx, wz))
    for ex in (-EYE_DX, EYE_DX):
        # vyrez oka se zkosenim
        h = h.cut(cyl_y(EYE_OPEN, -1, FRONT + 1, ex, EYE_Z))
        cone = (cq.Workplane("XZ").circle(EYE_OPEN / 2 + 1.2).workplane(offset=-1.2)
                .circle(EYE_OPEN / 2).loft().translate((ex, 0, EYE_Z)))
        h = h.cut(cone)
        # mělké lůžko pro prstenec oka + otvory pro kolíky
        h = h.cut(cyl_y(RING_OD + 0.4, -1, 0.8, ex, EYE_Z))

    # vystrazny pruh: cele pole zapustene 0.8 mm (vyplni se dvoubarevnou vlozkou)
    band, _, _ = stripe_band()
    h = h.cut(band.translate((0, -0.0001, 0)))

    # boční žebra (chladič) - pri tisku celem dolu jsou svisle; predni konec vybiha ze steny
    # sikmo (cca 50 st), aby nevznikl previs
    FIN_OUT, FIN_Y0, FIN_Y1 = 7.0, 14.0, HD - 12.0
    run = FIN_OUT / 0.8

    def fin_profile(y_front, y_back, sx):
        w0 = HW / 2 - 0.5
        pts = [(w0, y_front), (w0 + FIN_OUT, y_front + run), (w0 + FIN_OUT, y_back), (w0, y_back)]
        return [(sx * x, y) for x, y in pts]

    for sx in (-1, 1):
        prof = fin_profile(FIN_Y0, FIN_Y1, sx)
        for i in range(5):
            zc = 38 + i * 6.5
            h = h.union(cq.Workplane("XY").polyline(prof).close().extrude(2.4).translate((0, 0, zc - 1.2)))
        # spojovaci listy: predni sikma (rovnobezna s cely zeber), zadni rovna
        w0 = HW / 2 - 0.5
        front = [(sx * w0, FIN_Y0), (sx * (w0 + FIN_OUT), FIN_Y0 + run),
                 (sx * (w0 + FIN_OUT), FIN_Y0 + run + 3), (sx * w0, FIN_Y0 + 3)]
        h = h.union(cq.Workplane("XY").polyline(front).close().extrude(30.5).translate((0, 0, 36)))
        h = h.union(cq.Workplane("XY").polyline(fin_profile(FIN_Y1 - 3, FIN_Y1, sx)).close()
                    .extrude(30.5).translate((0, 0, 36)))

    # dno: kabelovy otvor + 2 srouby M3 do krku
    yc = HD / 2
    h = h.cut(cyl_z(NECK_ID, -1, WALL + 1, 0, yc))
    for sy in (-1, 1):
        h = h.cut(cyl_z(CLR_M3, -1, WALL + 1, 0, yc + sy * NECK_HOLE_R))
    # otvor pro antenu nahore
    h = h.cut(cyl_z(5.3, HH - WALL - 1, HH + 1, 0, yc))
    # zkoseni predni hrany (cisty vzhled)
    return h


RING_OD, RING_ID, RING_T = 47.0, 34.0, 3.0
RING_PIN_R = (47.0 + 34.0) / 4


def eye_ring():
    """Prstenec oka - samostatny dil (tiskne se plochou dolu, jina barva)."""
    r = (cq.Workplane("XY").circle(RING_OD / 2).circle(RING_ID / 2).extrude(RING_T)
         .faces(">Z").edges().chamfer(0.8))
    # 4 dekorativni sestihranne "sroubky"
    for a in (45, 135, 225, 315):
        x = RING_PIN_R * math.cos(math.radians(a))
        y = RING_PIN_R * math.sin(math.radians(a))
        r = r.union(cq.Workplane("XY").polygon(6, 4.6).extrude(0.9).translate((x, y, RING_T)))
        r = r.cut(cq.Workplane("XY").polygon(6, 2.0).extrude(1).translate((x, y, RING_T + 0.4)))
    return r


def eye_bracket(ex):
    """Drzak jednoho oka: diagonalni lista pres oba mosazne zavity M2, oka M3 do sloupku hlavy.
    Vraci dil v souradnicich sestavy (hlava, y od celni plochy)."""
    (x1, z1), (x2, z2) = de_ears(ex)
    ang = math.degrees(math.atan2(z2 - z1, x2 - x1))
    length = math.hypot(x2 - x1, z2 - z1)
    yb = DISP_FACE + BRASS_TOP + PAD_H
    bar = (cq.Workplane("XZ").center((x1 + x2) / 2, (z1 + z2) / 2)
           .slot2D(length + 10, 9, ang).extrude(-BR_T).translate((0, yb, 0)))
    b = bar
    for u, v in DE_HOLES:
        wx, wz = de_world(ex, u, v)
        b = b.union(cyl_y(5.5, DISP_FACE + BRASS_TOP, yb + 0.1, wx, wz))
        b = b.cut(cyl_y(2.4, 0, 30, wx, wz))
        b = b.cut(cyl_y(4.4, yb + BR_T - 1.6, 30, wx, wz))      # zapusteni hlavy M2
    for wx, wz in de_ears(ex):
        b = b.cut(cyl_y(CLR_M3, 0, 30, wx, wz))
        b = b.cut(cyl_y(HEAD_M3, yb + BR_T - 1.6, 30, wx, wz))
    return b


def head_back():
    pts = octagon(HW, HH, HCH)
    t = 3.0
    c = cq.Workplane("XY").polyline(pts).close().extrude(t)
    c = c.edges(">Z").chamfer(1.0)
    bx, bz0, bz1 = HW / 2 - WALL - 8.0, WALL + 7.0, HH - WALL - 7.0
    for sx in (-1, 1):
        for zc in (bz0, bz1):
            y = zc - HH / 2
            c = c.cut(cyl_z(CLR_M3, -1, t + 1, sx * bx, y))
            c = c.cut(cyl_z(HEAD_M3, t - 1.6, t + 1, sx * bx, y))
    # vetraci drazky
    for i in range(6):
        y = -18 + i * 7
        c = c.cut(box(-30, 30, y - 1.5, y + 1.5, -1, t + 1))
    # typovy stitek - zapusteny ramecek
    c = c.cut(box(-34, 34, 24, 34, t - 0.6, t + 1).cut(box(-32, 32, 26, 32, t - 1, t + 2)))
    return c


def antenna():
    a = cyl_z(5.0, 0, 7, 0, 0)                      # cep do hlavy
    a = a.union(cyl_z(10.0, 7, 8, 0, 0))           # limec (dosedne na hlavu)
    # kuzelovy prechod limce do stozaru (~50 st) -> tisk hlavici dolu bez podper
    a = a.union(cq.Workplane("XY").circle(5.0).workplane(offset=3.6).circle(2.0).loft().translate((0, 0, 8)))
    a = a.union(cyl_z(4.0, 9.5, 23, 0, 0))         # stozar
    cap = (cq.Workplane("XY").polygon(8, 9.5).extrude(5).edges("<Z").chamfer(1.0)
           .edges(">Z").chamfer(0.4).translate((0, 0, 22)))   # spicka lezi pri tisku na podlozce
    a = a.union(cap)
    return a


# =====================================================================
#  KRK
# =====================================================================
def neck():
    n = cq.Workplane("XY").rect(NFL_W, NFL_D).extrude(NFL_T).edges("|Z").chamfer(5)
    n = n.union(cyl_z(NECK_OD, 0, NFL_T + NECK_H, 0, 0))
    # dekorativni drazky
    # drazky tvaru V (steny cca 52 st od vodorovne) -> tisk bez podper
    r0, dep, half = NECK_OD / 2, 0.7, 0.9
    for z in (NFL_T + 2.5, NFL_T + 5.5):
        groove = (cq.Workplane("XZ").polyline([(r0 + 0.5, z - half - 0.5 * half / dep), (r0 - dep, z),
                                               (r0 + 0.5, z + half + 0.5 * half / dep)]).close()
                  .revolve(360, (0, 0, 0), (0, 1, 0)))
        n = n.cut(groove)
    n = n.cut(cyl_z(NECK_ID, -1, NFL_T + NECK_H + 1, 0, 0))
    for sy in (-1, 1):
        n = n.cut(cyl_z(INS_M3, NFL_T + NECK_H - INS_M3_D, NFL_T + NECK_H + 1, 0, sy * NECK_HOLE_R))
    for sx in (-1, 1):
        n = n.cut(cyl_z(CLR_M3, -1, NFL_T + 1, sx * NFL_HOLE_X, 0))
    return n


# =====================================================================
#  TELO (tiskne se vzhuru nohama)
# =====================================================================
def body():
    outer = (cq.Workplane("XY").polyline(octagon(BW0, BD0, BCH)).close()
             .workplane(offset=BH).polyline(octagon(BW1, BD1, BCH)).close().loft())
    inner = (cq.Workplane("XY").polyline(octagon(BW0 - 2 * WALL, BD0 - 2 * WALL, BCH - 1)).close()
             .workplane(offset=BH - WALL).polyline(octagon(BW1 - 2 * WALL, BD1 - 2 * WALL, BCH - 1)).close()
             .loft().translate((0, 0, -0.01)))
    b = outer.cut(inner)
    b = b.edges(">Z").chamfer(1.5)

    # sloupky pro spodni viko
    bx, by = BOSS_XY
    for sx in (-1, 1):
        for sy in (-1, 1):
            b = b.union(corner_fill(outer, sx * (bx - 5), sy * (by - 5), sx, sy, "z", 0, 12, 12, steps=12))
    for sx in (-1, 1):
        for sy in (-1, 1):
            b = b.cut(cyl_z(INS_M3, -1, INS_M3_D, sx * bx, sy * by))

    # sloupky pro vlozky krku (pod hornim vikem)
    for sx in (-1, 1):
        b = b.union(cyl_z(9.0, BH - WALL - 8, BH - 0.5, sx * NFL_HOLE_X, 0))
        b = b.cut(cyl_z(INS_M3, BH - INS_M3_D, BH + 1, sx * NFL_HOLE_X, 0))
    # kabelovy otvor do krku
    b = b.cut(cyl_z(NECK_ID, BH - WALL - 1, BH + 1, 0, 0))

    # vystup kabelu USB-C (zadni strana dole, kabel vede z hlavy krkem)
    b = b.cut(box(-6, 6, BD0 / 2 - 8, BD0 / 2 + 2, -1, 8.0))
    b = b.cut(cyl_y(12, BD0 / 2 - 8, BD0 / 2 + 2, 0, 8.0))
    # vetraci drazky na bocich
    for sx in (-1, 1):
        for i in range(3):
            z = 22 + i * 6
            x0, x1 = sx * (BW0 / 2 - 14), sx * (BW0 / 2 + 2)
            b = b.cut(box(min(x0, x1), max(x0, x1), -14, 14, z - 1.5, z + 1.5))

    # luzko pro logo na predni (sikme) strane
    slope = math.atan(((BD0 - BD1) / 2) / BH)
    zc = LOGO_ZC
    yface = -(BD0 / 2 - (BD0 - BD1) / 2 * zc / BH)
    b = b.cut(logo_solid(LOGO_W, LOGO_CLR, -1.0, LOGO_POCKET).rotate((0, 0, 0), (1, 0, 0), -math.degrees(slope))
              .translate((0, yface, zc)))
    return b, (yface, zc, slope)


def body_text(yface, zc, slope):
    """Logo vlozene do luzka (v sestave) - vycniva LOGO_T - LOGO_POCKET nad povrch."""
    return (logo_solid(LOGO_W, 0.0, -(LOGO_T - LOGO_POCKET), LOGO_POCKET)
            .rotate((0, 0, 0), (1, 0, 0), -math.degrees(slope)).translate((0, yface, zc)))


def logo_print():
    """Logo k samostatnemu tisku (naplocho)."""
    return logo_solid(LOGO_W, 0.0, 0, LOGO_T).rotate((0, 0, 0), (1, 0, 0), 90)


def bottom_plate():
    t = PLATE_T
    p = cq.Workplane("XY").polyline(octagon(BW0, BD0, BCH)).close().extrude(t).translate((0, 0, -t))
    p = p.edges("<Z").chamfer(1.0)
    bx, by = BOSS_XY
    for sx in (-1, 1):
        for sy in (-1, 1):
            p = p.cut(cyl_z(CLR_M3, -t - 1, 1, sx * bx, sy * by))
            p = p.cut(cyl_z(HEAD_M3, -t - 1, -t + 1.8, sx * bx, sy * by))
            # luzka pro silikonove nozicky D11
            p = p.cut(cyl_z(11.6, -t - 1, -t + 1.0, sx * (bx - 12), sy * (by - 2)))
    # odlehceni - vroubky ve spodni strane pro vazeni (volitelne olovene/ocelove podlozky do tela)
    usb_end = BW0 / 2 - WALL - 2.2
    can_x = 0.0
    return p, can_x, usb_end


# =====================================================================
#  SESTAVA
# =====================================================================
def build():
    parts = {}
    h = head()
    band, dark, yellow = stripe_band()
    b, (yface, zc, slope) = body()
    bp, can_x, usb_end = bottom_plate()
    parts["hlava"] = h
    parts["hlava_pruh_cerny"] = dark
    parts["hlava_pruh_zluty"] = yellow
    parts["prstenec_oka"] = eye_ring()
    parts["drzak_oka_L"] = eye_bracket(-EYE_DX)
    parts["drzak_oka_P"] = eye_bracket(EYE_DX)
    parts["zadni_viko"] = head_back()
    parts["antena"] = antenna()
    parts["krk"] = neck()
    parts["telo"] = b
    try:
        parts["telo_text"] = body_text(yface, zc, slope)
    except Exception as e:  # noqa
        print("text se nepodarilo vytvorit:", e)
    parts["spodni_viko"] = bp

    # umisteni v sestave
    z_neck = BH
    z_head = BH + NFL_T + NECK_H
    yh = -HD / 2  # stred hlavy nad krkem
    place = {
        "spodni_viko": (0, 0, 0),
        "telo": (0, 0, 0),
        "telo_text": (0, 0, 0),
        "krk": (0, 0, z_neck),
        "hlava": (0, yh, z_head),
        "hlava_pruh_cerny": (0, yh, z_head),
        "hlava_pruh_zluty": (0, yh, z_head),
        "drzak_oka_L": (0, yh, z_head),
        "drzak_oka_P": (0, yh, z_head),
        "zadni_viko": None,
        "antena": (0, 0, z_head + HH - 7),
    }
    asm = {}
    for k, v in parts.items():
        if k == "zadni_viko":
            asm[k] = (v.rotate((0, 0, 0), (1, 0, 0), -90).rotate((0, 0, 0), (0, 1, 0), 180)
                      .translate((0, yh + HD, z_head + HH / 2)))
        elif k == "prstenec_oka":
            for i, ex in enumerate((-EYE_DX, EYE_DX)):
                asm[f"prstenec_oka_{i+1}"] = (v.rotate((0, 0, 0), (1, 0, 0), 90)
                                             .translate((ex, yh + 0.8, z_head + EYE_Z)))
        else:
            asm[k] = v.translate(place[k])
    return parts, asm, dict(can_x=can_x, usb_end=usb_end, z_head=z_head, yh=yh)


if __name__ == "__main__":
    import sys
    parts, asm, info = build()
    print(info)
