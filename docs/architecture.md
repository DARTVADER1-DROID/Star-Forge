# Godot Architecture

## Engine conventions

- Godot 4.7.2 is the single engine runtime.
- Typed GDScript is the primary and only gameplay language.
- Gameplay is scene-oriented; reusable actors and world objects have dedicated scenes.
- Global state is restricted to the `GameState` and `ContentDatabase` autoloads.
- Content definitions are JSON with stable IDs and are loaded once by the content database.
- Touch input is handled by the HUD; player simulation receives normalized steering and action signals.
- Rendering uses Godot's Compatibility renderer to target a broad Android device range.

## Runtime layers

### Application and UI
`scenes/main.tscn` composes the world, player, entity containers and HUD. The HUD owns the touch joystick, action buttons, cargo panel, galaxy selector and recovery overlay.

### Persistent state
`scripts/autoload/game_state.gd` owns inventory, credits, ship state, FTL rules, system state and versioned local save data. It exposes signals rather than reaching into scene nodes.

### Content
`scripts/autoload/content_database.gd` validates and indexes JSON definitions for items, ship designs, NPC roles and star systems.

### Gameplay
- Player ship: movement, acceleration, armour, hull, energy and weapon firing.
- NPC ship: role-specific navigation, cargo manifest, combat and salvage.
- Projectile: lifetime, travel and collision-driven damage.
- World: system loading, asteroid and planetary obstacles, NPC population and salvage persistence.

### Data and assets
Definitions belong in `data/`; reusable scenes and scripts belong in `scenes/` and `scripts/`. Art is imported as Godot resources and referenced through stable resource paths.

## Code standards

- Use typed variables, parameters and return types for public APIs.
- Keep scripts focused on one responsibility.
- Use signals for cross-layer communication.
- Validate external data before indexing or applying it.
- Avoid per-frame node creation and unbounded collections.
- Keep gameplay input touch-only.
- Do not introduce native extensions until profiling demonstrates a concrete need.

## Runtime flow

1. Godot loads the project and autoloads.
2. The content database validates JSON data.
3. The main scene restores local player state and loads the current system.
4. The player and local actors run in the active scene only.
5. System transitions persist local state and consume FTL fuel.
6. Saves are written to the application user-data directory.
