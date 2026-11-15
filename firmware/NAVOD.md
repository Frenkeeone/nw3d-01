# NW3D-01 – firmware (verze 2)

Firmware pro desku **Waveshare ESP32-S3-DualEye-LCD-1.28**.

**Většina lidí nic z tohoto návodu nepotřebuje.** Firmware se nahrává z prohlížeče na stránce **[frenkeeone.github.io/nw3d-01](https://frenkeeone.github.io/nw3d-01/)** a vše ostatní se nastavuje na webu robota. Tenhle návod je pro ty, kdo chtějí kód upravovat a překládat sami.

## Jak to funguje
1. **Instalace:** webová stránka nahraje hotový firmware (ESP Web Tools) a hned potom přes USB zadá Wi-Fi (protokol Improv).
2. **Bez Wi-Fi:** když robot Wi-Fi nezná nebo se k ní minutu nemůže připojit, vytvoří vlastní síť **NW3D-xxxx**. Po připojení telefonem se otevře nastavovací stránka (adresa 192.168.4.1). Oči ukazují název sítě a adresu.
3. **Průvodce na webu robota** (`http://nw3d-robot.local`):
   - jazyk (čeština / angličtina),
   - Wi-Fi,
   - tiskárna: přihlášení k Bambu účtu e-mailem a heslem (případně kód z e-mailu nebo 2FA), výběr tiskárny ze seznamu, připojení přes cloud nebo v LAN; případně ručně v LAN bez účtu, nebo bez tiskárny,
   - město pro počasí a časové pásmo.
4. **Aktualizace:** robot dvakrát denně zjistí, jestli je na GitHubu novější vydání. Na webu se pak dá nainstalovat jedním tlačítkem, nebo nahrát soubor `nw3d-01-ota.bin` ručně.

Heslo k Bambu účtu se použije jen pro získání tokenu a nikam se neukládá. Token, Wi-Fi a nastavení zůstávají v paměti desky (NVS). Webová stránka robota token nikdy neukazuje.

## Vlastní překlad – PlatformIO (doporučeno)
VS Code + rozšíření PlatformIO, otevři složku `firmware` a dej *Upload*. Soubor `platformio.ini` je připravený (Arduino core 3.3, LovyanGFX 1.2.19, PubSubClient 2.8, ArduinoJson 7).

## Vlastní překlad – Arduino IDE
1. Arduino IDE 2, deska **esp32 by Espressif Systems** verze 3.2.0 nebo novější (URL správce desek `https://espressif.github.io/arduino-esp32/package_esp32_index.json`).
2. Knihovny: **LovyanGFX** ≥ 1.2.19, **PubSubClient**, **ArduinoJson** 7.
3. Otevři `NW3D_Eyes/NW3D_Eyes.ino`.
4. Nastavení v menu Nástroje:

| Položka | Hodnota |
|---|---|
| Deska | ESP32S3 Dev Module |
| USB CDC On Boot | Enabled |
| Flash Mode | QIO 80MHz |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
| PSRAM | **OPI PSRAM** (bez toho se obraz nevytvoří) |
| USB Mode | Hardware CDC and JTAG |

5. `config.h` je **nepovinný**. Když v něm vyplníš Wi-Fi a tiskárnu, robot je při úplně prvním spuštění převezme (dokud je paměť desky prázdná). Jinak vše nastavíš na webu.

## Struktura kódu
| Soubor | Co dělá |
|---|---|
| `NW3D_Eyes.ino` | hlavní smyčka: volba obrazovek, jas, přechody stavu tisku |
| `board.*` | displeje GC9A01 (DMA), podsvícení, tlačítko BOOT |
| `ui.*`, `fonts_vlw.h` | kreslení obrazovek (lze přeložit i na PC pro náhledy, `tools/preview.cpp`) |
| `i18n.*` | překlady textů na displejích |
| `cfg.*` | připojení: Wi-Fi, tiskárna, token, poloha (NVS) |
| `settings.*` | vzhled: obsah očí, jas, noc, kurzy, zvuk (NVS) |
| `wifimgr.*` | Wi-Fi, nastavovací síť s přesměrováním, Improv přes USB |
| `printer*.cpp` | MQTT spojení s tiskárnou (cloud i LAN) a rozbor zpráv |
| `bambu.*` | Bambu cloud: přihlášení, seznam tiskáren, poslední úloha |
| `cover.*` | náhled tištěného modelu (obrázek úlohy z cloudu) |
| `weather.*`, `quotes.*` | počasí (Open-Meteo), kurzy (Yahoo Finance) |
| `audio.*`, `es8311*` | zvuky přes kodek ES8311 a reproduktor desky |
| `ota.*` | kontrola a instalace nové verze z GitHubu |
| `web.*`, `web_page.h` | webové rozhraní a API |

## Vydání nové verze
Na GitHubu otevři **Actions → Firmware → Run workflow** a do pole vyplň verzi, např. `v2.0.1` (případně v gitu vytvoř a nahraj značku `v2.0.1`). GitHub Actions firmware sestaví, vytvoří vydání se soubory `nw3d-01-full.bin` (webová instalace) a `nw3d-01-ota.bin` (aktualizace přes web) a obnoví instalační stránku. Značka s pomlčkou (`v2.1.0-beta.1`) se vydá jako testovací: instalační stránka ji nabídne, ale roboti ji sami nehlásí.

## Řešení potíží
| Co vidíš | Co udělat |
|---|---|
| Displeje svítí, ale nic neukazují | chybí **PSRAM: OPI PSRAM** (jen při vlastním překladu) |
| Obraz je otočený | web → Displeje → Natočení |
| L a P jsou prohozené | web → Displeje → Prohodit |
| Oči ukazují „NASTAVENÍ – připoj se k Wi-Fi NW3D-…“ | robot nezná Wi-Fi nebo se k ní nedostane; připoj se k jeho síti a zadej Wi-Fi |
| `nw3d-robot.local` se neotevře | použij IP adresu z pravého oka |
| „Přihlášení vypršelo“ | web → Připojení → Změnit tiskárnu → přihlas se znovu (token platí asi 3 měsíce) |
| „Žádná data“ v LAN | na tiskárně zapni LAN režim (u X1/H2/P2 i Developer mode) |
| Počasí ukazuje „nastav město“ | web → Připojení → Změnit místo |
| Náhled modelu „cloud only“ | náhled funguje jen při připojení přes Bambu cloud |
