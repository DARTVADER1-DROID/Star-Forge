extends Node2D
class_name StarForgeWorld

const STAR_COUNT := 110
const ASTEROID_SCENE: PackedScene = preload("res://scenes/asteroid.tscn")
const PLANET_SCENE: PackedScene = preload("res://scenes/planet_body.tscn")
const NPC_SCENE: PackedScene = preload("res://scenes/npc_ship.tscn")
const DROP_SCENE: PackedScene = preload("res://scenes/cargo_drop.tscn")

@onready var player: PlayerShip = $PlayerShip
@onready var hud: TouchHUD = $HUD
@onready var projectile_container: Node2D = $Projectiles
@onready var npc_container: Node2D = $NPCs
@onready var obstacle_container: Node2D = $Obstacles
@onready var drop_container: Node2D = $CargoDrops

var _stars := PackedVector2Array()
var _system_id := ""

func _ready() -> void:
    get_viewport().size_changed.connect(queue_redraw)
    _create_star_field()
    player.projectile_container = projectile_container
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
    hud.update_ship_status(player.hull, player.max_hull, player.armour, player.max_armour, player.energy, player.max_energy, GameState.credits, GameState.inventory)

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO, get_viewport_rect().size), Color(0.012,0.02,0.055))
    for star: Vector2 in _stars:
        draw_circle(star, 1.0, Color(0.55,0.67,0.86,0.72))

func _create_star_field() -> void:
    var random := RandomNumberGenerator.new()
    random.seed = 73421
    var bounds := get_viewport_rect().size
    for _i in range(STAR_COUNT):
        _stars.append(Vector2(random.randf_range(0.0,bounds.x),random.randf_range(0.0,bounds.y)))

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
    for index in range(planets):
        var body := PLANET_SCENE.instantiate() as PlanetBody
        obstacle_container.add_child(body)
        body.position = Vector2(150.0 + float(index)*280.0,100.0 + float(index%2)*330.0)
        body.body_radius = 30.0 + float(index%3)*12.0

    _spawn_npc("cargo_hauler","starter_freighter",station+Vector2(160,40),station)
    var pirate := _spawn_npc("pirate","starter_interceptor",station+Vector2(-160,-40),station)
    var hauler := npc_container.get_child(0) as NPCShip
    pirate.target = hauler
    _spawn_npc("patrol","starter_patrol",station+Vector2(0,150),station)
    _restore_system_drops()

func _spawn_npc(role: String, design: String, point: Vector2, station: Vector2) -> NPCShip:
    var npc := NPC_SCENE.instantiate() as NPCShip
    npc.role_id = role
    npc.ship_design_id = design
    npc.position = point
    npc.station_position = station
    npc.target = player
    npc.projectile_container = projectile_container
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
    for node: Node in drop_container.get_children():
        if node is CargoDrop:
            var crate := node as CargoDrop
            drops.append({"item_id":crate.item_id,"quantity":crate.quantity,"x":crate.position.x,"y":crate.position.y})
    GameState.system_states[_system_id] = {"drops":drops}

func _on_hud_action(action: String, value: String = "") -> void:
    match action:
        "LOOT":
            _collect_nearest_drop()
        "FTL":
            hud.show_galaxy_map()
        "JUMP":
            if GameState.jump_to_system(value):
                hud.hide_galaxy_map()
                _apply_player_state()
        "BAG":
            hud.toggle_inventory()
        "SAVE":
            GameState.save_game()
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

func _exit_tree() -> void:
    _update_player_state()
    _save_system_state()
    GameState.save_game()
