# Godot Tests

Automated validation runs without a display device.

- `godot --headless --editor --path . --import --quit` validates project import and resources.
- `godot --headless --path . --script res://tests/godot_smoke_test.gd` loads the main scene and validates required runtime nodes and content definitions.

Add deterministic tests for cargo capacity, armour damage spillover, FTL adjacency and fuel consumption, NPC role data, save-file validation and system-state restoration. Keep tests in GDScript and run them in CI.
