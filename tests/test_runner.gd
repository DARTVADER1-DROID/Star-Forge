extends Node

func _ready() -> void:
    call_deferred("_run_tests")

func _run_tests() -> void:
    var failures: Array[String] = []
    _test_content_database(failures)
    _test_asset_library(failures)
    _test_asset_manifest(failures)
    _test_cargo_capacity(failures)
    _test_fuel_purchase(failures)
    _test_armour_damage(failures)
    _test_ftl_rules(failures)
    _test_map_graph(failures)
    _test_system_map(failures)
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

func _test_asset_library(failures: Array[String]) -> void:
    if not AssetLibrary.has_asset("ship_player_runner.png"):
        failures.append("Player ship sprite is missing from the imported asset pack.")
    if not AssetLibrary.has_asset("bg_nebula_320x180.png"):
        failures.append("Nebula background is missing from the imported asset pack.")

func _test_asset_manifest(failures: Array[String]) -> void:
    var file := FileAccess.open("res://assets/asset_manifest.json",FileAccess.READ)
    if file == null:
        failures.append("Asset manifest is missing.")
        return
    var parsed: Variant = JSON.parse_string(file.get_as_text())
    if not parsed is Dictionary or int(parsed.get("asset_count",0)) < 100:
        failures.append("Asset manifest should index the full source art library.")
        return
    for entry: Variant in parsed.get("assets",[]):
        if not entry is Dictionary or not ResourceLoader.exists("res://"+str(entry.get("path",""))):
            failures.append("Manifest references a missing asset: " + str(entry.get("path","invalid")))

func _test_cargo_capacity(failures: Array[String]) -> void:
    GameState.inventory = {"metal":0,"fuel":0,"food":0,"medicine":0,"weapon_salvage":0}
    if not GameState.add_item("metal",98):
        failures.append("Cargo should accept a valid stack.")
    if GameState.add_item("weapon_salvage",2):
        failures.append("Cargo must reject capacity overflow.")
    if GameState.cargo_used() != 98:
        failures.append("Cargo volume calculation is incorrect.")

func _test_fuel_purchase(failures: Array[String]) -> void:
    GameState.inventory = {"metal":0,"fuel":25,"food":0,"medicine":0,"weapon_salvage":0}
    GameState.credits = 250
    if not GameState.buy_fuel(10):
        failures.append("Planet refuelling should sell fuel when credits and cargo space are available.")
    if int(GameState.inventory.fuel) != 35 or GameState.credits != 110:
        failures.append("Fuel purchase must update inventory and credits at the catalog price.")
    GameState.inventory = {"metal":0,"fuel":25,"food":0,"medicine":0,"weapon_salvage":0}
    GameState.credits = 250

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

func _test_map_graph(failures: Array[String]) -> void:
    var map_script: Script = load("res://scripts/ui/galaxy_map.gd")
    if map_script == null:
        failures.append("Galaxy map script is missing.")
        return
    var map := map_script.new() as Control
    map.size = Vector2(460,220)
    map.set("systems",ContentDatabase.systems)
    map.call("configure","solace","vesper")
    var solace_point: Vector2 = map.call("_system_point","solace")
    if not Rect2(Vector2.ZERO,map.size).has_point(solace_point):
        failures.append("Galaxy map system node is outside its map viewport.")
    var solace: Dictionary = ContentDatabase.get_system("solace")
    if not Array(solace.get("links",[])).has("vesper") or Array(solace.get("links",[])).has("ironwake"):
        failures.append("Galaxy map graph adjacency does not match FTL routing.")
    map.free()

func _test_system_map(failures: Array[String]) -> void:
    var map_script: Script = load("res://scripts/ui/system_map.gd")
    if map_script == null:
        failures.append("System map script is missing.")
        return
    var map := map_script.new() as Control
    map.call("configure","solace",ContentDatabase.get_system("solace"),Vector2(480,270))
    var body_points: Dictionary = map.get("_body_points")
    if body_points.size() != 9:
        failures.append("System map should expose the station, three planets and five moons in Solace.")
    if not body_points.has("STATION") or not body_points.has("PLANET 1"):
        failures.append("System map is missing expected celestial body markers.")
    map.free()

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
    for node_name: String in ["PlayerShip","HUDLayer","HUDLayer/HUD","ProjectilePool","NPCs","Obstacles","CargoDrops"]:
        if game.get_node_or_null(node_name) == null:
            failures.append("Main scene missing node: " + node_name)
    var hud := game.get_node("HUDLayer/HUD") as TouchHUD
    if hud == null:
        failures.append("HUD must be a dedicated Control on its own CanvasLayer.")
    else:
        if not hud.is_visible_in_tree() or hud.size.x <= 0.0 or hud.size.y <= 0.0:
            failures.append("HUD must fill the viewport and remain visible in the UI canvas.")
    var ftl_button := game.get_node("HUDLayer/HUD/FTLButton") as Button
    if ftl_button == null:
        failures.append("HUD must create a dedicated FTL button.")
    else:
        if ftl_button.text != "FTL" or not ftl_button.visible:
            failures.append("FTL button must be clearly labelled and visible.")
        if not is_equal_approx(ftl_button.anchor_top,1.0) or not is_equal_approx(ftl_button.anchor_right,1.0):
            failures.append("FTL button must stay anchored in the lower-right flight-control cluster.")
        var ftl_rect := ftl_button.get_global_rect()
        if ftl_rect.size.x < 80.0 or ftl_rect.size.y < 50.0 or not Rect2(Vector2.ZERO,get_viewport_rect().size).encloses(ftl_rect):
            failures.append("FTL button must have a usable size and remain inside the viewport.")
    if hud != null:
        for button_name: String in ["BOOSTButton","FIREButton","LOOTButton","FTLButton","BAGButton","SAVEButton"]:
            var flight_button := hud.get_node_or_null(button_name) as Button
            if flight_button == null or not flight_button.visible:
                failures.append("Touch HUD is missing a visible flight/action button: " + button_name)
    var pool := game.get_node("ProjectilePool") as ProjectilePool
    var player := game.get_node("PlayerShip") as PlayerShip
    var camera := game.get_node("PlayerShip/Camera2D") as Camera2D
    if camera == null or not camera.enabled:
        failures.append("Player ship must own an enabled follow camera.")
    if player.collision_layer != 2 or player.collision_mask != 0:
        failures.append("Player ship must use the non-blocking player collision layer.")
    if hud != null:
        var land_button := hud.get_node_or_null("LANDButton") as Button
        if land_button == null or land_button.visible:
            failures.append("Contextual LAND button must exist and start hidden.")
        if hud.get_node_or_null("PlanetServicesPanel") == null:
            failures.append("Planetary service panel must be registered in the HUD.")
    if pool.get_child_count() != 64:
        failures.append("Projectile pool should preallocate 64 slots.")
    player.energy = 100.0
    player._fire_weapon()
    if pool.get_active_count() != 1:
        failures.append("Player weapon should activate one pooled projectile.")
    if not is_equal_approx(player.energy,92.0):
        failures.append("Player weapon should consume eight energy per shot.")
    pool.clear_active()
    if pool.get_active_count() != 0:
        failures.append("Projectile pool should deactivate all active shots.")
    if game.get_node("NPCs").get_child_count() != 3:
        failures.append("Initial system should spawn three NPC roles.")
    game.queue_free()
