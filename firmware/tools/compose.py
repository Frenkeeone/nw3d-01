import sys, numpy as np, os
from PIL import Image, ImageDraw, ImageFont
def load(f):
    a=np.fromfile(f,dtype='>u2').reshape(240,240)
    r=((a>>11)&31)*255//31; g=((a>>5)&63)*255//63; b=(a&31)*255//31
    return Image.fromarray(np.dstack([r,g,b]).astype('uint8'))
def panel(name, title):
    W,H=640,340; im=Image.new('RGB',(W,H),(209,211,213)); d=ImageDraw.Draw(im)
    for e,cx in enumerate((175,465)):
        d.ellipse((cx-150,30,cx+150,330),fill=(255,106,19))   # oranzovy prstenec
        d.ellipse((cx-124,56,cx+124,304),fill=(20,20,20))
        eye=load(f"{name}_{e}.raw"); m=Image.new('L',(240,240),0); ImageDraw.Draw(m).ellipse((0,0,239,239),fill=255)
        im.paste(eye,(cx-120,60),m)
    f=ImageFont.truetype(os.path.join(os.path.dirname(__file__),'ChakraPetch-SemiBold.ttf'),22)
    d.text((W//2,4),title,fill=(40,40,40),font=f,anchor='ma')
    return im
items=[("print","Tisk (67 %)"),("pause","Pauza"),("prep","Příprava"),("eyes","Oči – nečinný"),("blink","Mrknutí"),("clock","Hodiny"),
       ("weather","Počasí"),("weather_rain","Déšť"),("weather_storm","Bouřka"),("weather_snow","Sníh"),("finished","Hotovo"),("failed","Chyba"),("boot","Test při startu"),("msg","Hlášení")]
ims=[panel(n,t) for n,t in items]
cols=2; rows=(len(ims)+1)//2
sheet=Image.new('RGB',(640*cols,340*rows),(240,240,240))
for i,im in enumerate(ims): sheet.paste(im,((i%cols)*640,(i//cols)*340))
sheet.save(sys.argv[1]); print(sheet.size)
