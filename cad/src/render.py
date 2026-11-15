import os as _os
ROOT=_os.path.normpath(_os.path.join(_os.path.dirname(_os.path.abspath(__file__)), ".."))
import sys, numpy as np, robot as R, dummies as D, cadquery as cq, vtk
parts, asm, info = R.build()
yh, zh = info["yh"], info["z_head"]
import os
def hx(h): h=h.lstrip("#"); return tuple(int(h[i:i+2],16)/255 for i in (0,2,4))
PAL = os.environ.get("PALETTE","")
if PAL.startswith("light"):
    GREY=hx("D1D3D5"); DARK=hx("3A3A3A"); ORANGE=hx("FF6A13"); YEL=hx("F4C21A")
else:
    GREY=(0.42,0.42,0.40); DARK=(0.17,0.17,0.17); ORANGE=(0.93,0.45,0.13); YEL=(0.96,0.75,0.10)
LOGO = hx(os.environ.get("LOGO","0DD8E0"))
col = {"hlava":GREY,"zadni_viko":GREY,"krk":DARK,"telo":GREY,"spodni_viko":DARK,"antena":DARK,
       "prstenec_oka_1":ORANGE,"drzak_oka_L":ORANGE,"drzak_oka_P":ORANGE,"prstenec_oka_2":ORANGE,"hlava_pruh_cerny":DARK,"hlava_pruh_zluty":YEL,"telo_text":LOGO}
objs = dict(asm)
# displeje: modre obrazovky s ocima
for i,ex in enumerate((-R.EYE_DX,R.EYE_DX)):
    objs[f"lcd{i}"] = R.cyl_y(33.0, R.DISP_FACE-0.3, R.DISP_FACE+0.3, ex, R.EYE_Z).translate((0,yh,zh)); col[f"lcd{i}"]=(0.02,0.10,0.22)
    objs[f"pup{i}"] = R.cyl_y(14, R.DISP_FACE-0.6, R.DISP_FACE, ex+2, R.EYE_Z+1).translate((0,yh,zh)); col[f"pup{i}"]=(0.45,0.72,0.95)
    ring = (R.cyl_y(28, R.DISP_FACE-0.5, R.DISP_FACE, ex, R.EYE_Z).cut(R.cyl_y(23, R.DISP_FACE-1, R.DISP_FACE+1, ex, R.EYE_Z))
            .intersect(R.box(ex-20, ex+20, -5, 10, R.EYE_Z-2, R.EYE_Z+20).union(R.box(ex, ex+20, -5, 10, R.EYE_Z-20, R.EYE_Z+20)))).translate((0,yh,zh))
    objs[f"prog{i}"] = ring; col[f"prog{i}"]=(0.36,0.79,0.65)
if len(sys.argv)>1 and sys.argv[1]=="explode":
    off={"hlava":(0,0,70),"hlava_pruh_cerny":(0,-12,70),"hlava_pruh_zluty":(0,-12,70),"zadni_viko":(0,45,70),"antena":(0,0,110),
         "krk":(0,0,35),"spodni_viko":(0,0,-40),"prstenec_oka_1":(0,-25,70),"prstenec_oka_2":(0,-25,70)}
    for i in range(2):
        for p in ("lcd","pup","prog"): off[f"{p}{i}"]=(0,0,70)
    objs={k:v.translate(off.get(k,(0,0,0))) for k,v in objs.items()}
    bd,_=D.dualeye(yh,zh); objs["board"]=bd.translate((0,12,70)); col["board"]=(0.10,0.35,0.15)
    for k in ("drzak_oka_L","drzak_oka_P"): objs[k]=objs[k].translate((0,22,0))
ren=vtk.vtkRenderer(); ren.SetBackground(0.96,0.96,0.95)
for k,v in objs.items():
    import tempfile,os
    f=f"/tmp/nw3d/r_{k}.stl"; os.makedirs("/tmp/nw3d",exist_ok=True)
    cq.exporters.export(v,f,tolerance=0.05,angularTolerance=0.15)
    rd=vtk.vtkSTLReader(); rd.SetFileName(f)
    nm=vtk.vtkPolyDataNormals(); nm.SetInputConnection(rd.GetOutputPort()); nm.SetFeatureAngle(30)
    m=vtk.vtkPolyDataMapper(); m.SetInputConnection(nm.GetOutputPort())
    a=vtk.vtkActor(); a.SetMapper(m); p=a.GetProperty(); p.SetColor(*col.get(k,GREY)); p.SetSpecular(0.25); p.SetSpecularPower(25)
    ren.AddActor(a)
rw=vtk.vtkRenderWindow(); rw.SetOffScreenRendering(1); rw.AddRenderer(ren); rw.SetSize(1400,1400)
cam=ren.GetActiveCamera()
views={"front":(-0.55,-1.4,0.45),"back":(0.9,1.3,0.6),"side":(-1.5,-0.6,0.3)}
name=sys.argv[2] if len(sys.argv)>2 else "front"
d=views[name]; cz=110 if (len(sys.argv)<=1 or sys.argv[1]!="explode") else 110
cam.SetFocalPoint(0,0,cz); cam.SetPosition(d[0]*500,d[1]*500,cz+d[2]*500); cam.SetViewUp(0,0,1)
ren.ResetCamera(); cam.Zoom(1.15)
lk=vtk.vtkLightKit(); lk.AddLightsToRenderer(ren)
rw.Render()
w2i=vtk.vtkWindowToImageFilter(); w2i.SetInput(rw); w2i.Update()
wr=vtk.vtkPNGWriter(); out=ROOT+f"/images/nahled_{sys.argv[1] if len(sys.argv)>1 else 'sestava'}_{name}{os.environ.get('SUFFIX','')}.png"; wr.SetFileName(out); wr.SetInputConnection(w2i.GetOutputPort()); wr.Write(); print(out)
