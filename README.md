# Star-Forge

An original, offline-first, top-down 2D pixel-art space sandbox RPG.

## Current milestone: v0.1 playable prototype

The repository contains the first playable star-system sandbox: steer a ship, fight a pirate, destroy an NPC freighter, collect physical cargo salvage, jettison selected cargo by quantity, and sell goods at a station. NPC cargo collection can be toggled with F3.

See [v0.1 prototype notes](docs/v0.1-playable-prototype.md) and [controls](docs/controls.md).

## Project goals

- Android is the primary platform; desktop builds are for development and testing.
- Low-memory Android compatibility is a design goal to validate on real devices.
- Native C17 game code with explicit memory ownership and predictable runtime behavior.
- Offline exploration, ship customization, combat, trading, missions, factions, and progression.

## Technology

| Area | Choice |
|---|---|
| Core language | C17 |
| Platform layer | SDL3 |
| Build system | CMake |
| Android target | Android NDK + SDL3 Android project |
| Cloud Android builds | GitHub Actions |
| Game definitions | JSON during authoring |
| Version control | Git + GitHub |

## Build an Android APK without installing anything locally

Android APKs are built on GitHub's cloud runners. You do not need Android Studio, the Android SDK, the NDK, Gradle, or a C compiler installed on your desktop.

1. Open the repository's **Actions** tab.
2. Select **Build Android APK in the cloud**.
3. Press **Run workflow** and select the `main` branch.
4. Wait for the workflow to finish successfully.
5. Open that workflow run and download the **star-forge-android-debug** artifact.
6. Extract the downloaded ZIP and install the APK on your Android device. Android may ask you to allow installation from that source.

Every push to `main` also starts an APK build automatically. Build artifacts are retained for 14 days.

The workflow uses the official SDL3 Android project generator and builds the native C code with Android's NDK in the cloud. See the [SDL3 Android build documentation](https://wiki.libsdl.org/SDL3/README-android).

## Build the desktop prototype (optional)

A desktop build is optional. It is not needed to generate the Android APK.

Requires CMake 3.21+, a C17 compiler, Git, and network access on the first configure if SDL3 is fetched automatically.

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
    ctest --test-dir build --output-on-failure

Run the generated `star_forge` executable.

To use a system-installed SDL3 package, configure with `-DSTARFORGE_FETCH_SDL3=OFF`.

## Code formatting

C and header files use the repository's `.clang-format` rules. After installing clang-format, format all source files with:

    sh tools/format.sh

Keep functions, conditions, declarations, and data structures on separate readable lines. Do not compress implementation code into one-line blocks.

## Repository layout

- `src/` — SDL frontend and simulation
- `data/` — starter item and NPC role definitions
- `assets/` — sprites, audio, fonts, and other assets
- `docs/` — architecture, performance, design, and controls
- `tests/` — simulation tests
- `tools/` — developer utilities, including formatting
- `.github/workflows/` — CI and cloud build automation

## Not yet implemented

Touch controls, final sprite art, animation, save/load, FTL travel, multiple systems, dynamic markets, advanced NPC factions/reputation, and full ship fitting are future milestones.

See [Architecture](docs/architecture.md), [Performance](docs/performance.md), [Asset Pipeline](docs/asset-pipeline.md), and [Game Design Foundations](docs/game-design.md).
