# EVA Countdown v0.1.0 — Prerelease

First public prerelease for the Waveshare ESP32-S3-RLCD-4.2: a five-minute, offline countdown inspired by Evangelion's timer display.

## Included

- Start/pause/resume with KEY; hold KEY or press BOOT to reset.
- True LCD NGE digits, Japanese power panels and a flashing final minute.
- ESP-IDF source, bundled font assets, host checks and reproducible display previews.
- Firmware ZIP with flashing instructions, SHA-256 hashes, source and licenses.

## Hardware and installation

Only the Waveshare ESP32-S3-RLCD-4.2 is supported. Use a USB data cable and follow FLASHING.md inside the ZIP. Flashing replaces the installed application.

## Known limits

Fixed five-minute duration. No sound, time-of-day clock, battery reporting or sleep mode. Power source labels are decorative. The 25 Hz display update target is not a measured optical refresh rate.

The original build booted successfully and the owner confirmed appearance, start/pause, reset, and completion of the countdown to zero without issues. The cleaned v0.1.0 candidate passes the firmware build and host logic checks, and its rendered preview frames match the earlier build. It has not yet been flashed to the board. Battery runtime and long-running reliability remain unmeasured.

This prerelease is an early version for trying the timer and reporting feedback. Layout, controls and features may change. Include the firmware version, board model, reproduction steps and a display photo when reporting a problem.

## Credits

Scotty Kwok's eva-timer and True LCD NGE, Waveshare's driver examples, Zen Antique and Barlow Condensed. Apache-2.0 project code; asset-specific terms apply, including CC BY-SA 3.0 for digit adaptations. See THIRD_PARTY.md in the included source. Unofficial fan project.
