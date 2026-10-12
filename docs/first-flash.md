# First device flash — 2026-10-11

Connected board identified as ESP32-S3 revision 0.2, 8 MB embedded PSRAM and 16 MB flash. Secure Boot and flash encryption reported disabled.

A full 16,777,216-byte pre-flash image was read and checksummed in the device workspace's ignored `backups/` directory. This is a saved read, not a tested restore.

Flashed bootloader at 0x0, partition table at 0x8000, application at 0x10000. Esptool verified all three write hashes.

USB log confirmed `eva_countdown`, ESP-IDF 5.4.2, display initialization returning, and:

```text
I (426) eva: Ready: KEY start/pause; hold KEY or press BOOT reset. 5 minutes.
I (1855) eva: Running
I (4935) eva: Paused
```

No panic/abort/watchdog error observed in the approximately ten-second captured log. Serial control lines required DTR released before RTS reset to capture startup reliably.

On 2026-10-12 the owner confirmed: screen looks right; start/pause and reset work. The owner also confirmed that the countdown reached zero without issues. Battery runtime and long-duration reliability have not been confirmed. These observations apply to the initial flashed build, before the v0.1.0 source cleanup.
