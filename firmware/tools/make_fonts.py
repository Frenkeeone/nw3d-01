"""Generuje vyhlazena pisma ve formatu VLW (LovyanGFX / TFT_eSPI) jako C pole.
Pismo: Chakra Petch (SIL Open Font License 1.1, viz OFL.txt)."""
import struct, os
from PIL import ImageFont, Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "NW3D_Eyes", "fonts_vlw.h")

ASCII = "".join(chr(c) for c in range(0x20, 0x7F))
CZ = "ÁČĎÉĚÍŇÓŘŠŤÚŮÝŽáčďéěíňóřšťúůýž°"
FULL = ASCII + CZ
DIGITS = " 0123456789:%-/.,+°"

FONTS = [  # (nazev, soubor, velikost px, znaky)
    ("font_small", "ChakraPetch-Medium.ttf",   20, FULL),
    ("font_med",   "ChakraPetch-SemiBold.ttf", 28, FULL),
    ("font_big",   "ChakraPetch-SemiBold.ttf", 62, DIGITS),
    ("font_huge",  "ChakraPetch-Bold.ttf",     96, DIGITS),
]

def vlw(ttf, size, chars):
    f = ImageFont.truetype(os.path.join(HERE, ttf), size)
    ascent, descent = f.getmetrics()
    glyphs, bitmaps = [], []
    for ch in sorted(set(chars), key=ord):
        adv = int(round(f.getlength(ch)))
        if ch == " ":
            glyphs.append((ord(ch), 0, 0, adv, 0, 0)); bitmaps.append(b""); continue
        x0, y0, x1, y1 = f.getbbox(ch, anchor="ls")
        w, h = max(0, x1 - x0), max(0, y1 - y0)
        im = Image.new("L", (max(w, 1), max(h, 1)), 0)
        ImageDraw.Draw(im).text((-x0, -y0), ch, font=f, fill=255, anchor="ls")
        glyphs.append((ord(ch), h, w, adv, -y0, x0))
        bitmaps.append(im.tobytes()[: w * h])
    data = struct.pack(">6i", len(glyphs), 11, size, 0, ascent, descent)
    for u, h, w, adv, dy, dx in glyphs:
        data += struct.pack(">7i", u, h, w, adv, dy, dx, 0)
    for b in bitmaps:
        data += b
    return data

with open(OUT, "w", encoding="utf-8") as o:
    o.write("// Vygenerovano tools/make_fonts.py - pismo Chakra Petch (SIL OFL 1.1)\n#pragma once\n#include <stdint.h>\n\n")
    for name, ttf, size, chars in FONTS:
        d = vlw(ttf, size, chars)
        o.write(f"// {ttf} {size}px, {len(set(chars))} znaku, {len(d)} B\n")
        o.write(f"static const uint8_t {name}[] = {{\n")
        for i in range(0, len(d), 24):
            o.write("  " + ",".join(str(b) for b in d[i:i+24]) + ",\n")
        o.write("};\n\n")
        print(name, len(d), "B")
