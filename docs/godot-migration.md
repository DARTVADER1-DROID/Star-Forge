# Godot Migration

## Decision

The active development direction is Godot 4 with typed GDScript. The existing SDL3/C17 implementation is retained temporarily as a reference until the Godot port has feature parity and has passed device validation.

## Migration rules

- Do not delete or rewrite the original SDL implementation during the initial migration.
- Port mechanics from the design and gameplay documents; do not blindly translate old code.
- Keep gameplay systems modular and data-driven.
- Use typed GDScript, descriptive names, small focused scripts, and documented public APIs.
- Keep all gameplay input touch-only. Do not add keyboard or mouse gameplay bindings.
- Lock Android orientation to landscape.
- Use a 960 x 540 logical viewport with canvas-item stretching and responsive anchors.
- Start with the Compatibility renderer and profile on actual Android hardware.
- Keep the game offline-first.
- Use object pools for high-churn projectiles and effects before increasing entity counts.
- Never claim 1 GB device compatibility without testing on a genuine low-memory device.

## Initial vertical slice

The first Godot slice establishes:
- Android-oriented landscape project settings.
- A top-down player ship with touch joystick steering.
- Dedicated touch BOOST and FIRE buttons.
- Hull, armour and energy status display.
- Weapon energy consumption and passive energy regeneration.
- Projectile scene separation.
- Cloud-based Android export.

## Porting order

1. Input, viewport, camera and player flight.
2. Ship stats, armour, hull, energy, weapons and projectile pooling.
3. Asteroids and physical collision.
4. Cargo, salvage, station trading and inventory.
5. NPC ship definitions, roles and behavior.
6. Galaxy map, FTL fuel rules and per-system persistence.
7. Save/load, recovery, balancing and performance profiling.

## Future native optimisation

GDScript remains the default through feature completion. A later C/C++ rewrite is a separate project decision and must be justified by profiling and a stable gameplay specification.
