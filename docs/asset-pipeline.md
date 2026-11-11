# Godot Asset Pipeline

## Source-of-truth policy

- Individual source assets live in the tracked `assets/` tree; the ZIP is retained only as an original source archive/recovery copy.
- Do not make a build depend on runtime ZIP extraction. Godot must import the tracked source files directly.
- Never commit generated `.godot/` import cache files.
- Keep gameplay definitions in `data/`, separate from artwork.
- Use explicit `preload()` references in `scripts/autoload/asset_library.gd` for assets needed by Android exports. Do not discover exported textures by scanning loose source directories at runtime.

## Directory ownership

```text
assets/
  backgrounds/   nebulae, starfields, parallax layers
  ships/         player and NPC hull sprites
  stations/      stations and dock structures
  planets/       terrestrial, gas giant and moon sprites
  asteroids/     asteroid variants, ore and wreckage
  salvage/       cargo crates and recoverable goods
  projectiles/   lasers and other weapon projectiles
  effects/       thrusters, explosions, impacts
  ui/            HUD controls, panels, icons and indicators
  audio/         sound effects and music
  fonts/         bitmap or licensed fonts
data/
  items.json
  ship_designs.json
  npc_roles.json
  systems.json
```

## Import and naming rules

- Keep runtime pixel art as PNG and use nearest-neighbor filtering.
- Use lowercase snake_case filenames; do not encode mutable gameplay stats in filenames.
- Keep original editable Aseprite/Pixelorama projects in a clearly named `source/` subfolder when available.
- Preserve source dimensions and transparent padding; configure pivots in Godot scenes.
- Use the Compatibility renderer and Android ETC2/ASTC import settings.
- Prefer appropriately sized textures; do not load unrelated system art into active scenes.

## Validation

The CI workflow extracts the original archive only as a migration/bootstrap step, tracks the extracted files on the development branch, imports the tracked assets, and runs the scene/gameplay checks. Once the extracted source tree is committed, normal editor and build workflows use `assets/` directly.
