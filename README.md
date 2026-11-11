# Star-Forge

An original, offline-first, top-down 2D pixel-art space sandbox RPG.

## Current milestone: v0.1 playable prototype

The repository now contains the first small playable star-system sandbox: steer a ship, fight a pirate, watch an NPC freighter drop physical cargo when destroyed, collect salvage, jettison selected cargo by quantity, and sell goods at a station. NPC cargo collection can be toggled with F3.

See [v0.1 prototype notes](docs/v0.1-playable-prototype.md) and [controls](docs/controls.md).

## Project goals

- Android is the primary platform; desktop builds are the initial development target.
- Low-memory Android compatibility is a design goal to validate on real devices.
- Native C17 game code with explicit memory ownership and predictable runtime behavior.
- Offline exploration, ship customization, combat, trading, missions, factions, and progression.

## Technology

| Area | Choice |
|---|---|
| Core language | C17 |
| Platform layer | SDL3 |
| Build system | CMake |
| Android target | NDK + CMake; packaging is a later milestone |
| Game definitions | JSON during authoring |
| Version control | Git + GitHub |

## Build desktop prototype

Requires CMake 3.21+, a C17 compiler, Git, and network access on the first configure if SDL3 is fetched automatically.

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
    ctest --test-dir build --output-on-failure

Run the generated `star_forge` executable.

To use a system-installed SDL3 package, configure with `-DSTARFORGE_FETCH_SDL3=OFF`.

## Repository layout

- `src/` — SDL frontend and simulation
- `data/` — starter item and NPC role definitions
- `assets/` — sprites, audio, fonts, and other game assets
- `docs/` — architecture, performance, design, and controls
- `tests/` — simulation tests
- `.github/workflows/` — CI and build automation

## Not yet implemented

Android APK packaging, touch controls, final sprite art, animation, save/load, FTL travel, multiple systems, dynamic markets, advanced NPC factions/reputation, and full ship fitting are future milestones.

See [Architecture](docs/architecture.md), [Performance](docs/performance.md), [Asset Pipeline](docs/asset-pipeline.md), and [Game Design Foundations](docs/game-design.md).
