# Godot Asset Pipeline

## Source of truth
- Source PNGs live in tracked `assets/`; the original ZIP is retained as an archive only.
- `assets/asset_manifest.json` inventories every source image.
- `scripts/autoload/asset_library.gd` explicitly preloads every manifest image for Android exports.
- Never use runtime directory scanning or ZIP extraction.
- Missing optional visuals use procedural fallbacks; required assets are checked in CI.

## Directory ownership
```text
assets/
  backgrounds/   nebula and parallax starfield layers
  ships/         player and NPC hull sprites
  stations/      orbital stations
  planets/       planets, moons, stars and orbit art
  asteroids/     asteroid variants, ore and wreckage
  salvage/       cargo crates and recoverable goods
  projectiles/   lasers and weapon projectiles
  effects/       thrusters, explosions, impacts and animation frames
  ui/            HUD controls, panels, icons and indicators
  audio/         future sound effects and music
data/
  economy/ ships/ world/ services/
```

## Runtime rules
- Pixel art uses nearest-neighbour filtering.
- Load textures through `AssetLibrary.load_texture(file_name)`.
- Audio features call `AudioManager.play_sound(sound_id)`; only AudioManager owns AudioStreamPlayer instances.
- Keep Compatibility renderer and Android ETC2/ASTC import settings.
