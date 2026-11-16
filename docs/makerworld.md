# MakerWorld – podklady pro zveřejnění

Pracovní verze popisu. Fotky a fotky do montážního návodu doplní Frantisek.

## Před zveřejněním – kontrola
- [ ] Fotky: jen **skutečné fotky** vytištěného robota (pravidla MakerWorld nepovolují AI obrázky jako obrázky modelu). Rendery z `cad/images` lze dát nanejvýš do popisu.
- [ ] Sekce **AI-Generated Content**: model i firmware vznikly s pomocí AI (Claude) → zvážit označení podle [AI Policy](https://wiki.bambulab.com/en/makerworld/ai-policy).
- [ ] Licence na MakerWorld: odpovídající CC BY-NC-SA 4.0 (v repozitáři `cad/LICENSE`).
- [ ] Ověřit, zda pravidla dovolují odkaz na GitHub (instalace firmwaru). Kdyby ne: odkaz jen v profilu / v komentáři.
- [ ] Znak na hrudi: štítky se znaky a vlastní logo jsou v konfigurátoru (odkázat na něj).
- [ ] Tiskové profily (3MF) pro Bambu Studio: hlava s pruhem, tělo, drobné díly (antény, prstence, držáky, štítek).
- [ ] Odkaz na instalaci: https://frenkeeone.github.io/nw3d-01/

## Název
**NW3D-01 – desk robot with display eyes that shows your Bambu print progress**

(CZ: NW3D-01 – stolní robot, který očima ukazuje průběh tisku)

## Krátký popis (summary)
EN: A friendly desk robot with two round display eyes. It watches your Bambu Lab printer and shows progress, time left and a picture of the model being printed – plus clock, weather and tickers when idle. One ready-made ESP32-S3 board, no soldering, firmware installs from your browser.

CZ: Přátelský stolní robot se dvěma kulatými displeji místo očí. Hlídá tiskárnu Bambu Lab a ukazuje průběh, zbývající čas a obrázek tištěného modelu – a když se netiskne, hodiny, počasí a kurzy. Jedna hotová deska ESP32-S3, žádné pájení, firmware se nahraje z prohlížeče.

## Popis (EN)
**NW3D-01** sits on your desk and keeps an eye on your printer – literally.

**What the eyes show**
- 🟢 Print progress ring around the eyes, time left, finish time, layer and temperatures
- 🖼️ A preview picture of the model being printed (Bambu cloud)
- 🎉 "DONE" with a happy eye and a short tune when the print finishes, an alert when it fails
- 👀 Animated eyes, clock, date, weather and up to 4 tickers (Brent crude, gold, Bitcoin, stocks…) while idle
- 🌙 Night mode with dimming
- You choose what each eye shows – separately for printing and idle – on the robot's own web page

**Easy setup – no Arduino, no coding**
1. Open the installer page (link below) in Chrome or Edge, plug in the board, click *Install*.
2. Enter your Wi-Fi.
3. Open `http://nw3d-robot.local` – sign in to your Bambu account, pick your printer from the list, type your city. Done.

Works with Bambu Lab printers through the Bambu cloud (printer stays in normal mode) or in LAN mode. Czech and English. Updates install over Wi-Fi.

**Electronics:** Waveshare **ESP32-S3-DualEye-LCD-1.28** (two 1.28" round displays, speaker on board). Nothing else – no soldering, no wiring.

**Printing**
- All parts print **without supports**.
- PLA is fine (prototype: Bambu PLA Basic Light Gray), about 250 g in the main colour.
- The head comes as a multi-colour 3MF with the yellow/black warning stripe – just assign filaments.
- The chest badge is a separate part you glue in – pick an emblem or upload your own logo in the configurator.
- Extras: a blind eye cap and a cracked "black eye" ring, if one of your displays ever breaks 😉

**Hardware (BOM)**
| Item | Qty |
|---|---|
| Waveshare ESP32-S3-DualEye-LCD-1.28 | 1 |
| M3 × 8 hex socket screw | 16 |
| M3 heat-set insert, OD 4.5 mm, L 4 mm | 16 |
| M2 × 6 screw (into the board's brass standoffs) | 4 |
| Silicone feet Ø 11 mm | 4 |
| Foam double-sided tape (speaker) | a piece |
| Super glue (eye rings, chest badge) | |
| USB-C data cable + 5 V USB charger | 1 |

**Before printing the whole head**, print the small test piece `00_test_oko` and one eye bracket to check that the display fits.

**Links**
- Firmware installer: https://frenkeeone.github.io/nw3d-01/
- Source code, STEP files and full guide: https://github.com/Frenkeeone/nw3d-01

**Privacy:** your Wi-Fi, settings and Bambu access token stay on the robot. Your password is used once to sign in and is never stored.

Unofficial project, not affiliated with Bambu Lab or Waveshare.

## Popis (CZ)
**NW3D-01** stojí na stole a doslova hlídá tvoji tiskárnu.

**Co ukazují oči**
- 🟢 Prstenec průběhu tisku kolem očí, zbývající čas, kdy bude hotovo, vrstvu a teploty
- 🖼️ Obrázek modelu, který se právě tiskne (přes Bambu cloud)
- 🎉 Po dotisku „HOTOVO“ s usměvavým okem a krátkou melodií, při chybě upozornění
- 👀 Bez tisku animované oči, hodiny, datum, počasí a až 4 kurzy (ropa Brent, zlato, bitcoin, akcie…)
- 🌙 Noční režim se ztlumením
- Co ukazuje které oko, si nastavíš na webu robota – zvlášť pro tisk a bez tisku

**Snadné nastavení – bez Arduina a programování**
1. Otevři instalační stránku (odkaz níže) v Chrome nebo Edge, připoj desku, klikni na *Nainstalovat*.
2. Zadej Wi-Fi.
3. Otevři `http://nw3d-robot.local` – přihlas se k Bambu účtu, vyber tiskárnu ze seznamu a napiš město. Hotovo.

Funguje s tiskárnami Bambu Lab přes Bambu cloud (tiskárna zůstává v normálním režimu) nebo v LAN režimu. Česky a anglicky. Aktualizace se instalují přes Wi-Fi.

**Elektronika:** Waveshare **ESP32-S3-DualEye-LCD-1.28** (dva kulaté displeje 1,28", reproduktor na desce). Nic dalšího – žádné pájení ani drátky.

**Tisk**
- Všechny díly se tisknou **bez podpěr**.
- Stačí PLA (prototyp: Bambu PLA Basic Light Gray), asi 250 g hlavní barvy.
- Hlava je jako vícebarevný 3MF se žluto-černým pruhem – jen přiřadíš filamenty.
- Štítek na hrudi je samostatný díl, který se vlepí – znak nebo vlastní logo vybereš v konfigurátoru.
- Navíc: krytka slepého oka a rozbité okruží „monokl“ pro případ, že se displej rozbije 😉

**Materiál**
| Položka | Ks |
|---|---|
| Waveshare ESP32-S3-DualEye-LCD-1.28 | 1 |
| šroub M3 × 8 imbus | 16 |
| zalisovací matice M3, OD 4,5 mm, délka 4 mm | 16 |
| šroub M2 × 6 (do mosazných sloupků desky) | 4 |
| silikonové nožičky Ø 11 mm | 4 |
| oboustranná pěnová páska (reproduktor) | kousek |
| vteřinové lepidlo (prstence očí, štítek) | |
| datový kabel USB-C + USB nabíječka 5 V | 1 |

**Než vytiskneš celou hlavu**, vytiskni malý zkušební díl `00_test_oko` a jeden držák oka a zkontroluj, že displej sedí.

**Odkazy**
- Instalace firmwaru: https://frenkeeone.github.io/nw3d-01/
- Zdrojový kód, STEP a celý návod: https://github.com/Frenkeeone/nw3d-01

**Soukromí:** Wi-Fi, nastavení a přístupový token zůstávají jen v robotovi. Heslo se použije jednou pro přihlášení a neukládá se.

Neoficiální projekt, není spojený s Bambu Lab ani Waveshare.

## Montážní návod – osnova fotek (doplní Frantisek)
1. Vytištěné díly a spojovací materiál
2. Zatavení vložek M3 (hlava – sloupky a rohy, tělo, krk)
3. Nahrání firmwaru (instalační stránka) – ještě před montáží
4. Vložení desky do hlavy, rámečky displejů do lůžek
5. Držáky očí: 2× M2 do desky, 2× M3 do sloupků
6. Krk na tělo, protažení kabelu
7. Hlava na krk (2× M3 zespodu hlavy)
8. Reproduktor na zadní víko, víko 4× M3
9. Spodní víko, nožičky
10. Prstence očí, štítek, anténa – hotovo
