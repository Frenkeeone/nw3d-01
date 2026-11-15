import cadquery as cq, robot as R
def display(ex, yh, zh):
    # DPS + LCD + kolikova lista (zezadu)
    x0 = ex - R.AA_CX
    z0 = R.EYE_Z - R.AA_CY
    pcb = R.box(x0, x0+R.PCB_W, R.FRONT+R.LCD_T, R.FRONT+R.LCD_T+R.PCB_T, z0, z0+R.PCB_H)
    lcd = R.cyl_y(35.6, R.FRONT+0.01, R.FRONT+R.LCD_T, ex, R.EYE_Z)
    hdr = R.box(x0+R.PCB_W-3.2, x0+R.PCB_W-0.6, R.FRONT+R.LCD_T+R.PCB_T, R.FRONT+R.LCD_T+R.PCB_T+2.5+14, z0+9.1-1.3, z0+9.1+7*2.54+1.3)
    for px,pz in R.HOLES:
        wx,wz = R.pcb_to_world(ex,px,pz); pcb = pcb.cut(R.cyl_y(2.0,0,20,wx,wz))
    return pcb.union(lcd).union(hdr).translate((0,yh,zh))
def esp(can_x, usb_end):
    z = R.PLAT_H + 1.0 + 3.1
    pcb = R.box(usb_end-R.ESP_L, usb_end, -R.ESP_W/2, R.ESP_W/2, z, z+1.6)
    can = R.box(can_x-9, can_x+9, -9, 9, R.PLAT_H+1.0, z)
    usb = R.box(usb_end-7.5, usb_end+0.5, -11, 11, z-3.3, z)
    pins = R.box(usb_end-R.ESP_L+2, usb_end-4, -R.ESP_W/2+0.2, -R.ESP_W/2+2.8, z+1.6, z+1.6+2.5+14)
    pins = pins.union(pins.mirror("XZ"))
    return pcb.union(can).union(usb).union(pins)


def dualeye(yh, zh, pcb_t=None, comp=None):
    """Zjednodusena deska DualEye: sklo (2.2) + DPS + vystupek pripojeni + mosazne zavity + soucastky + USB-C."""
    f = R.DISP_FACE
    g = R.GLASS_T
    pcb = R.PCB_T_DE
    comp = comp if comp is not None else R.BRASS_TOP - g - pcb
    m = None
    for ex in (-R.EYE_DX, R.EYE_DX):
        d = R.cyl_y(R.DISP_D, f, f + g + pcb, ex, R.EYE_Z)
        sgn = -1 if ex > 0 else 1
        xa = ex + sgn * (R.DISP_D / 2 - 1); xb = ex + sgn * (R.DISP_D / 2 + R.TAB_OUT)
        d = d.union(R.box(min(xa, xb), max(xa, xb), f, f + g, R.EYE_Z - 7, R.EYE_Z + 7))
        d = d.union(R.cyl_y(30, f + g + pcb, f + g + pcb + comp, ex, R.EYE_Z))
        for u, v in R.DE_HOLES:
            wx, wz = R.de_world(ex, u, v)
            d = d.union(R.cyl_y(3.5, f + g + pcb, f + R.BRASS_TOP, wx, wz))
        m = d if m is None else m.union(d)
    m = m.union(R.box(-10.5, 10.5, f + g, f + g + pcb, R.EYE_Z - 7, R.EYE_Z + 7))
    plug = R.box(-6, 6, f + 0.1, f + 6.9, R.EYE_Z - 5 - 28, R.EYE_Z - 5)
    return m.translate((0, yh, zh)), plug.translate((0, yh, zh))
