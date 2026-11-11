# Gameplay Mechanics — Prototype Rules

This document records the current implemented rules and the intended direction for the next playable iteration. It distinguishes implemented behavior from design proposals so that future changes do not accidentally treat balancing placeholders as final.

## Implemented in the current prototype

### Player flight

- Ships use a top-down 2D coordinate system.
- The ship's facing angle determines its thrust direction.
- Turning is currently rate-limited to 2.8 radians per second.
- Normal thrust acceleration is 190 world units per second squared.
- Reverse thrust acceleration is 123.5 world units per second squared.
- Normal speed is capped at 190 world units per second.
- Linear drag is applied each simulation step.
- Asteroid collisions push ships outside the collision radius and damp/reverse their velocity.

### Boost and movement power

- Boost is held while the boost input is active.
- Boost acceleration is 340 world units per second squared.
- Boost speed cap is 270 world units per second.
- Normal acceleration is 190 world units per second squared.
- Normal speed cap is 190 world units per second.
- Turning, thrust, reverse thrust and boost do not consume ship energy.

These are prototype balance values and should be tuned through playtesting.

### Armour, hull and energy

- Armour absorbs incoming damage before hull is affected.
- Damage exceeding remaining armour spills into hull.
- A ship is destroyed when hull reaches zero.
- The player starts with 60 armour and 100 hull.
- The player starts with 100 energy.
- Each player weapon shot costs 8 energy.
- Weapons cannot fire without sufficient energy.
- Energy regenerates at 12 units per second.
- NPC weapons also consume energy and NPC energy regenerates at the same prototype rate.
- Movement does not consume energy.

### Combat

- Player projectiles travel at 360 world units per second.
- Player weapon cooldown is 0.22 seconds.
- Player projectile damage is 24.
- Pirate projectiles travel at 230 world units per second.
- Pirate weapon cooldown is 0.85 seconds.
- Enemy projectile damage to the player is 8.
- Projectiles are stored in a fixed-capacity pool.
- Destroyed NPCs release their remaining cargo as physical salvage.
- A destroyed NPC has a 35% chance to drop one weapon salvage item.

### NPC roles

- Cargo haulers navigate toward the station.
- Pirates pursue the cargo hauler while it is alive, then target the player.
- Pirates maintain a preferred combat distance and fire when the target is within range.
- NPC salvage collection can be enabled or disabled.
- More dedicated roles (trader, miner, patrol, escort) are planned, not yet implemented.

### Cargo and salvage

- Commodity types currently include metal, fuel, food, medicine and weapon salvage.
- Ordinary commodities consume one cargo volume unit per item.
- Weapon salvage consumes two cargo volume units per item.
- Player cargo capacity is currently 100 volume units.
- Salvage exists as physical drops in the world.
- The player collects the nearest eligible drop within 38 world units.
- NPCs may collect nearby salvage when NPC salvage behavior is enabled.
- The player can jettison a selected quantity of ordinary commodities.
- The player can sell all of one selected commodity when within 90 world units of the station.

## Proposed mechanics — not implemented yet

### Ship fitting

- Hull archetypes define hardpoints, allowed mount sizes, mount types and equipment mass limits.
- Weapons are installed into compatible hardpoints.
- A weapon must pass both hardpoint compatibility and equipment-mass checks.
- Power generation and consumption will be checked separately from mass.
- Installed weapon mass and cargo mass must not be conflated.

### Economy

- Each planet will eventually have its own supply, demand and market prices.
- Price changes should be driven by local supply and demand, not arbitrary random price rolls.
- Fuel remains a tradeable commodity and is also consumed by FTL travel.
- A station market should expose buy and sell prices separately.
- Dynamic market state must be saved locally.

### Star systems and FTL

- Each star system is a separate local room with its own local coordinates.
- The prototype defines four systems: Solace, Vesper, Kestrel and Ironwake.
- Each system has its own planet and moon counts and station location.
- Some systems contain asteroid fields; other systems do not.
- Asteroids are environmental obstacles for now; mining is a future feature.
- Inter-system travel is only possible through an installed FTL drive.
- The prototype FTL jump consumes 5 units of the Fuel commodity.
- A jump is rejected when the ship lacks an FTL drive or sufficient fuel.
- Jumping resets local velocity and position and clears local NPCs, projectiles and salvage.
- Directly flying to the edge of a system never transitions to another system.
- Future fleet travel will calculate fuel from fleet mass and route distance; current prototype cost is fixed.

### Salvage ownership and NPC behavior

- A physical salvage drop remains available until collected, destroyed or expired by an explicitly defined rule.
- NPC collection should require sufficient free cargo volume.
- NPCs should not collect hostile or inaccessible salvage without an appropriate role/behavior.
- Player collection should report capacity failures rather than silently discarding cargo.

## Engineering constraints

- Keep simulation independent of SDL and Android APIs.
- Keep all frequently updated entities in fixed-capacity pools until profiling justifies another storage model.
- Avoid heap allocation in the per-frame simulation loop.
- Keep tuning values centralized and documented.
- Add deterministic tests for every new economic or combat rule.
