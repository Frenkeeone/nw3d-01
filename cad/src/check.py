import time, robot
t=time.time()
parts, asm, info = robot.build()
print("build %.1fs"%(time.time()-t), info)
for k,v in asm.items():
    bb=v.val().BoundingBox(); 
    print(f"{k:20s} x[{bb.xmin:7.1f},{bb.xmax:7.1f}] y[{bb.ymin:7.1f},{bb.ymax:7.1f}] z[{bb.zmin:7.1f},{bb.zmax:7.1f}] valid={v.val().isValid()} vol={v.val().Volume():.0f}")
import pickle
