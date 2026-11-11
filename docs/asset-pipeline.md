# Godot Asset Pipeline

## Import
- Keep source artwork in `assets/` and use PNG for runtime pixel art.
- Godot imports source files into its generated resource cache; never commit the generated `.godot/` directory.
- The cloud APK workflow extracts the archived art pack into `assets/` before Godot imports resources. The JSON files already in `data/` remain the authoritative gameplay definitions.
- Prefer nearest-neighbor filtering for pixel art.
- Use the Compatibility renderer and ETC2/ASTC texture compression for Android exports.
- Keep editable Pixelorama/Aseprite files alongside runtime exports where practical.

## Structure
```text
assets/
  ships/
  weapons/
  projectiles/
  effects/
  asteroids/
  stations/
  backgrounds/
  ui/
  audio/
  fonts/
data/
  items.json
  ship_designs.json
  npc_roles.json
  systems.json
```

## Sprite conventions
- Source origin is top-left; X increases right and Y increases down.
- Configure pivots in the imported Sprite2D or scene, not by changing source artwork.
- Ship artwork should face up unless the asset explicitly defines another orientation.
- Use stable lowercase snake_case filenames and stable content IDs.
- Atlas related small sprites when it reduces texture switches without making updates difficult.

## Memory
- Prefer appropriately sized textures over large source sheets at runtime.
- Avoid loading unused art in every scene.
- Release system-specific resources when leaving a system.
- Profile texture memory and compression on actual Android devices.
