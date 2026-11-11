# Star Forge asset library

All runtime art is stored as individually tracked source files under this directory. The original `StarForge_Asset_Pack.zip` at the repository root is retained as a pristine archive, not as the runtime asset source.

| Folder | Contents |
|---|---|
| `backgrounds/` | Nebulae, starfields and parallax backgrounds |
| `ships/` | Player, civilian, cargo, pirate and military ships |
| `stations/` | Stations and structures |
| `planets/` | Terrestrial planets, gas giants and moons |
| `asteroids/` | Asteroid variants, ore and wreckage |
| `salvage/` | Cargo containers and salvage item sprites |
| `projectiles/` | Laser and weapon projectile sprites |
| `effects/` | Thrusters, explosions and impacts |
| `ui/` | Joystick, button skins, panels and HUD icons |
| `audio/` | Sound effects and music |
| `fonts/` | Licensed fonts and bitmap fonts |

**Rules:** lowercase snake_case names, original PNG source art, no generated Godot import cache in Git, and explicit Android resource references in `scripts/autoload/asset_library.gd`.

See [the asset pipeline guide](../docs/asset-pipeline.md).
