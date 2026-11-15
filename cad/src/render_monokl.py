import robot as R, monokl as M, cadquery as cq, vtk, math
parts, asm, info = R.build()
yh, zh = info["yh"], info["z_head"]
cv = M.build()
def place(o, ex):  # jako prstenec v sestave
    return o.rotate((0,0,0),(1,0,0),90).translate((ex, yh + 0.8, zh + R.EYE_Z))
col = {"hlava":(0.82,0.83,0.84),"cer":(0.12,0.12,0.12),"zlu":(0.95,0.75,0.08),"ring":(1.0,0.42,0.07),
       "black":(0.08,0.08,0.08),"purple":(0.37,0.26,0.72),"grey":(0.82,0.83,0.84),"cyan":(0.02,0.85,0.9),"hl":(0.82,0.83,0.84),"scr":(0.03,0.05,0.08)}
objs = [("hlava",asm["hlava"]),("cer",asm["hlava_pruh_cerny"]),("zlu",asm["hlava_pruh_zluty"]),
        ("grey",asm["prstenec_oka_2"]),
        ("scr", cq.Workplane("XZ").circle(17).extrude(-0.5).translate((R.EYE_DX, yh+R.DISP_FACE-0.2, zh+R.EYE_Z))),
        ("scr", cq.Workplane("XZ").circle(17).extrude(-0.5).translate((-R.EYE_DX, yh+R.DISP_FACE-0.2, zh+R.EYE_Z))),
        ("purple",place(cv["prstenec_fialovy"],-R.EYE_DX)),("black",place(cv["praskliny"],-R.EYE_DX))]
def scene(objs, pos, focal, fn, zoom, size=(1400,1000)):
    ren=vtk.vtkRenderer(); ren.SetBackground(0.96,0.96,0.95)
    for i,(k,v) in enumerate(objs):
        f=f"/tmp/nw3d/c_{i}.stl"; cq.exporters.export(v,f,tolerance=0.02,angularTolerance=0.08)
        rd=vtk.vtkSTLReader(); rd.SetFileName(f); nm=vtk.vtkPolyDataNormals(); nm.SetInputConnection(rd.GetOutputPort()); nm.SetFeatureAngle(40)
        m=vtk.vtkPolyDataMapper(); m.SetInputConnection(nm.GetOutputPort()); a=vtk.vtkActor(); a.SetMapper(m); a.GetProperty().SetColor(*col[k]); ren.AddActor(a)
    rw=vtk.vtkRenderWindow(); rw.SetOffScreenRendering(1); rw.AddRenderer(ren); rw.SetSize(*size)
    cam=ren.GetActiveCamera(); cam.SetFocalPoint(*focal); cam.SetPosition(*pos); cam.SetViewUp(0,0,1); ren.ResetCamera(); cam.Zoom(zoom)
    lk=vtk.vtkLightKit(); lk.AddLightsToRenderer(ren); rw.Render()
    w=vtk.vtkWindowToImageFilter(); w.SetInput(rw); w.Update(); p=vtk.vtkPNGWriter(); p.SetFileName(fn); p.SetInputConnection(w.GetOutputPort()); p.Write()
scene(objs,(60,-400,zh+120),(0,yh,zh+42),"../images/nahled_monokl_hlava.png",1.5)
part=[("purple",cv["prstenec_fialovy"]),("black",cv["praskliny"])]
scene(part,(0,-25,110),(0,0,0),"../images/nahled_monokl_dil.png",1.2,(1000,1000))
