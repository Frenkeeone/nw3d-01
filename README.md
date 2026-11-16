# NW3D-01 – desk robot with display eyes

**[Česky níže ↓](#česky)**

NW3D-01 is a 3D-printed desk robot whose round display eyes show the progress of your **Bambu Lab** print, a clock, weather and stock or commodity prices. It is built around a single board, the **Waveshare ESP32-S3-DualEye-LCD-1.28**, and needs no soldering.

| | |
|---|---|
| **Install firmware** | **[frenkeeone.github.io/nw3d-01](https://frenkeeone.github.io/nw3d-01/)** – straight from Chrome/Edge, no software needed |
| **Configurator** | **[frenkeeone.github.io/nw3d-01/konfigurator](https://frenkeeone.github.io/nw3d-01/konfigurator/)** – pick head, body, eyes, mouth, antenna, emblem and colours, download 3MF files |
| **3D models** | [`cad/`](cad/) – STL, 3MF (multi-colour) and STEP |
| **Firmware source** | [`firmware/`](firmware/) – Arduino / PlatformIO |

## What it does
- Progress ring around the eyes, time left, finish time, layer, temperatures and a picture of the model being printed
- Animated eyes, clock, date, weather (Open-Meteo) and up to 4 tickers (Brent crude, gold, Bitcoin, EUR/CZK, stocks…)
- Each eye's content is chosen separately on the robot's web page, for printing and for idle time
- "DONE" screen and a short tune when a print finishes, an alert when it fails
- Night mode with dimming, Czech and English
- Setup wizard: sign in to your Bambu account (e-mail + password + e-mail code), pick your printer from the list, type your city
- Works through Bambu cloud (printer stays in normal mode) or in LAN mode
- Updates over Wi-Fi from GitHub releases

## Getting started
1. Print the parts (see [`cad/README.md`](cad/README.md)) and buy the [parts list](#bill-of-materials).
2. Open **[the installer](https://frenkeeone.github.io/nw3d-01/)**, plug in the board and click *Install firmware*.
3. Enter your Wi-Fi when asked, then open **http://nw3d-robot.local** and follow the wizard.
4. Assemble the robot.

No computer at hand? A robot without Wi-Fi settings opens its own network **NW3D-xxxx** – join it with a phone and the setup page appears.

## Bill of materials
| Item | Qty | Note |
|---|---|---|
| Waveshare ESP32-S3-DualEye-LCD-1.28 | 1 | ESP32-S3 (16 MB flash, 8 MB PSRAM), two 1.28" round GC9A01 displays, speaker |
| Screw M3 × 8, hex socket | 16 | button or cylinder head |
| Heat-set insert M3, OD 4.5 mm, length 4 mm | 16 | hole 4.25 mm |
| Screw M2 × 6 | 4 | into the board's brass standoffs |
| Silicone feet Ø 11 mm | 4 | |
| Foam double-sided tape | small piece | speaker |
| Super glue | | eye rings, chest badge |
| USB-C data cable + 5 V adapter | 1 | thin and flexible is best |
| Filament | ≈ 250 g | main colour (light grey in the photos) + small amounts of yellow, black and cyan |

The filament amount is an estimate from the model volume; your slicer will tell you exactly.

## Privacy
**The author of this project does not collect, receive or have access to any of your data.** There is no server, account or analytics run by the author.
- Wi-Fi credentials, settings, the Bambu access token and the optional web-page password stay **only in the robot's memory**. The password is stored as a salted SHA-256 hash, never in plain text.
- Your Bambu password is used once to obtain the token and is never stored.
- The robot connects only to your printer, Bambu Lab servers (cloud mode), Open-Meteo (weather), Yahoo Finance (tickers) and GitHub (update check).
- The installer page is static and hosted on GitHub Pages; the installer script is loaded from unpkg.com. Like any website, these services see visitors' IP addresses under their own privacy policies.
- The robot's web page is reachable only in your home network. You can protect it with a password (System → Password). Forgot it? Reinstall the firmware with "erase device".

## Licence
- Firmware and scripts: [MIT](LICENSE)
- 3D models (`cad/`): [CC BY-NC-SA 4.0](cad/LICENSE) – share and remix, non-commercial
- Third-party parts: Chakra Petch font (SIL OFL 1.1), Espressif GC9A01 and ES8311 drivers (Apache 2.0) – see [NOTICE.md](NOTICE.md)

Unofficial project, not affiliated with Bambu Lab, Waveshare or Yahoo. The Bambu printer message format is undocumented and may change with printer firmware updates.

---

## Česky

NW3D-01 je stolní robot z 3D tiskárny. Jeho kulaté displeje-oči ukazují průběh tisku na tiskárně **Bambu Lab**, hodiny, počasí a kurzy. Celý je postavený na jedné desce **Waveshare ESP32-S3-DualEye-LCD-1.28** a nic se nepájí.

- **Instalace firmwaru:** **[frenkeeone.github.io/nw3d-01](https://frenkeeone.github.io/nw3d-01/)** – přímo v prohlížeči Chrome nebo Edge, nic se neinstaluje
- **Konfigurátor:** **[frenkeeone.github.io/nw3d-01/konfigurator](https://frenkeeone.github.io/nw3d-01/konfigurator/)** – tvar hlavy a těla, oči, pusa, anténa, znak a barvy, stažení 3MF
- **Modely:** [`cad/`](cad/) – STL, 3MF (vícebarevné) a STEP, návod v [`cad/README.cs.md`](cad/README.cs.md)
- **Zdrojový kód:** [`firmware/`](firmware/), návod pro vlastní překlad v [`firmware/NAVOD.md`](firmware/NAVOD.md)

### Postup
1. Vytiskni díly a nakup součástky (tabulka výše: deska Waveshare, 16× šroub M3×8, 16× zalisovací matice M3 4,5×4 mm, 4× šroub M2×6, 4 nožičky, oboustranná páska, vteřinové lepidlo, datový kabel USB-C, asi 250 g filamentu).
2. Otevři **[instalační stránku](https://frenkeeone.github.io/nw3d-01/)**, připoj desku a klikni na *Nainstalovat firmware*.
3. Zadej Wi-Fi, otevři **http://nw3d-robot.local** a projdi průvodce (jazyk, Bambu účet, tiskárna, město).
4. Smontuj robota.

Bez počítače: robot, který nezná Wi-Fi, vytvoří vlastní síť **NW3D-xxxx**. Připoj se k ní telefonem a otevře se nastavení.

**Soukromí:** **autor projektu žádná vaše data nesbírá, nedostává ani k nim nemá přístup** – neprovozuje žádný server ani analytiku. Wi-Fi, nastavení, přístupový token a volitelné heslo k webu robota zůstávají jen v paměti robota (heslo jen jako otisk SHA-256 se solí). Heslo k Bambu účtu se použije jednou a neukládá se. Robot komunikuje jen s vaší tiskárnou, servery Bambu Lab, Open-Meteo, Yahoo Finance a GitHubem. Instalační stránka je statická na GitHub Pages (instalátor se načítá z unpkg.com); tyto služby vidí IP adresy návštěvníků podle svých pravidel. Web robota lze chránit heslem (Systém → Heslo); zapomenuté heslo = znovu nahrát firmware se smazáním zařízení.

**Licence:** firmware MIT, modely CC BY-NC-SA 4.0 (nekomerčně).
