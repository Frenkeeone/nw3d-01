# Third-party components

| Component | Where | Licence |
|---|---|---|
| Chakra Petch font (Cadson Demak) | `firmware/NW3D_Eyes/fonts_vlw.h`, `firmware/tools/*.ttf` | SIL Open Font License 1.1 – `firmware/tools/OFL.txt` |
| ESP LCD GC9A01 driver (Espressif Systems) | `firmware/NW3D_Eyes/esp_lcd_gc9a01.*` | Apache License 2.0 |
| ES8311 audio codec driver (Espressif Systems, Arduino port from Waveshare examples) | `firmware/NW3D_Eyes/es8311*` | Apache License 2.0 |
| LovyanGFX | PlatformIO dependency | FreeBSD (BSD-2-Clause) |
| PubSubClient | PlatformIO dependency | MIT |
| ArduinoJson | PlatformIO dependency | MIT |
| ESP Web Tools (installer page) | loaded from unpkg.com | Apache License 2.0 |
| three.js 0.160.0 (configurator) | `site/konfigurator/lib/three.module.min.js`, `STLLoader.js`, `OrbitControls.js`, `SVGLoader.js`, `GLTFExporter.js` | MIT |
| fflate 0.8.2 (configurator) | `site/konfigurator/lib/fflate.js` | MIT |
| @google/model-viewer 4.3.1 (configurator AR preview, bundles three.js) | `site/konfigurator/lib/model-viewer.min.js` | Apache-2.0 / BSD-3-Clause (`LICENSE-model-viewer.txt`) |
| qrcode-generator 2.0.4 (configurator QR code) | `site/konfigurator/lib/qrcode.mjs` | MIT (Kazuhiko Arase) |

Apache 2.0 text: https://www.apache.org/licenses/LICENSE-2.0

Pin assignments follow the Waveshare ESP32-S3-DualEye-LCD-1.28 examples. The
Bambu Lab printer message format comes from community projects (ha-bambulab,
OpenBambuAPI); Bambu Lab does not document it officially.
