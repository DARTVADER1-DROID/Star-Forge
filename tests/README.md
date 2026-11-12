# Godot Tests

CI runs the test runner as a regular Godot scene so project autoloads and runtime services are initialized exactly as they are in the game.

Command:

`godot --headless --path . res://tests/test_runner.tscn`

Coverage includes:
- Content database loading and schema counts.
- Cargo capacity and item volume.
- Armour-first damage with hull spillover.
- FTL adjacency, fuel checks and fuel consumption.
- Local save/load round-trip.
- Main scene composition and initial NPC population.

Extend this runner with deterministic regression cases whenever gameplay rules change.
