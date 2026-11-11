extends Node2D
class_name StarForgeWorld

const STAR_COUNT := 110
const ASTEROID_SCENE: PackedScene = preload("res://scenes/asteroid.tscn")
const PLANET_SCENE: PackedScene = preload("res://scenes/planet_body.tscn")
const NPC_SCENE: PackedScene = preload("res://scenes/npc_ship.tscn")
const STATION_SCENE: PackedScene = preload("res://scenes/station.tscn")
const DROP_SCENE: PackedScene = preload("res://scenes/cargo_drop.tscn")

@onready var player: PlayerShip = $PlayerShip
@onready var hud: TouchHUD = $HUDLayer/HUD
@onready var projectile_pool: ProjectilePool = $ProjectilePool
@onready var npc_container: Node2D = $NPCs
@onready var obstacle_container: Node2D = $Obstacles
@onready var drop_container: Node2D = $CargoDrops

var _stars := PackedVector2Array()
var _system_id := ""
var npc_salvage_enabled := false
var _background_sprite: Sprite2D

func _ready() -> void:
    get_viewport().size_changed.connect(queue_redraw)
    _create_star_field()
    var background_texture := AssetLibrary.load_texture("bg_nebula_320x180.png")
    if background_texture != null:
        _background_sprite = Sprite2D.new()
        _background_sprite.texture = background_texture
        _background_sprite.position = Vector2.ZERO
        _background_sprite.scale = Vector2(24.0,24.0)
        _background_sprite.z_index = -1
        _background_sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
        add_child(_background_sprite)
    player.projectile_pool = projectile_pool
    hud.steering_changed.connect(player.set_steering)
    hud.boost_changed.connect(player.set_boosting)
    hud.fire_changed.connect(player.set_firing)
    hud.action_requested.connect(_on_hud_action)
    GameState.system_changed.connect(_load_system)
    GameState.player_destroyed.connect(hud.show_recovery_panel)
    GameState.load_game()
    _apply_player_state()
    _load_system(GameState.current_system_id)

func _process(_delta: float) -> void:
    _update_player_state()
    var station_data: Array = ContentDatabase.get_system(_system_id).get("station_position",[480,270])
    var station_position := Vector2(float(station_data[0]),float(station_data[1]))
    var station_distance := player.global_position.distance_to(station_position)
    hud.update_ship_status(player.hull,player.max_hull,player.armour,player.max_armour,player.energy,player.max_energy,GameState.credits,GameState.inventory,int(GameState.inventory.get("fuel",0)),str(ContentDatabase.get_system(_system_id).get("name",_system_id)),station_distance)
    _update_planet_proximity()
    hud.update_system_map(player.position)

func _update_planet_proximity() -> void:
    var nearest_id := ""
    var nearest_name := ""
    var nearest_distance := INF
    for node: Node in obstacle_container.get_children():
        if not node is PlanetBody:
            continue
        var planet := node as PlanetBody
        var distance := player.global_position.distance_to(planet.global_position)
        if distance <= planet.interaction_radius and distance < nearest_distance:
            nearest_distance = distance
            nearest_id = planet.body_id
            nearest_name = planet.body_name
    hud.set_nearby_planet(nearest_id,nearest_name)

func _draw() -> void:
    for star: Vector2 in _stars:
        draw_circle(star, 1.0, Color(0.55,0.67,0.86,0.72))

func _create_star_field() -> void:
    var random := RandomNumberGenerator.new()
    random.seed = 73421
    # Stars are distributed around the system origin so the camera can travel
    # without exposing an empty screen immediately outside the starting view.
    for _i in range(STAR_COUNT):
        _stars.append(Vector2(random.randf_range(-6000.0,6000.0),random.randf_range(-6000.0,6000.0)))

func _apply_player_state() -> void:
    var saved: Dictionary = GameState.player_ship_state
    player.position = Vector2(float(saved.position_x),float(saved.position_y))
    player.velocity = Vector2(float(saved.velocity_x),float(saved.velocity_y))
    player.hull = float(saved.hull)
    player.armour = float(saved.armour)
    player.energy = float(saved.energy)

func _update_player_state() -> void:
    GameState.player_ship_state.position_x = player.position.x
    GameState.player_ship_state.position_y = player.position.y
    GameState.player_ship_state.velocity_x = player.velocity.x
    GameState.player_ship_state.velocity_y = player.velocity.y
    GameState.player_ship_state.hull = player.hull
    GameState.player_ship_state.armour = player.armour
    GameState.player_ship_state.energy = player.energy

