# Godot-only Migration

## Migration status

The active repository runtime is Godot 4.7.2 with typed GDScript. The previous native implementation, its CMake configuration, C source, C headers, native image decoder, C tests and C-specific formatting tool have been removed from the active branch.

## Platform and engine decisions

- Godot 4.7.2 stable.
- Typed GDScript for all gameplay and tooling.
- Compatibility renderer with ETC2/ASTC texture import enabled for Android.
- Android arm64-v8a debug APK.
- Landscape orientation and a 960 × 540 logical canvas.
- Canvas-item scaling for varied screen aspect ratios.
- Touch-only gameplay controls.
- GitHub Actions for cloud validation and Android packaging.

## Ported gameplay foundation

- Player steering, thrust, boost and weapon fire.
- Hull, armour and energy rules.
- NPC roles and dedicated ship-design data.
- Asteroid and planetary collision bodies.
- Projectile collision and damage.
- Cargo inventory, capacity, physical drops, collection and station sale.
- Four star systems with adjacency-based FTL travel and fuel consumption.
- Versioned local save data and emergency ship recovery.

## Repository standards

- No C/C++ source, headers, CMake, SDL or native build scripts.
- All gameplay scripts use typed GDScript.
- Reusable entities have dedicated scenes.
- Content is data-driven and validated at startup.
- UI and gameplay input are touch-first.
- Generated Godot import caches and APKs are excluded from version control.

## Remaining completion work

- Full entity persistence for every system, including NPCs and markets.
- Complete planet/moon interactions and landing.
- Full ship catalogue, fitting, progression, missions and factions.
- Production-ready art/audio integration.
- Automated edge-case and lifecycle tests.
- Real-device validation for touch, orientation, performance and memory.

A future native rewrite is a separate decision and must be justified by measured profiling after gameplay completion.
