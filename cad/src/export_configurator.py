"""Export vsech variant pro webovy konfigurator: binarni STL v tiskove poloze + matice do sestavy.
Spusteni:  python3 export_configurator.py   (vystup: ../../site/konfigurator/parts/)"""
import os, sys, json, math, tempfile
from multiprocessing import Pool
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.normpath(os.path.join(HERE, "..", "..", "site", "konfigurator", "parts"))
sys.path.insert(0, HERE)


def rx(deg):
    a = math.radians(deg); c, s = math.cos(a), math.sin(a)
    return np.array([[1, 0, 0, 0], [0, c, -s, 0], [0, s, c, 0], [0, 0, 0, 1]])


def jobs():
    import variants as V
    J = []   # (id, builder, args, print_rotation_matrix_name)
    for st in V.HEAD_STYLES:
        for sz in V.HEAD_SIZES:
            suf = "" if sz == "M" else "_" + sz
            k = V.head_k(st, sz)
            for m in V.MOUTHS:
                if V.mouth_fits(st, m, sz):
                    J.append((f"head_{st}_{m}{suf}", "head", (st, m, k), "head"))
            J.append((f"back_{st}{suf}", "back", (st, k), "head"))
    for m in V.MOUTHS:
        for i, (role, _) in enumerate(V.mouth_shapes(m)):
            J.append((f"mouth_{m}_{i}", "mouth", (m, i), "head"))
    import bodies as B
    for st in ["hranate"] + B.STYLES:
        for sz in V.BODY_SIZES:
            if (st, sz) in V.BODY_SIZE_NA:
                continue
            suf = "" if sz == "M" else "_" + sz
            k = V.body_k(st, sz)
            J.append((f"body_{st}{suf}", "body", (st, k), "body"))
            J.append((f"bottom_{st}{suf}", "bottom", (st, k), "none"))
    J.append(("neck", "neck", (), "none"))
    J.append(("bracket_L", "bracket", (-1,), "bracket"))
    J.append(("bracket_R", "bracket", (1,), "bracket"))
    for k in V.EYES:
        n = len(V.eye_parts(k))
        for side in ("L", "R"):
            for i in range(n):
                J.append((f"eye_{k}_{side}_{i}", "eye", (k, side, i), "eye"))
    for k in V.ANTENNAS:
        J.append((f"antenna_{k}", "antenna", (k,), "antenna"))
    for k in V.EMBLEMS:
        for i in range(2 if k != "prazdny" else 1):
            J.append((f"badge_{k}_{i}", "badge", (k, i), "badge"))
    return J


