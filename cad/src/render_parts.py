import robot as R, dummies as D, cadquery as cq, vtk, os
parts, asm, info = R.build()
yh, zh = info["yh"], info["z_head"]
objs = {"hlava": asm["hlava"].translate((-75,0,-60)), "telo": asm["telo"].rotate((0,0,0),(0,0,1),180).translate((85,0,0)),
        "viko": asm["spodni_viko"].translate((85,0,-25))}
bd,pl=D.dualeye(yh,zh); objs["d0"]=bd.translate((-75,0,-60)); objs["d1"]=pl.translate((-75,0,-60))
objs["esp"]=asm["drzak_oka_L"].union(asm["drzak_oka_P"]).translate((-75,0,-60))
col={"hlava":(0.45,0.45,0.43),"telo":(0.45,0.45,0.43),"viko":(0.2,0.2,0.2),"d0":(0.1,0.4,0.2),"d1":(0.15,0.15,0.5),"esp":(0.93,0.45,0.13)}
ren=vtk.vtkRenderer(); ren.SetBackground(0.96,0.96,0.95)
for k,v in objs.items():
    f=f"/tmp/nw3d/p_{k}.stl"; cq.exporters.export(v,f,tolerance=0.05,angularTolerance=0.15)
    rd=vtk.vtkSTLReader(); rd.SetFileName(f); nm=vtk.vtkPolyDataNormals(); nm.SetInputConnection(rd.GetOutputPort())
    m=vtk.vtkPolyDataMapper(); m.SetInputConnection(nm.GetOutputPort()); a=vtk.vtkActor(); a.SetMapper(m); a.GetProperty().SetColor(*col[k]); ren.AddActor(a)
rw=vtk.vtkRenderWindow(); rw.SetOffScreenRendering(1); rw.AddRenderer(ren); rw.SetSize(1600,1000)
cam=ren.GetActiveCamera(); cam.SetFocalPoint(0,0,60); cam.SetPosition(150,650,350); cam.SetViewUp(0,0,1); ren.ResetCamera(); cam.Zoom(1.3)
lk=vtk.vtkLightKit(); lk.AddLightsToRenderer(ren); rw.Render()
w=vtk.vtkWindowToImageFilter(); w.SetInput(rw); w.Update(); p=vtk.vtkPNGWriter(); p.SetFileName("../images/nahled_vnitrek.png"); p.SetInputConnection(w.GetOutputPort()); p.Write()
