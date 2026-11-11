# Development Roadmap

## Milestone 0 — Repository foundation
- [x] Establish project goals and technology direction
- [x] Add initial CMake and SDL3 application shell
- [ ] Validate desktop build
- [ ] Add Android Gradle/NDK packaging and CI APK artifact

## Milestone 1 — Engine basics
- [ ] Logging and configuration
- [ ] Input abstraction, including touch controls
- [ ] Fixed-step simulation clock
- [ ] Memory arenas and bounded object pools
- [ ] Resource handles and asset loading

## Milestone 2 — First playable space scene
- [ ] Pixel-art camera and renderer
- [ ] Player ship movement
- [ ] Asteroid field
- [ ] Collision detection
- [ ] Weapon and projectile pool
- [ ] Basic enemy behavior

## Milestone 3 — Sandbox foundations
- [ ] Sector representation and streaming
- [ ] Stations and docking
- [ ] Ship equipment and hardpoints
- [ ] Cargo and trading
- [ ] Local save/load

## Milestone 4 — RPG systems
- [ ] Missions
- [ ] Factions and reputation
- [ ] Progression and equipment economy
- [ ] More ships, weapons, enemies, and locations

## Quality gates
- Profile memory and frame pacing on low-end Android hardware at every milestone.
- Test offline behavior.
- Test pause/resume and interrupted saves.
- Keep source readable and documented.
