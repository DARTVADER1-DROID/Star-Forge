# Godot Project Standards

## Engine and platform

- Godot 4.7.2 stable.
- Typed GDScript for all gameplay and project tooling.
- Compatibility renderer with ETC2/ASTC texture import enabled for Android.
- Android arm64-v8a debug APK.
- Landscape orientation and a 960 × 540 logical canvas.
- Canvas-item scaling for varied screen aspect ratios.
- Touch-only gameplay controls.
- GitHub Actions for cloud validation and Android packaging.

## Implemented gameplay foundation

- Player steering, thrust, boost and weapon fire.
- Hull, armour and energy rules.
- NPC roles and dedicated ship-design data.
- Asteroid, planet, moon and station collision bodies.
- Projectile collision and damage.
- Cargo inventory, capacity, physical drops, collection, jettison and station sale.
- NPC salvage collection toggle and cargo manifests.
- Four star systems with adjacency-based FTL travel and fuel consumption.
- Versioned local save data, system-state persistence and emergency recovery.

## Repository standards

- Godot project files, scenes, scripts and resources are the source of truth.
- All gameplay scripts use typed GDScript.
- Reusable entities have dedicated scenes.
- Content is data-driven and validated at startup.
- UI and gameplay input are touch-first.
- Generated import caches and APKs are excluded from version control.

## Remaining completion work

- Complete persistent market and planetary state.
- Planet/moon landing, orbit and interaction mechanics.
- Full ship catalogue, fitting, progression, missions and factions.
- Production-ready art/audio integration.
- Automated edge-case and lifecycle tests.
- Real-device validation for touch, orientation, performance and memory.

A later native optimisation is a separate decision and must be justified by measured profiling after gameplay completion.
