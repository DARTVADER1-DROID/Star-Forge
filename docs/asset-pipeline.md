# Asset Pipeline

## Source and runtime assets

- Export runtime sprites as PNG with transparency where appropriate.
- Keep editable Pixelorama/Aseprite project files alongside source art when practical.
- Do not commit generated build output.
- Use descriptive, stable filenames in lowercase snake_case.

## Suggested directories

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
  ships/
  weapons/
  items/
  missions/
```

## Sprite coordinate convention

- Sprite pixel origin is the top-left corner: (0, 0).
- X increases to the right.
- Y increases downward.
- A sprite's world position refers to its configured pivot.
- Store pivots and attachment points in sprite-local pixel coordinates relative to the top-left, then convert them to pivot-relative offsets in the engine.
- Ship artwork should normally face up in its source image unless a specific asset says otherwise.

## Example ship definition

```json
{
  "id": "player_ship",
  "sprite": "ships/player_ship.png",
  "size": [64, 64],
  "pivot": [32, 32],
  "hardpoints": [
    { "id": "left_gun", "position": [25, 42] },
    { "id": "right_gun", "position": [39, 42] }
  ]
}
```

The coordinates above are illustrative only. Actual coordinates must be measured from the finished sprite.

## Naming

Use stable IDs for content, such as `player_ship`, `light_laser`, and `asteroid_small`. Avoid using display names as internal identifiers.
