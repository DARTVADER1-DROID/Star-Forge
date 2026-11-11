# Development Roadmap

## Godot foundation
- [x] Godot project and landscape Android configuration
- [x] Compatibility renderer
- [x] Typed GDScript module layout
- [x] Touch joystick, BOOST and FIRE
- [x] Player hull, armour, energy and weapon cost
- [x] NPC role and ship-design data
- [x] Four-system FTL graph and fuel cost
- [x] Cargo inventory, drops and station sale foundations
- [x] Local save/load foundation
- [x] Godot headless smoke test
- [ ] Full physical-device validation and tuning

## Gameplay completion
- [ ] Persistent full entity state per star system (NPCs, drops, asteroids and market)
- [ ] Planet and moon interaction, landing and orbit mechanics
- [ ] Dedicated ship type/class matrix and fitting UI
- [ ] Complete commodity markets with local supply/demand
- [ ] Missions, factions and reputation
- [ ] Ship acquisition, upgrades and equipment
- [ ] NPC traders, miners, escorts and fuel haulers
- [ ] Audio, effects, menus and onboarding
- [ ] Atomic save backups and recovery testing

## Quality gates
- Godot headless import and smoke test pass.
- Android export completes and APK artifact is verified.
- Test touch controls at multiple aspect ratios.
- Test pause/resume, app backgrounding and interrupted saves.
- Profile memory, frame pacing, thermal behavior and battery use on physical Android hardware.
- Do not claim compatibility with 1 GB devices without a genuine low-memory device test.
