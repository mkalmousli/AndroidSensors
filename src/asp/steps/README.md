# steps

One module per pipeline step; `asp <step>` runs it.

`sync` (repo sync) → `patch` (install the overlay, render generated files) → `build` → `run` (boot the
emulator) → `verify` (end-to-end check). Also: `package` (emulator zip split into <2 GB parts + checksums),
`publish` (GitHub release, stdlib only), `doctor` (list stock packages our HALs replace).
`asp release` = sync + patch + build + package + publish.
