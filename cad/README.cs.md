# NW3D-01 – modely a montáž (česky)

English: [README.md](README.md). Soubory: `stl/`, `3mf/` (vícebarevné díly), `step/` (pro úpravy v CADu), `src/` (zdrojové skripty CadQuery), `images/` (náhledy).

Celková výška 165 mm (s nožičkami asi 168 mm), hlava 124 × 84 × 64 mm. Elektronika je jedna deska Waveshare DualEye (ESP32-S3 + 2 kulaté displeje 1,28"), uložená v hlavě. Desku **nerozdělovat**.

## Varianty a konfigurátor
**[Konfigurátor](https://frenkeeone.github.io/nw3d-01/konfigurator/)** kombinuje 4 tvary hlavy, 5 tvarů těla, 5 typů pusy, 6 stylů prstenců očí (každé oko zvlášť), 4 antény a 6 znaků na hrudi v libovolných barvách, hlavu i tělo ve třech velikostech (S/M/L), a vyexportuje hotové soubory 3MF. Varianty vytváří `src/variants.py`, všechny díly pro konfigurátor znovu vygeneruje `python3 src/export_configurator.py` (potřebuje `pip install cadquery trimesh shapely scikit-image`).

Velikost mění jen vnější tvar: deska, oči, držáky, krk a šrouby jsou ve všech velikostech stejné. Export kontroluje, že se deska do hlavy vejde (žádná kolize), že pusa má od okraje aspoň 2 mm (mřížka proto není na malé kulaté hlavě), že se štítek vejde na čelo těla a že na těle sedí příruba krku (malý zvon proto není). Konfigurátor navíc odhadne hmotnost, při jakém náklonu se robot převrhne a největší díl na podložce.

Tělo z konfigurátoru má jednotné lůžko pro štítek (50 × 32 mm, hloubka 1 mm), takže jakýkoli znak pasuje na jakékoli tělo. Stejné lůžko má i základní tělo `06_telo`, `06a_stitek` je prázdný štítek. Znaky a vlastní logo (SVG) jsou v konfigurátoru.

## Než pustíš celou hlavu
Vytiskni `00_test_oko.stl` (výřez čela s jedním okem a dvěma sloupky) a k tomu `02b_drzak_oka_pravy.stl`. Ověř si:
- že rámeček displeje (Ø 36,5) i s výstupkem připojení (celkem 39 mm, vybrání na vnitřní straně oka) sedí v lůžku a oko je ve výřezu vystředěné,
- že držák přitáhne desku k čelu. Model počítá s tím, že vrchol mosazných závitů M2 je **8,0 mm** za čelem displeje (`BRASS_TOP` v `robot.py`, změřeno).

Pokud je rozdíl větší než asi 0,5 mm, uprav `BRASS_TOP` v `src/robot.py` a spusť `python3 src/export.py` (potřebuje `pip install cadquery`). Sloupky jsou záměrně o 0,5 mm nižší než držák, takže držák při dotažení desku jemně přitlačí. Šrouby dotahuj citlivě.

## Díly a orientace tisku
STL soubory jsou už natočené do tiskové polohy.

| Soubor | Ks | Poznámka |
|---|---|---|
| 01_hlava | 1 | čelem dolů, bez podpěr |
| 01_hlava_s_pruhem.3mf | 1 | hlava + oba pruhy jako jeden objekt se třemi díly (filament 1 šedá, 2 žlutá, 3 černá) – jen nastav barvy. Samostatné STL 01a/01b jsou záloha |
| 02_prstenec_oka | **2** | oranžová, přilepit vteřinovým lepidlem do lůžka v čele |
| 08_slepe_oko.3mf | 1 | **náhradní krytka oka** místo prstence, když jeden displej nefunguje. Zakryje ho a má na sobě namalované oko. Jeden objekt se 3 díly (1 prstenec, 2 černá, 3 azurová), tiskne se zadní stranou dolů a barvy jsou v horních 3 vrstvách. Samostatné STL 08–08b jsou záloha |
| 09_monokl.3mf | 1 | **rozbité okruží („monokl“)** místo prstence nad prasklým displejem: fialový prstenec s prasklinami, ulomeným kouskem a vypadlým šroubkem. 2 díly (1 fialová, 2 černé praskliny v horních 3 vrstvách), zadní stranou dolů. Navržené pro levé oko (při pohledu zepředu), pro pravé v sliceru zrcadli v ose X. STL 09/09a jsou záloha |
| 02a / 02b držák oka levý / pravý | 1+1 | naplocho, jsou zrcadlové |
| 03_zadni_viko | 1 | vnitřní plochou dolů |
| 04_antena | 1 | hlavicí dolů (STL je už otočené), bez podpěr; nasunout do otvoru v temeni |
| 05_krk | 1 | přírubou dolů |
| 06_telo | 1 | vzhůru nohama, bez podpěr |
| 06a_stitek | 1 | prázdný štítek na hruď, vlepit do lůžka (hloubka 1 mm, štítek vyčnívá 0,6 mm, vůle 0,15 mm). Štítek se znakem nebo vlastním logem: konfigurátor |
| 07_spodni_viko | 1 | spodní plochou dolů |

Doporučení: PLA (na prototypu Bambu PLA Basic Light Gray), PETG nebo ASA; vrstva 0,2 mm, 4 perimetry, výplň 15–20 % gyroid. Všechny díly se tisknou bez podpěr. Hlavní barva zabere asi 250 g filamentu.

## Spojovací materiál
- 16× šroub **M3 × 8** s imbusem (půlkulatá nebo válcová hlava)
- 16× závitová vložka **M3, OD 4,5 mm, délka 4 mm** (otvor 4,25 mm, hloubka 7,5 mm): zadní víko hlavy 4×, sloupky držáků očí 4×, dno těla 4×, horní část těla 2×, krk 2×
- 4× šroub **M2 × 6** s imbusem, jen do mosazných závitů desky (vložky M2 nejsou potřeba)
- 4× silikonová nožička Ø 11 mm
- kousek oboustranné pěnové pásky (reproduktor)
- vteřinové lepidlo (prstence očí, štítek)
- USB-C kabel, ideálně tenký a ohebný, případně s úhlovým konektorem

## Elektronika a kabeláž
- Deska leží v hlavě, displeji dopředu. Konektor USB-C na středním můstku míří dolů, v čele je pro zástrčku kanál.
- Kabel vede dnem hlavy, krkem a tělem ven otvorem vzadu dole na těle.
- Reproduktor z balení přilep pěnovou páskou na vnitřní stranu zadního víka za větrací drážky.
- Volitelně: Li-Pol článek 3,7 V do těla, kabel krkem ke konektoru BAT (MX1.25).

## Montáž
1. Zataví vložky M3 (sloupky v hlavě, rohy hlavy, tělo, krk).
2. Desku vlož zezadu do hlavy, rámečky displejů do lůžek v čele.
3. Na každé oko přišroubuj držák: 2× M2 do mosazných závitů desky, pak 2× M3 do sloupků. Dotahuj citlivě.
4. Krk přišroubuj na tělo (2× M3), kabel protáhni krkem.
5. Hlavu nasaď na krk a přišroubuj 2× M3 přes dno hlavy (zezadu, před nasazením víka).
6. Reproduktor na zadní víko, víko 4× M3. Spodní víko 4× M3.
7. Prstence očí, štítek, anténa, nožičky.
