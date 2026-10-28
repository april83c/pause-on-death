# Death Pause (Geode, GD 2.2081)

Opens the real pause menu when you die. Resume = respawn like vanilla.

## Build (no local toolchain needed)
1. Push this folder to a new GitHub repo.
2. Actions tab -> "Build Geode Mod" runs automatically.
3. Download the "Build Output" artifact -> `april.death-pause.geode`.
4. Drop it in `<GD folder>/geode/mods/` (or Geode > mods folder button in-game).

## Build locally (Windows)
Needs Geode CLI + SDK (`geode sdk install`, `geode sdk install-binaries`) and VS Build Tools:
    geode build
