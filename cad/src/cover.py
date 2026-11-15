"""Slepé oko: krytka místo prstence oka nad nefunkčním displejem.
Tiskne se zadní stranou dolů, barvy jsou v horních vrstvách (jako 3MF se 3 díly)."""
import math, cadquery as cq
from robot import RING_OD, RING_ID, RING_T, RING_PIN_R

FACE_Z = RING_T - 1.0      # čelo "displeje" je o 1 mm zapuštěné pod prstencem
INLAY = 0.6                # hloubka barevných vložek (3 vrstvy po 0,2 mm)
MM = 32.4 / 240            # 1 px displeje v mm (aktivní plocha 32,4 mm)
IRIS_R, PUPIL_R = 60 * MM, 27 * MM

def disc(r, z0, z1, x=0, y=0):
    return cq.Workplane("XY").circle(r).extrude(z1 - z0).translate((x, y, z0))

def ring():
    r = (cq.Workplane("XY").circle(RING_OD / 2).circle(RING_ID / 2).extrude(RING_T)
         .faces(">Z").edges().chamfer(0.8))
    for a in (45, 135, 225, 315):
        x, y = RING_PIN_R * math.cos(math.radians(a)), RING_PIN_R * math.sin(math.radians(a))
        r = r.union(cq.Workplane("XY").polygon(6, 4.6).extrude(0.9).translate((x, y, RING_T)))
        r = r.cut(cq.Workplane("XY").polygon(6, 2.0).extrude(1).translate((x, y, RING_T + 0.4)))
    return r

def build():
    z1 = FACE_Z - INLAY
    iris = disc(IRIS_R, z1, FACE_Z).cut(disc(PUPIL_R, z1, FACE_Z))
    screen = disc(RING_ID / 2, 0, FACE_Z).cut(iris)      # černá: pozadí + zornice
    return {"prstenec": ring(), "displej_cerny": screen, "duhovka_azur": iris}

if __name__ == "__main__":
    import os
    out = "../stl"
    names = {"prstenec": "08_slepe_oko_prstenec", "displej_cerny": "08a_slepe_oko_cerna",
             "duhovka_azur": "08b_slepe_oko_azur", "odlesk": "08c_slepe_oko_odlesk"}
    for k, v in build().items():
        cq.exporters.export(v, f"{out}/{names[k]}.stl", tolerance=0.01, angularTolerance=0.05)
        print(k, v.val().Volume())
