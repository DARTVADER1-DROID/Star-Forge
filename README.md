# Star Forge

Star Forge is an original, offline-first, top-down 2D pixel-art space sandbox RPG.

## Engine and language

- **Engine:** Godot 4.7.2 stable
- **Language:** typed GDScript
- **Renderer:** Compatibility (OpenGL ES)
- **Primary platform:** Android, landscape orientation
- **Input:** touch-only gameplay; no keyboard or mouse bindings
- **Cloud builds:** GitHub Actions

This repository is Godot-only. Gameplay, tools, validation and Android packaging are implemented around the Godot project. No native C/C++ source or CMake build system is part of the repository.

## Open the project

Open this repository's root directory in Godot 4.7.2. The entry scene is `scenes/main.tscn`. The project uses a 960 × 540 logical canvas with canvas-item stretching and the Compatibility renderer.

## Current playable systems

- Floating touch joystick with rate-limited ship turning and thrust.
- Dedicated BOOST and FIRE buttons.
- Hull, armour and energy; movement is free, weapons consume energy.
- Energy regeneration, weapon cooldown and projectile combat.
- NPC cargo hauler, pirate interceptor and patrol roles.
- Asteroid, planet and moon-body collision foundations.
- Physical cargo drops, collection, cargo capacity and station selling.
- Four connected star systems with FTL drive and a 5-fuel jump cost.
- Local save/load and per-system cargo-drop persistence.
- Emergency recovery after ship destruction.

The Godot migration is still being iterated. These are implemented foundations, not a claim that every planned RPG feature is complete.

## Cloud Android APK

No desktop installation is required to produce a debug APK.

1. Open the repository's Actions tab.
2. Select **Build Star Forge Android APK**.
3. Run it on branch `migration/godot-gdscript`, or push a commit to that branch.
4. Download the `star-forge-godot-android-debug` artifact after the workflow succeeds.

The APK is arm64-v8a and debug-signed. It is for testing, not Play Store release.

## Validation

The CI workflow installs Godot headlessly, imports the project, parses the scenes/scripts and runs the Godot smoke test. The Android workflow separately exports and uploads the APK. Automated CI does not replace testing on a physical phone.

## Project layout

- `project.godot` — engine, renderer, display and autoload settings
- `scenes/` — reusable gameplay scenes
- `scripts/autoload/` — content database and persistent game state
- `scripts/player/` — player ship simulation
- `scripts/npc/` — NPC behavior
- `scripts/combat/` — projectiles and combat
- `scripts/world/` — systems, obstacles, planets and cargo drops
- `scripts/ui/` — touch HUD and panels
- `data/` — JSON content definitions
- `assets/` — source asset pack and imported runtime art
- `tests/` — headless project and gameplay smoke tests
- `.github/workflows/` — Godot CI and Android export

See `docs/godot-migration.md`, `docs/architecture.md`, `docs/gameplay-mechanics.md` and `docs/performance.md`.
