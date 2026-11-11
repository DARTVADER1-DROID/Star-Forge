# Star Forge

An original, offline-first, top-down 2D pixel-art space sandbox RPG.

## Active development: Godot 4 + GDScript migration

The active migration branch is `migration/godot-gdscript`. The project is moving to Godot 4 with typed GDScript to improve iteration speed, scene architecture, UI scaling and maintainability.

The previous SDL3/C17 implementation is retained temporarily as a reference while mechanics are ported and validated. It is not the active game runtime.

## Project goals

- Android is the primary platform; desktop builds are for development and testing.
- Landscape-only, touch-first gameplay. No keyboard or mouse gameplay bindings.
- Offline exploration, ship customization, combat, trading, missions, factions and progression.
- Low-memory Android compatibility is an optimisation target that must be verified on physical devices.
- Preserve readable, modular, data-driven code.

## Technology

| Area | Choice |
|---|---|
| Engine | Godot 4.7.2 stable |
| Main language | Typed GDScript |
| Native optimisation | Consider C/C++ only after profiling and feature completion |
| Renderer | Compatibility / OpenGL |
| Game type | 2D |
| Android target | arm64-v8a debug APK |
| Cloud Android builds | GitHub Actions |
| Content definitions | JSON and Godot resources |
| Version control | Git + GitHub |

## Cloud Android build

No desktop installation is required. GitHub Actions installs Godot, Android SDK and export templates, imports the project and exports the APK.

1. Open the repository's Actions tab.
2. Select **Build Star Forge Android APK**.
3. Run the workflow on `migration/godot-gdscript`.
4. Download the `star-forge-godot-android-debug` artifact after a successful run.

Pushes to the migration branch also trigger the build. Artifacts are retained for 14 days.

## Initial Godot vertical slice

- Landscape project configuration and adaptive canvas-item scaling.
- Touch-only floating joystick.
- Dedicated BOOST and FIRE touch buttons.
- Top-down ship steering and movement.
- Hull, armour and energy status.
- Weapons consume energy; ordinary movement does not.
- Projectile scene separation.

The first slice uses placeholder vector-drawn visuals. Existing art is retained in the asset pack and will be integrated as the Godot asset pipeline is validated.

## Migration plan

See [Godot migration notes](docs/godot-migration.md), [game design](docs/game-design.md), [gameplay mechanics](docs/gameplay-mechanics.md) and [performance requirements](docs/performance.md).

The SDL code under `src/` remains reference material until equivalent Godot mechanics are implemented and tested. Do not remove it prematurely.
