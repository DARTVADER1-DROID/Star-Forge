extends SceneTree

func _initialize() -> void:
    call_deferred("_run_smoke_test")

func _run_smoke_test() -> void:
    var main_scene := load("res://scenes/main.tscn") as PackedScene
    if main_scene == null:
        push_error("Main scene could not be loaded.")
        quit(1)
        return

    var game_root := main_scene.instantiate()
    root.add_child(game_root)
    await process_frame

    var player := game_root.get_node_or_null("PlayerShip")
    var hud := game_root.get_node_or_null("HUD")
    if player == null or hud == null:
        push_error("Main scene is missing the player ship or touch HUD.")
        game_root.queue_free()
        quit(1)
        return

    print("Star Forge Godot smoke test passed.")
    game_root.queue_free()
    quit(0)
