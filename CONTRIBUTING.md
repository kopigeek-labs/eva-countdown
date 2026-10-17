# Contributing

Small, focused changes are welcome. For bugs, include the board model, firmware version, ESP-IDF version, reproduction steps, and expected/actual behavior. A clear display photo and short sanitized serial log help.

## Local checks

1. Run `./tools/check.sh` with a C++17 compiler.
2. If changing artwork, install `tools/requirements.txt`, run `.venv/bin/python tools/fetch_fonts.py` to download and verify the original fonts, regenerate `main/assets.h` with `tools/generate_assets.py` and `main/ui_assets.h` with `tools/generate_ui.py`, then run `PYTHON=.venv/bin/python ./tools/preview.sh` and inspect the native-size PNGs.
3. Build with ESP-IDF 5.4.2: `idf.py build`.
4. Describe what you actually checked. A host preview or passing build does not establish physical display behavior.

C++ formatting follows `.clang-format`. Avoid hand-editing `main/assets.h` or `main/ui_assets.h`; edit the generator or font sources. Keep the vendor register sequence separate from layout work. Preserve attribution and separate asset licenses.

Do not commit build outputs, flash backups, device identifiers, credentials, or local paths. Pull requests should explain the user-visible change and relevant validation. No automatic publishing or hardware flashing is performed by CI.

## Licensing and attribution

By submitting a contribution, you agree to license your original contribution under this project's Apache-2.0 license. Only submit material you have the right to contribute. Bundled third-party code, fonts and artwork retain their own licenses; this contribution policy does not relicense them.

When adapting another project, include its name, source URL, revision, copyright notice, license and a description of the changes in `THIRD_PARTY.md`. Preserve applicable source headers and license texts in `licenses/`, and update `NOTICE` with relevant attribution. In particular, preserve the CC BY-SA 3.0 terms on the True LCD NGE digit adaptations and the SIL OFL notices for the bundled fonts. Do not replace those notices with an Apache-only header.

Credits to EVA Countdown and upstream contributors travel with redistributions through the applicable licenses and notices. A visible project link in a fork's README is welcome; no additional advertising or on-screen credit condition is imposed.
