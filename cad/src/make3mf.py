"""3MF hlavy s pruhy: jeden objekt, tri dily, prirazene filamenty pro Bambu Studio."""
import zipfile, trimesh, numpy as np
S = __import__("os").path.normpath(__import__("os").path.join(__import__("os").path.dirname(__import__("os").path.abspath(__file__)), "..")) + "/stl/"
parts = [("hlava", S+"01_hlava.stl", 1, "#D1D3D5"),
         ("pruh_zluty", S+"01a_hlava_pruh_zluty.stl", 2, "#F2C014"),
         ("pruh_cerny", S+"01b_hlava_pruh_cerny.stl", 3, "#1E1E1E")]
NS = 'xmlns="http://schemas.microsoft.com/3dmanufacturing/core/2015/02" xmlns:m="http://schemas.microsoft.com/3dmanufacturing/material/2015/02"'
objs, comps = [], []
mats = "".join(f'<m:base name="{n}" displaycolor="{c}FF"/>' for n,_,_,c in parts)
for i,(name, f, ext, col) in enumerate(parts, start=1):
    m = trimesh.load(f); m.merge_vertices()
    v = "".join(f'<vertex x="{a:.4f}" y="{b:.4f}" z="{c:.4f}"/>' for a,b,c in m.vertices)
    t = "".join(f'<triangle v1="{a}" v2="{b}" v3="{c}"/>' for a,b,c in m.faces)
    objs.append(f'<object id="{i}" name="{name}" type="model" pid="100" pindex="{i-1}"><mesh><vertices>{v}</vertices><triangles>{t}</triangles></mesh></object>')
    comps.append(f'<component objectid="{i}"/>')
asm_id = len(parts)+1
# posun na stred podlozky Bambu (256x256) - stred X/Y
allm = trimesh.util.concatenate([trimesh.load(f) for _,f,_,_ in parts])
cx, cy = allm.bounds.mean(axis=0)[:2]
tx, ty = 128 - cx, 128 - cy
model = (f'<?xml version="1.0" encoding="UTF-8"?><model unit="millimeter" xml:lang="en-US" {NS}>'
         f'<metadata name="Application">NW3D-01 generator</metadata>'
         f'<resources><m:basematerials id="100">{mats}</m:basematerials>{"".join(objs)}'
         f'<object id="{asm_id}" name="01_hlava_s_pruhem" type="model"><components>{"".join(comps)}</components></object></resources>'
         f'<build><item objectid="{asm_id}" transform="1 0 0 0 1 0 0 0 1 {tx:.3f} {ty:.3f} 0" printable="1"/></build></model>')
cfg = ['<?xml version="1.0" encoding="UTF-8"?>\n<config>', f'  <object id="{asm_id}">',
       '    <metadata key="name" value="01_hlava_s_pruhem"/>', '    <metadata key="extruder" value="1"/>']
for i,(name,_,ext,_) in enumerate(parts, start=1):
    cfg += [f'    <part id="{i}" subtype="normal_part">', f'      <metadata key="name" value="{name}"/>',
            f'      <metadata key="extruder" value="{ext}"/>', '    </part>']
cfg += ['  </object>', '</config>']
ct = ('<?xml version="1.0" encoding="UTF-8"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="model" ContentType="application/vnd.ms-package.3dmanufacturing-3dmodel+xml"/>'
      '<Default Extension="config" ContentType="text/xml"/></Types>')
rels = ('<?xml version="1.0" encoding="UTF-8"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Target="/3D/3dmodel.model" Id="rel0" Type="http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel"/></Relationships>')
out = __import__("os").path.normpath(__import__("os").path.join(__import__("os").path.dirname(__import__("os").path.abspath(__file__)), "..")) + "/3mf/01_hlava_s_pruhem.3mf"
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
    z.writestr("[Content_Types].xml", ct); z.writestr("_rels/.rels", rels)
    z.writestr("3D/3dmodel.model", model); z.writestr("Metadata/model_settings.config", "\n".join(cfg))
print(out)
# kontrola: nacteni zpet
sc = trimesh.load(out)
print("nacteno:", type(sc).__name__, [ (k, len(g.faces)) for k,g in getattr(sc,'geometry',{}).items()])
