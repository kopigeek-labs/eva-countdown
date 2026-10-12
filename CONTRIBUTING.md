# Contributing

Small, focused changes are welcome. For bugs, include the board model, firmware version, ESP-IDF version, reproduction steps, and expected/actual behavior. A clear display photo and short sanitized serial log help.

## Local checks

1. Run `./tools/check.sh` with a C++17 compiler.
2. If changing artwork, install `tools/requirements.txt`, regenerate `main/assets.h` with `tools/generate_assets.py`, then run `PYTHON=.venv/bin/python ./tools/preview.sh` and inspect the native-size PNGs.
3. Build with ESP-IDF 5.4.2: `idf.py build`.
4. Describe what you actually checked. A host preview or passing build does not establish physical display behavior.

C++ formatting follows `.clang-format`. Avoid hand-editing `main/assets.h`; edit the generator or font sources. Keep the vendor register sequence separate from layout work. Preserve attribution and separate asset licenses.

Do not commit build outputs, flash backups, device identifiers, credentials, or local paths. Pull requests should explain the user-visible change and relevant validation. No automatic publishing or hardware flashing is performed by CI.
