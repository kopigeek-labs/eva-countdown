# Sources and licenses

## EVA timer

Reference: https://github.com/scottykwok/eva-timer
Pinned revision: `42e1b12361aa1f569bea8026f31c5cd3d4096da7`.

- Design reference: the upstream SVG and timer layout, under `licenses/eva-timer-MIT.txt`, copyright 2023 Scotty Kwok. The reference SVG is not bundled. The firmware is a new native implementation; it does not execute the browser/PyScript application.
- Digit font: `src/fonts/true-lcd-nge/true-lcd-nge.ttf` at the pinned revision. `tools/fetch_fonts.py` downloads an unchanged, hash-verified copy to the ignored `assets/fonts/true-lcd-nge.ttf` cache when regenerating graphics.
- True LCD NGE by scottykwok, derived from True LCD by hippiefabou, CC BY-SA 3.0. Source links and terms in `licenses/True-LCD-NGE.txt`. Generated digit bitmaps are resized monochrome adaptations and retain CC BY-SA 3.0. https://creativecommons.org/licenses/by-sa/3.0/

## Other font assets

- Zen Antique Regular from https://github.com/google/fonts/tree/main/ofl/zenantique, SIL OFL 1.1; license retained in `licenses/ZenAntique-OFL.txt`.
- Barlow Condensed Regular from https://github.com/google/fonts/tree/main/ofl/barlowcondensed, SIL OFL 1.1; license retained in `licenses/BarlowCondensed-OFL.txt`. Used for compact Latin labels rather than bundling the reference repository's Arial font.
- Font binaries are not bundled in the source checkout or release ZIP. `tools/fetch_fonts.py` downloads the exact originals from pinned upstream commits and verifies `assets/SHA256SUMS`. Zen Antique is pinned to Google Fonts commit `30a4532c801f2650897aa9949ceab031a6401037`; Barlow Condensed is pinned to `89f5431ff0db41bd2fe3f7ba21a723a01622428b`. Once fetched, generation is offline. Normal firmware builds need no font download.

## Waveshare

Source: https://github.com/waveshareteam/ESP32-S3-RLCD-4.2
Pinned revision: `eb1f63427d735a22b9c30e22fa63ebddae1834d3`.

Display initialization and landscape packing adapted from `02_Example/ESP-IDF/08_LVGL_V8_Test/components/port_bsp/display_bsp.cpp`; GPIO assignments from that example's `main/main.cpp` and `main/user_config.h`. Button GPIOs from `02_Example/ESP-IDF/10_FactoryProgram/components/port_bsp/button_bsp.c`.

Copyright 2026 Waveshare, Apache-2.0; retained in `licenses/Waveshare-Apache-2.0.txt`. Changes: native synchronous SPI transport, explicit internal DMA buffer, row-major rendering followed by packed conversion, no LVGL or PSRAM lookup tables. Register sequence and reset delays preserved.

## Project additions and generated files

Original source, scripts, documentation and layout additions: Copyright 2026
Kopigeek Labs and contributors, Apache-2.0 (root LICENSE). This does not replace
licenses on bundled third-party works.

`main/assets.h` combines a generated layout/background with font-derived bitmap
arrays. The digit arrays retain CC BY-SA 3.0. The file as a whole must not be
represented as solely Apache-2.0. The preview PNG includes those same assets;
retain this attribution when redistributing it. The generators and a verified
font-source downloader are included so the adaptations can be modified.

## Firmware runtime

Compiled firmware also incorporates ESP-IDF and toolchain runtime code. The
release packager includes ESP-IDF's Apache license, FreeRTOS's MIT license,
Newlib's bundled notices, and GCC runtime licensing notices alongside the source
and asset licenses. Source-only contributors need not install font tooling to
build the firmware.

## v0.2 additions

`main/ui_assets.h` contains lettering generated from the same Barlow Condensed and Zen Antique fonts under their existing SIL OFL notices. Fetch the original fonts with `tools/fetch_fonts.py`, then regenerate with `tools/generate_ui.py`.

RTC pins checked against Waveshare's pinned `02_Example/ESP-IDF/04_I2C_PCF85063/main/user_config.h` and the board schematic. The PCF85063A driver is a new minimal implementation of the NXP register interface, not a copy of SensorLib. USB host sensing uses bundled ESP-IDF's `usb_serial_jtag_is_connected()`. Primary references: [board schematic](https://files.waveshare.com/wiki/ESP32-S3-RLCD-4.2/ESP32-S3-RLCD-4.2-schematic.pdf) and [ESP-IDF USB Serial/JTAG API](https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32s3/api-reference/peripherals/usb_serial_jtag.html).
