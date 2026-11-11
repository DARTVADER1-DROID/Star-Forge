extends SceneTree

func _initialize() -> void:
    call_deferred("_run")

func _run() -> void:
    var packed := load("res://scenes/main.tscn") as PackedScene
    if packed == null:
        push_error("Main scene failed to load.")
        quit(1)
        return
    var game := packed.instantiate()
    root.add_child(game)
    await process_frame
    for node_name: String in ["PlayerShip","HUD","Projectiles","NPCs","Obstacles","CargoDrops"]:
        if game.get_node_or_null(node_name) == null:
            push_error("Main scene missing node: " + node_name)
            quit(1)
            return
    if ContentDatabase.systems.size() != 4:
        push_error("Expected four configured systems.")
        quit(1)
        return
    if not GameState.can_jump_to("vesper"):
        push_error("Starter ship should be able to reach Vesper.")
        quit(1)
        return
    print("Star Forge scene/content smoke test passed.")
    game.queue_free()
    quit(0)
