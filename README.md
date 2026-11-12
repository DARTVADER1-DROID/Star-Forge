# Star Forge

Star Forge is an original, offline-first, top-down 2D pixel-art space sandbox RPG built exclusively for Android phones and tablets.

## Technology

- **Engine:** Godot 4.7.2 stable
- **Language:** typed GDScript
- **Renderer:** Compatibility (OpenGL ES)
- **Target platform:** Android only
- **Display:** landscape orientation, scalable 960 × 540 logical canvas
- **Input:** touchscreen only; no keyboard or mouse gameplay support
- **Builds:** cloud-based Android APK export through GitHub Actions

The Godot editor may be used as a development tool, but the game itself is an Android-only product. There are no desktop game targets or desktop gameplay controls.

## Development

Open this repository's root directory in Godot 4.7.2 to edit the project. The entry scene is `scenes/main.tscn`. The project uses a 960 × 540 logical canvas with canvas-item stretching and the Compatibility renderer.

## Current playable systems

- Floating touch joystick with rate-limited ship turning and thrust.
- Dedicated BOOST and FIRE buttons.
- Hull, armour and energy; movement is free, weapons consume energy.
- Energy regeneration, weapon cooldown and projectile combat.
- NPC cargo hauler, pirate interceptor and patrol roles.
- Asteroid, planet, moon and station collision bodies.
- Physical cargo drops, collection, cargo capacity, jettison and station selling.
- Optional NPC salvage collection.
- Four connected star systems with FTL drive and a 5-fuel jump cost.
- Local save/load, per-system NPC/drop persistence and emergency recovery.

The game is still being iterated. This is a playable foundation, not a claim that every planned RPG feature is complete.

## Cloud Android APK

No local Android SDK or desktop build setup is required to produce a debug APK.

1. Open the repository's Actions tab.
2. Select **Build Star Forge Android APK**.
3. Run it on branch `migration/godot-gdscript`, or push a commit to that branch.
4. Download the `star-forge-godot-android-debug` artifact after the workflow succeeds.

The APK is arm64-v8a and debug-signed. It is for testing, not Play Store release.

## Validation

The CI workflow installs Godot headlessly, imports the project and runs the project-aware GDScript test scene. The Android workflow separately exports and uploads the APK. Automated CI does not replace testing on a physical phone.

## Project layout

- `project.godot` — engine, renderer, Android display and autoload settings
- `scenes/` — reusable gameplay scenes
- `scripts/autoload/` — content database and persistent game state
- `scripts/player/` — player ship simulation
- `scripts/npc/` — NPC behavior
- `scripts/combat/` — projectiles and combat
- `scripts/world/` — systems, obstacles, planets and cargo drops
- `scripts/ui/` — touch HUD and panels
- `data/` — JSON content definitions
- `assets/` — source art and imported runtime resources
- `tests/` — headless project and gameplay regression tests
- `.github/workflows/` — Godot validation and Android APK export

See `docs/architecture.md`, `docs/gameplay-mechanics.md` and `docs/performance.md`.
