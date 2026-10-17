# Flashing a firmware ZIP

Use only a firmware ZIP built for **Waveshare ESP32-S3-RLCD-4.2 (16 MB flash)**. This replaces the installed application. Preserve any existing firmware you need; a flash dump can contain private data and must not be committed or published.

1. Verify the ZIP checksum before extracting, then unzip into a folder and open a terminal there.
2. Verify the extracted file checksums using `SHA256SUMS`. On Linux use `sha256sum -c`; on macOS use `shasum -a 256 -c`. On Windows use `Get-FileHash -Algorithm SHA256` and compare against the supplied values.
3. Install **esptool 4.12.0** into a Python virtual environment, then activate it:

   ```sh
   python -m pip install esptool==4.12.0
   ```

4. Connect a USB data cable, identify the board's serial port, then run from the extracted folder:

   ```sh
   python -m esptool --chip esp32s3 --port YOUR_SERIAL_PORT --baud 460800 write_flash --flash_mode dio --flash_size 16MB --flash_freq 80m 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 eva_countdown.bin
   ```

5. For v0.2 after reset, expect Normal `5:00:00` paused. Try KEY to start/pause and hold KEY to reset. BOOT click cycles screens; hold BOOT in Countdown to choose Racing `1:00:00`. Set Clock over USB with the included `source/tools/set_clock.py` helper.

`manifest.json` records binary hashes, offsets, version, and build toolchain. The ZIP contains the corresponding source and license notices. It contains no device backup, NVS dump or credentials. Do not use an erase-all step for routine flashing.

## Troubleshooting

- **Port missing:** try a known data cable, check board power, reconnect, and identify the new port. Close any other serial monitor.
- **Cannot connect:** hold BOOT while powering on to enter download mode, release it, and retry. Check the board manual for the physical sequence.
- **No serial output:** use 115200 baud with USB Serial/JTAG and release DTR. Prefer `idf.py monitor` for firmware development.
- **Blank/incorrect display:** confirm the exact board, firmware version, and ambient lighting. File an issue with a photo and boot log, omitting private data.
- **Want the previous firmware:** use your own retained backup or Waveshare's matching factory firmware. This project does not ship a factory image or claim a tested restore procedure.