func _load_system(id: String) -> void:
    if not ContentDatabase.systems.has(id):
        push_error("Unknown system: " + id)
        return
    if not _system_id.is_empty():
        _save_system_state()
    _system_id = id
    projectile_pool.clear_active()
    for child: Node in npc_container.get_children():
        child.queue_free()
    for child: Node in obstacle_container.get_children():
        child.queue_free()
    for child: Node in drop_container.get_children():
        child.queue_free()

    var definition := ContentDatabase.get_system(id)
    var station_data: Array = definition.get("station_position",[480,270])
    var station := Vector2(float(station_data[0]),float(station_data[1]))
    if bool(definition.get("has_asteroids",false)):
        var asteroid_positions: Array[Vector2] = [Vector2(520,250),Vector2(680,420),Vector2(860,310),Vector2(740,160)]
        for point: Vector2 in asteroid_positions:
            var asteroid := ASTEROID_SCENE.instantiate()
            obstacle_container.add_child(asteroid)
            asteroid.position = point

    var planets := int(definition.get("planet_count",2))
    var moon_count := int(definition.get("moon_count",0))
    var planet_positions: Array[Vector2] = []
    for index in range(planets):
        var planet_position := Vector2(150.0 + float(index)*280.0,100.0 + float(index%2)*330.0)
        planet_positions.append(planet_position)
        var body := PLANET_SCENE.instantiate() as PlanetBody
        body.position = planet_position
        body.body_id = "%s_planet_%02d" % [_system_id,index+1]
        body.body_name = "%s %d" % [str(definition.get("name",_system_id)),index+1]
        body.body_radius = 30.0 + float(index%3)*12.0
        body.interaction_radius = body.body_radius + 76.0
        obstacle_container.add_child(body)
    for moon_index in range(moon_count):
        if planet_positions.is_empty():
            break
        var parent_position: Vector2 = planet_positions[moon_index % planet_positions.size()]
        var orbit_angle := float(moon_index) * 2.399
        var moon := PLANET_SCENE.instantiate() as PlanetBody
        moon.position = parent_position + Vector2(cos(orbit_angle),sin(orbit_angle))*76.0
        moon.body_id = "%s_moon_%02d" % [_system_id,moon_index+1]
        moon.body_name = "%s Moon %d" % [str(definition.get("name",_system_id)),moon_index+1]
        moon.body_radius = 11.0 + float(moon_index % 3)*3.0
        moon.interaction_radius = moon.body_radius + 76.0
        moon.body_color = Color(0.48,0.52,0.60)
        obstacle_container.add_child(moon)

    var station_body := STATION_SCENE.instantiate()
    obstacle_container.add_child(station_body)
    station_body.position = station
    _restore_or_spawn_npcs(station)
    _restore_system_drops()

func _find_planet(planet_id: String) -> PlanetBody:
    for node: Node in obstacle_container.get_children():
        if node is PlanetBody and (node as PlanetBody).body_id == planet_id:
            return node as PlanetBody
    return null

func _restore_or_spawn_npcs(station: Vector2) -> void:
    var system_state: Dictionary = GameState.system_states.get(_system_id,{})
    var saved: Array = system_state.get("npcs",[])
    if not bool(system_state.get("initialized",false)):
        var hauler := _spawn_npc("cargo_hauler","starter_freighter",station+Vector2(160,40),station)
        var pirate := _spawn_npc("pirate","starter_interceptor",station+Vector2(-160,-40),station)
        pirate.target = hauler
        _spawn_npc("patrol","starter_patrol",station+Vector2(0,150),station)
        return

    var restored: Array[NPCShip] = []
    for entry: Variant in saved:
        if not entry is Dictionary or float(entry.get("hull",0.0)) <= 0.0:
            continue
        var npc := _spawn_npc(str(entry.get("role_id","patrol")),str(entry.get("ship_design_id","starter_patrol")),Vector2(float(entry.get("x",station.x)),float(entry.get("y",station.y))),station)
        npc.velocity = Vector2(float(entry.get("vx",0.0)),float(entry.get("vy",0.0)))
        npc.hull = float(entry.get("hull",npc.hull))
        npc.armour = float(entry.get("armour",npc.armour))
        npc.energy = float(entry.get("energy",npc.energy))
        npc.cargo = entry.get("cargo",{})
        restored.append(npc)
    for npc: NPCShip in restored:
        if npc.role_id == "pirate" and not restored.is_empty():
            npc.target = restored[0]
        else:
            npc.target = player

func _spawn_npc(role: String, design: String, point: Vector2, station: Vector2) -> NPCShip:
    var npc := NPC_SCENE.instantiate() as NPCShip
    npc.role_id = role
    npc.ship_design_id = design
    npc.position = point
    npc.station_position = station
    npc.target = player
    npc.loot_enabled = npc_salvage_enabled
    npc.projectile_pool = projectile_pool
    npc.drop_container = drop_container
    npc_container.add_child(npc)
    return npc

