# Star Forge Architecture

## Product constraints
- Godot 4.7.2, typed GDScript, Android-only landscape and touch-only gameplay.
- Compatibility renderer, low-memory Android target, no keyboard/mouse gameplay.
- Keep the development PR unmerged until explicitly approved.

## Runtime ownership
- `scenes/main.tscn`: composition root for world, player, follow camera, parallax background, entity containers, projectile pool and HUD canvas.
- `scripts/player/`: player movement, ship state and weapons.
- `scripts/npc/`: NPC roles, docking and combat decisions.
- `scripts/world/`: world entities and focused world services.
- `scripts/combat/`: projectiles and bounded pooling.
- `scripts/ui/`: touch HUD, galaxy navigation and local system maps.
- `scripts/audio/`: shared sound-effect playback service.
- `scripts/autoload/`: persistent state, content indexes and Android asset registry.

## SOLID boundaries
- `SystemLayout` is the shared data adapter for celestial bodies, asteroid placements and station coordinates. Galaxy/system views and runtime scene construction consume the same definitions.
- `SystemContentBuilder` builds static system bodies from data; it does not own HUD, saves or NPC AI.
- `ContentDatabase.validate_world_catalog()` checks system links, stable body IDs and referenced texture registrations before release.
- `WorldInteractionService` finds the nearest eligible planet/station interaction; it does not create UI or mutate player state.
- `StarForgeWorld` coordinates system lifecycle and connects signals.
- `NPCShip` owns movement, docking and combat eligibility. Docked traffic is stationary and protected from pirate targeting.
- UI emits intent signals (LAND, DOCK, REFUEL, JUMP); world validates range and performs actions.
- Use stable IDs, not display names or hard-coded scene references.

## Content boundaries
- `data/economy/items.json`: commodities and equipment.
- `data/ships/designs.json`: hull definitions.
- `data/ships/roles.json`: NPC roles and manifests.
- `data/world/systems.json`: system graph and configuration.
- `data/world/planets/`, `data/world/maps/`, `data/services/`: future modular catalogs.
- Definitions reference stable IDs; do not duplicate mutable gameplay stats.

## Galaxy navigation
The galaxy map supports touch drag to pan, pinch to zoom, and tap to select any known system for inspection/planning. FTL remains restricted to adjacent systems with sufficient fuel; selecting a distant system never bypasses route rules.

## Assets and audio
Every manifest texture is explicitly preloaded by `AssetLibrary` for Android PCK inclusion. CI compares the manifest with the registry. Optional missing textures use procedural fallbacks.
`AudioManager` owns a bounded eight-player SFX pool, sound-ID registration and global volume/mute settings. Feature modules request sound IDs; they do not create unmanaged players. The current source pack has no audio clips, so the manager is ready for later assets but does not invent placeholder sounds.

## Persistence and tests
`GameState` owns inventory, credits, FTL, per-system state and versioned saves. Save stable IDs and numeric state, never scene references. CI imports the project, runs gameplay tests and exports an Android debug APK.