def build(job):
    pid, kind, args, rot = job
    import cadquery as cq, trimesh
    import robot as R, variants as V
    flip = False
    if kind == "head":
        s = V.head(*args).translate((0, V.Y_HEAD, V.Z_HEAD))
    elif kind == "back":
        s = V.head_back(*args)
    elif kind == "mouth":
        m, i = args
        s = V.mouth_parts(m)[i][1].translate((0, V.Y_HEAD, V.Z_HEAD))
    elif kind == "body":
        import bodies as B
        s = B.any_body(*args)
    elif kind == "bottom":
        import bodies as B
        s = B.any_bottom(*args)
    elif kind == "neck":
        s = R.neck().translate((0, 0, R.BH))
    elif kind == "bracket":
        s = R.eye_bracket(args[0] * R.EYE_DX).translate((0, V.Y_HEAD, V.Z_HEAD))
    elif kind == "eye":
        k, side, i = args
        flat = V.eye_parts(k)[i][1]
        ex = -R.EYE_DX if side == "L" else R.EYE_DX
        s = V.eye_place(flat, ex, mirror=(k == "monokl" and side == "R"))
    elif kind == "antenna":
        s, flip = V.antenna(*args)
    elif kind == "badge":
        k, i = args
        s = V.badge_parts(k)[i][1]
    # tiskova poloha (otoceni sestavy)
    if rot == "head":
        Rm = rx(90)
    elif rot == "body":
        Rm = rx(180)
    elif rot == "bracket":
        Rm = rx(-90)
    elif rot == "eye":
        Rm = rx(-90)
    elif rot == "antenna":
        Rm = rx(180) if flip else np.eye(4)
    elif rot == "badge":
        Rm = rx(-90) @ rx(math.degrees(V.SLOPE))
    else:
        Rm = np.eye(4)
    tmp = tempfile.mktemp(suffix=".stl")
    cq.exporters.export(s, tmp, tolerance=0.02, angularTolerance=0.12)
    m = trimesh.load(tmp)
    os.remove(tmp)
    m.apply_transform(Rm)
    m.export(os.path.join(OUT, pid + ".stl"), file_type="stl")
    Minv = np.linalg.inv(Rm)
    return pid, [round(float(v), 6) for v in Minv.T.flatten()], len(m.faces)   # sloupcove (three.js)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    J = jobs()
    only = sys.argv[1:]
    if only == ["--sizes"]:   # jen zmensene / zvetsene varianty (M se nemeni)
        J = [j for j in J if j[0].endswith(("_S", "_L"))]
    elif only:
        J = [j for j in J if any(j[0].startswith(o) for o in only)]
    print(len(J), "dilu")
    n = int(os.environ.get("JOBS", "1"))
    if n > 1:
        import multiprocessing as mp
        with mp.get_context("spawn").Pool(n) as pool:   # OCC nesnese fork
            res = pool.map(build, J, chunksize=1)
    else:
        res = []
        for j in J:
            res.append(build(j))
            print(j[0], flush=True)
    idx_path = os.path.join(OUT, "matrices.json")
    mats = json.load(open(idx_path)) if os.path.exists(idx_path) else {}
    for pid, M, nf in res:
        mats[pid] = M
        print(f"{pid:32s} {nf:7d} trojuhelniku")
    json.dump(mats, open(idx_path, "w"), indent=0)
    import variants as V, bodies as B, trimesh
    mat = lambda M: [round(float(v), 6) for v in M.T.flatten()]
    BODY_ST = ["hranate"] + B.STYLES
    meta = {"HH": V.HH, "BH": V.BH,
            "headTop": {st: round(V.head_top(st), 2) for st in V.HEAD_STYLES},
            "headTopSz": {st: {sz: round(V.head_top(st, V.head_k(st, sz)), 2) for sz in V.HEAD_SIZES} for st in V.HEAD_STYLES},
            "bodyH": {st: {sz: round(B.body_height(st, V.body_k(st, sz)), 2) for sz in V.BODY_SIZES} for st in BODY_ST},
            "bodyNA": [f"{st}_{sz}" for st, sz in sorted(V.BODY_SIZE_NA)],
            "mouthNA": [f"{st}_{m}_{sz}" for st in V.HEAD_STYLES for m in V.MOUTHS for sz in V.HEAD_SIZES
                        if not V.mouth_fits(st, m, sz)],
            "badgeMount": {st: mat(B.badge_mount(st)) for st in B.STYLES},
            "badgeMountSz": {f"{st}_{sz}": mat(B.badge_mount(st, V.body_k(st, sz)))
                             for st in BODY_ST for sz in V.BODY_SIZES if (st, sz) not in V.BODY_SIZE_NA},
            "support": {f"{st}_{sz}": [[round(x, 1), round(y, 1)] for x, y in B.support(st, V.body_k(st, sz))]
                        for st in BODY_ST for sz in V.BODY_SIZES if (st, sz) not in V.BODY_SIZE_NA},
            "badge": {"w": V.BADGE_W, "h": V.BADGE_H, "r": V.BADGE_R, "t": V.BADGE_T, "sym": V.SYM_D}}
    # objem a teziste kazdeho dilu v sestave (pro odhad stability)
    mass = {}
    for pid, M in mats.items():
        f = os.path.join(OUT, pid + ".stl")
        if not os.path.exists(f):
            continue
        m = trimesh.load(f)
        m.apply_transform(np.array(M).reshape(4, 4).T)
        c = m.center_mass if m.is_volume else m.centroid
        mass[pid] = [round(float(abs(m.volume)), 0)] + [round(float(v), 1) for v in c]
    meta["mass"] = mass
    json.dump(meta, open(os.path.join(OUT, "meta.json"), "w"), indent=0)
