extends Node

func _ready() -> void:
    call_deferred("_run_tests")

func _run_tests() -> void:
    var failures: Array[String] = []
    _test_content_database(failures)
    _test_cargo_capacity(failures)
    _test_armour_damage(failures)
    _test_ftl_rules(failures)
    _test_save_round_trip(failures)
    _test_main_scene(failures)

    if failures.is_empty():
        print("All Star Forge Godot tests passed.")
        get_tree().quit(0)
        return

    for failure: String in failures:
        push_error(failure)
    get_tree().quit(1)

func _test_content_database(failures: Array[String]) -> void:
    if ContentDatabase.items.size() != 5:
        failures.append("Expected five item definitions.")
    if ContentDatabase.ship_designs.size() < 3:
        failures.append("Expected at least three ship designs.")
    if ContentDatabase.systems.size() != 4:
        failures.append("Expected four star systems.")

func _test_cargo_capacity(failures: Array[String]) -> void:
    GameState.inventory = {"metal":0,"fuel":0,"food":0,"medicine":0,"weapon_salvage":0}
    if not GameState.add_item("metal",98):
        failures.append("Cargo should accept a valid stack.")
    if GameState.add_item("weapon_salvage",2):
        failures.append("Cargo must reject capacity overflow.")
    if GameState.cargo_used() != 98:
        failures.append("Cargo volume calculation is incorrect.")

func _test_armour_damage(failures: Array[String]) -> void:
    GameState.player_ship_state.hull = 20.0
    GameState.player_ship_state.armour = 5.0
    GameState.apply_player_damage(8.0)
    if not is_equal_approx(float(GameState.player_ship_state.armour),0.0):
        failures.append("Armour must absorb damage first.")
    if not is_equal_approx(float(GameState.player_ship_state.hull),17.0):
        failures.append("Damage should spill into hull after armour.")

func _test_ftl_rules(failures: Array[String]) -> void:
    GameState.current_system_id = "solace"
    GameState.inventory.fuel = 5
    if not GameState.can_jump_to("vesper"):
        failures.append("Connected system should be reachable.")
    if GameState.can_jump_to("ironwake"):
        failures.append("Non-adjacent system must not be reachable.")
    if not GameState.jump_to_system("vesper"):
        failures.append("Valid FTL jump should succeed.")
    if int(GameState.inventory.fuel) != 0:
        failures.append("FTL jump must consume five fuel.")
    GameState.current_system_id = "solace"
    GameState.inventory.fuel = 25

func _test_save_round_trip(failures: Array[String]) -> void:
    GameState.credits = 321
    if not GameState.save_game():
        failures.append("Save operation failed.")
        return
    GameState.credits = 0
    if not GameState.load_game() or GameState.credits != 321:
        failures.append("Save/load did not restore credits.")
    GameState.credits = 250
    GameState.current_system_id = "solace"
    GameState.inventory = {"metal":0,"fuel":25,"food":0,"medicine":0,"weapon_salvage":0}

func _test_main_scene(failures: Array[String]) -> void:
    var packed := load("res://scenes/main.tscn") as PackedScene
    if packed == null:
        failures.append("Main scene failed to load.")
        return
    var game := packed.instantiate()
    add_child(game)
    await get_tree().process_frame
    for node_name: String in ["PlayerShip","HUD","Projectiles","NPCs","Obstacles","CargoDrops"]:
        if game.get_node_or_null(node_name) == null:
            failures.append("Main scene missing node: " + node_name)
    if game.get_node("NPCs").get_child_count() != 3:
        failures.append("Initial system should spawn three NPC roles.")
    game.queue_free()
