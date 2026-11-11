# Gameplay Mechanics — Godot Implementation

This document distinguishes current prototype behavior from features that remain in the design backlog.

## Player flight

- Top-down 2D movement with a touch joystick.
- Ship facing turns at a rate-limited angular speed.
- Normal thrust acceleration: 190 world units/s².
- Boost acceleration: 340 world units/s².
- Normal speed cap: 190 world units/s.
- Boost speed cap: 270 world units/s.
- Drag is applied every physics step.
- Movement and boost do not consume ship energy.
- Asteroids, planets, moons and stations use physical collision bodies.

## Ship condition and power

- Player starts with 100 hull, 60 armour and 100 energy.
- Armour absorbs incoming damage before hull is affected.
- Damage beyond remaining armour spills into hull.
- Hull reaching zero destroys the ship and opens the recovery panel.
- Emergency recovery restores 70 hull, 30 armour and 100 energy.
- Energy regenerates at 12 units/s.
- Each player shot costs 8 energy.
- Weapons cannot fire without enough energy.

## Combat

- Player projectile speed: 360 world units/s.
- Player weapon cooldown: 0.22 seconds.
- Player projectile damage: 24.
- Pirate projectile speed: 230 world units/s.
- Pirate weapon cooldown: 0.85 seconds.
- Pirate projectile damage: 8.
- Projectiles expire after 1.8 seconds.
- A fixed pool of 64 projectile nodes is preallocated per active system; shots are reused instead of repeatedly allocated and freed.
- NPC armour and hull use the same armour-first damage rule.
- Destroyed NPC cargo is released as physical salvage.
- NPC destruction has a 35% chance to release one weapon-salvage item.

## NPC roles

- Cargo haulers navigate toward their system station.
- Pirates pursue a cargo hauler target and fire when in range.
- Patrol ships orbit the local station.
- NPC roles and ship designs are separate JSON definitions.
- NPC cargo is constrained by ship-design cargo capacity.
- Optional NPC salvage collection can be enabled from the cargo panel.

## Cargo and economy prototype

- Commodities: metal, fuel, food, medicine and weapon salvage.
- Ordinary commodities use one cargo-volume unit per item.
- Weapon salvage uses two cargo-volume units per item.
- Player cargo capacity: 100 volume units.
- Salvage exists as physical drops in the world.
- Player collection range: 38 world units.
- NPC collection requires free cargo capacity and a nearby drop.
- Player can jettison a selected quantity as a physical drop.
- Player can sell an item stack near the local station.
- Item prices and volume are loaded from `data/items.json`.

## Galaxy and FTL

- Four prototype systems: Solace, Vesper, Kestrel and Ironwake.
- Each system is a separate local scene-space.
- Ordinary interstellar travel is unavailable; an installed FTL drive is required.
- A jump requires a directly connected system and five fuel.
- Fuel is deducted only after a valid jump.
- Local NPC and cargo-drop state is saved when leaving a system.
- Returning to a system restores its saved NPCs and drops.
- System definitions and adjacency are data-driven in `data/systems.json`.

## Save and recovery

- Local save data is written under Godot's application user-data directory.
- Save files carry a schema version.
- Inventory, credits, current system, player condition and local system state are saved.
- Invalid system references and unsupported save versions are rejected.

## Planned, not yet implemented

- Planet and moon landing, orbit, surface interaction and resource extraction.
- Dynamic supply/demand markets for each planet and station.
- Full ship fitting, hardpoints, modules and equipment mass/power validation.
- Additional ship types and classes, ship acquisition and upgrades.
- Trader, miner, escort and dedicated fuel-hauler AI.
- Missions, factions, reputation and progression.
- Atomic backup rotation and recovery from interrupted writes.

## Engineering constraints

- Keep frequently updated actors bounded and avoid unbounded scene growth.
- Reuse projectiles and high-churn effects through pools as their count increases.
- Load only local-system content.
- Keep content IDs stable and validate data before use.
- Add regression tests whenever gameplay rules change.
- Profile memory and frame pacing on actual Android devices.
