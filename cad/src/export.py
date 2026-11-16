import os, robot as R, cadquery as cq
out = __import__("os").path.normpath(__import__("os").path.join(__import__("os").path.dirname(__import__("os").path.abspath(__file__)), "..")); os.makedirs(out+"/stl", exist_ok=True); os.makedirs(out+"/step", exist_ok=True)
parts, asm, info = R.build()
def pr(k, v):
    if k in ("hlava","hlava_pruh_cerny","hlava_pruh_zluty"):
        return v.rotate((0,0,0),(1,0,0),90)
    if k.startswith("drzak_oka"):
        r = v.rotate((0,0,0),(1,0,0),-90); bb = r.val().BoundingBox()
        return r.translate((-bb.center.x, -bb.center.y, -bb.zmin))
    if k == "antena":   # tiskne se hlavici dolu
        r = v.rotate((0,0,0),(1,0,0),180); return r.translate((0,0,-r.val().BoundingBox().zmin))
    if k == "telo_text":
        return R.badge_print()
    if k == "telo":
        return v.rotate((0,0,0),(1,0,0),180).translate((0,0,R.BH))
    if k=="spodni_viko":
        return v.translate((0,0,R.PLATE_T))
    return v
names = {"hlava":"01_hlava","hlava_pruh_zluty":"01a_hlava_pruh_zluty","hlava_pruh_cerny":"01b_hlava_pruh_cerny",
 "prstenec_oka":"02_prstenec_oka_2ks","drzak_oka_L":"02a_drzak_oka_levy","drzak_oka_P":"02b_drzak_oka_pravy","zadni_viko":"03_zadni_viko","antena":"04_antena","krk":"05_krk",
 "telo":"06_telo","telo_text":"06a_stitek","spodni_viko":"07_spodni_viko"}
for k,v in parts.items():
    p = pr(k,v)
    bb=p.val().BoundingBox(); print(k, "zmin", round(bb.zmin,2), "size", round(bb.xlen,1), round(bb.ylen,1), round(bb.zlen,1))
    cq.exporters.export(p, f"{out}/stl/{names[k]}.stl", tolerance=0.02, angularTolerance=0.1)
a = cq.Assembly(name="NW3D-01")
for k,v in asm.items(): a.add(v, name=k)
a.save(f"{out}/step/NW3D-01_sestava.step")
for k,v in parts.items():
    cq.exporters.export(v, f"{out}/step/step_{names[k]}.step")
import pickle
pickle.dump(info, open(out+"/info.pkl","wb"))
# zkusebni dil: cast celni steny s jednim okem
t = R.head().intersect(R.box(4, 58, -1, R.DISP_FACE+R.BRASS_TOP+R.PAD_H, R.EYE_Z-24, R.EYE_Z+24))
t = t.rotate((0,0,0),(1,0,0),90)
cq.exporters.export(t, f"{out}/stl/00_test_oko.stl", tolerance=0.02, angularTolerance=0.1)
