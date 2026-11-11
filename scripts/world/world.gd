extends Node2D
class_name StarForgeWorld

const STAR_COUNT := 110
const NPC_SCENE: PackedScene = preload("res://scenes/npc_ship.tscn")
const DROP_SCENE: PackedScene = preload("res://scenes/cargo_drop.tscn")

@onready var player: PlayerShip = $PlayerShip
@onready var hud: TouchHUD = $HUDLayer/HUD
@onready var projectile_pool: ProjectilePool = $ProjectilePool
@onready var npc_container: Node2D = $NPCs
@onready var obstacle_container: Node2D = $Obstacles
@onready var drop_container: Node2D = $CargoDrops

var _system_id := ""
var npc_salvage_enabled := false

func _ready() -> void:
    # ParallaxStarfield owns background rendering independently from world logic.
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
    _update_world_interaction()
    hud.update_system_map(player.position)

func _update_world_interaction() -> void:
    var interaction := WorldInteractionService.find_nearest(player.global_position,obstacle_container.get_children())
    hud.set_nearby_interaction(str(interaction.get("action","")),str(interaction.get("id","")),str(interaction.get("name","")))

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
    var build_result := SystemContentBuilder.build_system(obstacle_container,id,definition)
    var station: Vector2 = build_result.station_position
    _restore_or_spawn_npcs(station)
    _restore_system_drops()

func _find_planet(planet_id: String) -> PlanetBody:
    for node: Node in obstacle_container.get_children():
        if node is PlanetBody and (node as PlanetBody).body_id == planet_id:
            return node as PlanetBody
    return null

func _find_station() -> StationBody:
    for node: Node in obstacle_container.get_children():
        if node is StationBody:
            return node as StationBody
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
        "DOCK":
            var station := _find_station()
            if is_instance_valid(station) and player.global_position.distance_to(station.global_position) <= station.interaction_radius:
                player.velocity = Vector2.ZERO
                hud.show_station_services(station.body_name)
        "REFUEL":
            var can_refuel := false
            if value == "station":
                var station := _find_station()
                can_refuel = is_instance_valid(station) and player.global_position.distance_to(station.global_position) <= station.interaction_radius
            else:
                var planet := _find_planet(value)
                can_refuel = is_instance_valid(planet) and player.global_position.distance_to(planet.global_position) <= planet.interaction_radius
            if can_refuel:
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
