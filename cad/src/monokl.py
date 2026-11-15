"""Rozbité okruží ("monokl"): prstenec oka s prasklinami, ulomeným kouskem a vypadlým šroubkem.
Tiskne se zadní stranou dolů. Praskliny jsou vložka v horních 0,6 mm (jiná barva)."""
import math, cadquery as cq
from robot import RING_OD, RING_ID, RING_T, RING_PIN_R

CRACK_D = 0.6          # hloubka praskliny (3 vrstvy)
RO, RI = RING_OD / 2, RING_ID / 2

def P(r, a):  # polarni -> xy, uhel ve stupnich od +x proti smeru hodin, +y = nahoru
    return (r * math.cos(math.radians(a)), r * math.sin(math.radians(a)))

# praskliny: (polomer, uhel, sirka)
CRACKS = [
    [(RO - 1.0, 152, 1.3), (21.6, 146, 1.1), (20.4, 149, 0.95), (19.0, 140, 0.8), (17.6, 143, 0.7), (16.5, 136, 0.6)],
    [(20.4, 149, 0.8), (22.6, 141, 0.7), (23.0, 130, 0.6)],
    [(RO - 1.0, 168, 1.2), (21.2, 176, 1.0), (19.6, 172, 0.85), (18.2, 183, 0.7), (16.5, 186, 0.6)],
    [(19.6, 172, 0.7), (20.6, 192, 0.6)],
    [(16.5, -18, 1.0), (18.6, -24, 0.85), (20.1, -20, 0.7), (21.9, -28, 0.6)],
    [(16.5, 74, 0.8), (18.4, 80, 0.65), (19.7, 76, 0.6)],
]
# ulomeny kousek na vnejsim okraji (r, uhel)
CHIP = [(RO + 2, 150), (21.9, 152), (21.0, 156), (21.8, 160), (20.7, 164), (21.5, 168), (22.3, 171), (RO + 2, 172)]
MISSING_BOLT = 135     # tady sroubek "vypadl"

def seg(p0, p1, w0, w1, z0, z1):
    (x0, y0), (x1, y1) = p0, p1
    dx, dy = x1 - x0, y1 - y0
    L = math.hypot(dx, dy); nx, ny = -dy / L, dx / L
    pts = [(x0 + nx * w0 / 2, y0 + ny * w0 / 2), (x1 + nx * w1 / 2, y1 + ny * w1 / 2),
           (x1 - nx * w1 / 2, y1 - ny * w1 / 2), (x0 - nx * w0 / 2, y0 - ny * w0 / 2)]
    s = cq.Workplane("XY").polyline(pts).close().extrude(z1 - z0).translate((0, 0, z0))
    return s.union(cq.Workplane("XY").circle(w1 / 2).extrude(z1 - z0).translate((x1, y1, z0)))

def crack_prism(z0, z1):
    out = None
    for c in CRACKS:
        for (r0, a0, w0), (r1, a1, w1) in zip(c, c[1:]):
            s = seg(P(r0, a0), P(r1, a1), w0, w1, z0, z1)
            out = s if out is None else out.union(s)
    # prodlouzeni pres vnitrni hranu, at praskliny dobihaji az na okraj
    return out

def build():
    r = (cq.Workplane("XY").circle(RO).circle(RI).extrude(RING_T).faces(">Z").edges().chamfer(0.8))
    for a in (45, 135, 225, 315):
        x, y = P(RING_PIN_R, a)
        if a == MISSING_BOLT:   # jen prazdne sestihranne luzko
            r = r.cut(cq.Workplane("XY").polygon(6, 4.8).extrude(1).translate((x, y, RING_T - 0.6)))
            continue
        r = r.union(cq.Workplane("XY").polygon(6, 4.6).extrude(0.9).translate((x, y, RING_T)))
        r = r.cut(cq.Workplane("XY").polygon(6, 2.0).extrude(1).translate((x, y, RING_T + 0.4)))
    chip = cq.Workplane("XY").polyline([P(rr, a) for rr, a in CHIP]).close().extrude(RING_T + 2).translate((0, 0, -1))
    # ulomek: zkoseny lom, ne kolmy rez - dole o kousek mensi (bez previsu, lom do strany)
    r = r.cut(chip)
    prism = crack_prism(RING_T - CRACK_D, RING_T + 1)
    cracks = r.intersect(prism)
    ring = r.cut(prism)
    return {"prstenec_fialovy": ring, "praskliny": cracks}

if __name__ == "__main__":
    names = {"prstenec_fialovy": "09_monokl_prstenec", "praskliny": "09a_monokl_praskliny"}
    for k, v in build().items():
        cq.exporters.export(v, f"../stl/{names[k]}.stl", tolerance=0.01, angularTolerance=0.05)
        print(k, round(v.val().Volume(), 1))
