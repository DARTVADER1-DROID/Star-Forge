# Star-Forge

An original, offline-first, top-down 2D pixel-art space sandbox RPG.

## Project goals

- Android is the primary platform; desktop builds are development and secondary targets.
- Target compatibility includes low-memory Android devices, with **1 GB total device RAM** as the minimum-device design goal.
- Native C game code with explicit memory ownership and predictable runtime behavior.
- Exploration, spacecraft customization, combat, trading, missions, factions, and progression.
- Original art, universe, characters, systems, and content.

## Technology

| Area | Choice |
|---|---|
| Core language | C17 |
| Platform layer | SDL3 |
| Android native toolchain | Android NDK + CMake |
| Android packaging | Gradle |
| Build system | CMake |
| Graphics direction | Custom 2D renderer; OpenGL ES 2.0 compatibility baseline |
| Asset source files | PNG sprites and editable Pixelorama/Aseprite project files |
| Game definitions | JSON during authoring; compact runtime data where useful |
| Version control | Git + GitHub |

## Repository layout

- `src/` — game entry point and game-specific code
- `engine/` — reusable engine modules
- `assets/` — sprites, audio, fonts, and other game assets
- `data/` — data-driven ship, weapon, item, and world definitions
- `docs/` — architecture, performance, and asset-pipeline notes
- `tests/` — automated tests
- `.github/workflows/` — CI and build automation

## Current status

**Foundation stage.** This repository is being initialized. The first milestone is a small, readable SDL3 application shell, followed by the custom renderer, input, timing, resource management, and a playable ship-in-an-asteroid-field prototype.

## Build status

Build instructions will be added as the desktop and Android build paths are validated. Android APK generation is planned through GitHub Actions so a desktop Android Studio installation is not required.

## Design constraints

- Offline gameplay must not depend on an account, server, or network connection.
- Memory budgets are engineering targets and must be validated on real devices.
- Low-end device performance takes priority over expensive visual effects.
- Keep platform-specific code behind narrow interfaces.
- Avoid unnecessary runtime dependencies and allocations in hot loops.

See [Architecture](docs/architecture.md), [Performance](docs/performance.md), [Asset Pipeline](docs/asset-pipeline.md), and [Game Design Foundations](docs/game-design.md).
