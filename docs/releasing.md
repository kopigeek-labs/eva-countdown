# Preparing v0.1.0

Everything here can be prepared locally. Building a ZIP does not publish it. The first GitHub release should be tagged **v0.1.0**, matching `VERSION` and the firmware's reported application version.

## 1. Review the candidate

Read the README and THIRD_PARTY.md. The proposed project license is Apache-2.0; font and artwork exceptions remain explicit. Check the native-size preview images, not just the enlarged README image.

Run:

```sh
./tools/check.sh
.venv/bin/python tools/generate_assets.py
PYTHON=.venv/bin/python ./tools/preview.sh
idf.py build
python tools/package_release.py
```

The packaging command creates a ZIP and checksum in ignored `dist/`. It bundles three flash images, offsets/hashes, the corresponding working-tree source, font sources, project licenses and SDK notices. It refuses to overwrite an existing ZIP. Use `--output-dir dist/recheck` for another candidate. The package captures a source snapshot, not a Git tag; rebuild after every code change before packaging. `--build-dir` can select another build directory.

GitHub Actions will run host checks, verify generated assets, build with IDF 5.4.2, and upload a candidate artifact after the first push. This workflow has not been exercised on GitHub yet and never creates a release.

## 2. Try the exact candidate on the board

The earlier build's display, start/pause and reset were owner-verified. For the release candidate, confirm:

- [ ] Boot shows `5:00:00` in the intended orientation.
- [ ] KEY starts, pauses and resumes; the short release after a hold does not restart.
- [ ] Holding KEY and pressing BOOT each reset to five minutes.
- [ ] The last minute warning is readable and the timer stops at zero.
- [ ] KEY at zero starts a fresh five-minute countdown.

Record the exact candidate ZIP checksum and observations. These are quick functional checks, not a display-performance benchmark. A release can remain a GitHub prerelease while physical checks are incomplete.

## 3. Review what will be public

`git status --short` should show only project material. Do not include `artifacts/`, `dist/`, `backups/`, full-flash dumps, private serial logs or local SDK paths. The source has vendored fonts with separate licenses; retain them and their attributions.

For the first commit, explicitly stage the source/documentation paths after review:

```sh
git add .gitignore .editorconfig .clang-format .github CMakeLists.txt VERSION sdkconfig.defaults README.md LICENSE NOTICE THIRD_PARTY.md CHANGELOG.md CONTRIBUTING.md main tools assets licenses docs preview
git diff --cached --check
git diff --cached --stat
```

Then inspect the staged diff. Committing and tagging are separate from pushing. When ready, make the initial commit and annotated `v0.1.0` tag. Do not tag an older or different binary's source.

## 4. Publish only when explicitly approved

Keep the GitHub repository private while the blog and support links are placeholders. Replace the affiliate / Buy Me a Coffee links before considering public visibility. A private source push is authorized; changing visibility or publishing a release requires separate approval.


After approval: push `main`, wait for CI, then tag/push the exact accepted commit and create a GitHub release with `docs/release-notes-v0.1.0.md`. Attach the verified ZIP and its `.sha256` file. Use the prerelease flag if any acceptance checks are still open. The README's local-candidate notice and changelog date should be updated as part of that publication preparation.

No commit, tag, push, release creation, or device flash is performed by the local package helper.