func _restore_system_drops() -> void:
    var saved: Array = GameState.system_states.get(_system_id,{}).get("drops",[])
    for entry: Variant in saved:
        if not entry is Dictionary:
            continue
        var drop := DROP_SCENE.instantiate() as CargoDrop
        drop.item_id = str(entry.get("item_id","metal"))
        drop.quantity = int(entry.get("quantity",1))
        drop_container.add_child(drop)
        drop.position = Vector2(float(entry.get("x",0.0)),float(entry.get("y",0.0)))

func _save_system_state() -> void:
    var drops: Array[Dictionary] = []
    var npcs: Array[Dictionary] = []
    for npc_node: Node in npc_container.get_children():
        if npc_node is NPCShip:
            var npc := npc_node as NPCShip
            npcs.append({
                "role_id":npc.role_id,
                "ship_design_id":npc.ship_design_id,
                "x":npc.position.x,
                "y":npc.position.y,
                "vx":npc.velocity.x,
                "vy":npc.velocity.y,
                "hull":npc.hull,
                "armour":npc.armour,
                "energy":npc.energy,
                "cargo":npc.cargo
            })
    for node: Node in drop_container.get_children():
        if node is CargoDrop:
            var crate := node as CargoDrop
            drops.append({"item_id":crate.item_id,"quantity":crate.quantity,"x":crate.position.x,"y":crate.position.y})
    GameState.system_states[_system_id] = {"initialized":true,"drops":drops,"npcs":npcs}

func _on_hud_action(action: String, value: String = "") -> void:
    match action:
        "LOOT":
            _collect_nearest_drop()
        "LAND":
            var planet := _find_planet(value)
            if is_instance_valid(planet) and player.global_position.distance_to(planet.global_position) <= planet.interaction_radius:
                player.velocity = Vector2.ZERO
                hud.show_planet_services(planet.body_id,planet.body_name)
        "REFUEL":
            var planet := _find_planet(value)
            if is_instance_valid(planet) and player.global_position.distance_to(planet.global_position) <= planet.interaction_radius:
                var purchased := GameState.buy_fuel(10)
                if purchased:
                    GameState.save_game()
                hud.show_planet_service_result("Added 10 fuel." if purchased else "Not enough credits or cargo space.")
        "FTL":
            hud.show_galaxy_map()
        "JUMP":
            await _begin_ftl_jump(value)
        "BAG":
            hud.toggle_inventory()
        "SAVE":
            GameState.save_game()
        "NPC_LOOT":
            npc_salvage_enabled = not npc_salvage_enabled
            for npc_node: Node in npc_container.get_children():
                if npc_node is NPCShip:
                    (npc_node as NPCShip).loot_enabled = npc_salvage_enabled
            hud.set_npc_loot_enabled(npc_salvage_enabled)
        "JETTISON":
            var parts := value.split(":")
            if parts.size() == 2:
                _jettison_item(parts[0],int(parts[1]))
        "SELL":
            var station_data: Array = ContentDatabase.get_system(_system_id).get("station_position",[480,270])
            var station := Vector2(float(station_data[0]),float(station_data[1]))
            if player.global_position.distance_to(station) <= 90.0:
                GameState.sell_item(value,int(GameState.inventory.get(value,0)))
                hud.refresh_inventory()
        "RECOVER":
            player.hull = 70.0
            player.armour = 30.0
            player.energy = 100.0
            player.position = Vector2(480,270)
            player.velocity = Vector2.ZERO
            _update_player_state()
            GameState.save_game()

func _begin_ftl_jump(destination: String) -> void:
    if not GameState.can_jump_to(destination):
        return
    player.visible = false
    player.set_physics_process(false)
    await hud.play_ftl_transition()
    var completed := GameState.jump_to_system(destination)
    if completed:
        _apply_player_state()
    await hud.finish_ftl_transition()
    player.visible = true
    player.set_physics_process(true)

func _jettison_item(item_id: String, quantity: int) -> void:
    if not GameState.remove_item(item_id,quantity):
        return
    var drop := DROP_SCENE.instantiate() as CargoDrop
    drop.item_id = item_id
    drop.quantity = quantity
    drop_container.add_child(drop)
    drop.global_position = player.global_position + Vector2(26.0,0.0)
    GameState.save_game()

func _collect_nearest_drop() -> void:
    var nearest: CargoDrop
    var best_distance := 38.0
    for node: Node in drop_container.get_children():
        if node is CargoDrop:
            var crate := node as CargoDrop
            var distance := player.global_position.distance_to(crate.global_position)
            if distance < best_distance:
                nearest = crate
                best_distance = distance
    if nearest != null and GameState.add_item(nearest.item_id,nearest.quantity):
        nearest.queue_free()
        GameState.save_game()

func _notification(what: int) -> void:
    if what == NOTIFICATION_APPLICATION_PAUSED:
        _update_player_state()
        _save_system_state()
        GameState.save_game()

func _exit_tree() -> void:
    _update_player_state()
    _save_system_state()
    GameState.save_game()
