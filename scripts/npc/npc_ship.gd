extends CharacterBody2D
class_name NPCShip

@export var role_id := "cargo_hauler"
@export var ship_design_id := "starter_freighter"
@export var acceleration := 95.0
@export var maximum_speed := 110.0
@export var weapon_cooldown := 0.85
@export var station_dock_radius := 44.0
@export var station_safety_radius := 118.0

var hull := 65.0
var armour := 20.0
var energy := 100.0
var cargo: Dictionary = {}
var station_position := Vector2.ZERO
var target: Node2D
var projectile_pool: ProjectilePool
var drop_container: Node2D
var loot_enabled := false
var cargo_capacity := 100
var is_docked := false
var _fire_timer := 0.0
var _loot_timer := 0.0
var _phase := 0.0
var _sprite: Sprite2D

func _ready() -> void:
    var role: Dictionary = ContentDatabase.npc_roles.get(role_id,{})
    var design: Dictionary = ContentDatabase.ship_designs.get(ship_design_id,{})
    if role.is_empty() or design.is_empty():
        push_error("NPC has unknown role/design: %s/%s" % [role_id,ship_design_id])
        queue_free()
        return
    hull = float(design.get("base_hull",65.0))
    cargo_capacity = int(design.get("cargo_capacity",50))
    _phase = float(get_instance_id()%1000)/1000.0*TAU
    var sprite_name := "ship_pirate_interceptor.png" if role_id == "pirate" else ("ship_cargo_hauler.png" if role_id == "cargo_hauler" else "ship_military_frigate.png")
    var texture := AssetLibrary.load_texture(sprite_name)
    if texture != null:
        _sprite = Sprite2D.new()
        _sprite.texture = texture
        _sprite.scale = Vector2(0.48,0.48)
        _sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
        add_child(_sprite)
    var manifest: Dictionary = role.get("manifest",{})
    var random := RandomNumberGenerator.new()
    random.randomize()
    for item_id: String in manifest:
        var limits: Array = manifest[item_id]
        if limits.size() == 2:
            cargo[item_id] = random.randi_range(int(limits[0]),int(limits[1]))

func _physics_process(delta: float) -> void:
    if hull <= 0.0:
        _drop_cargo()
        queue_free()
        return
    energy = minf(100.0,energy+12.0*delta)
    _fire_timer = maxf(0.0,_fire_timer-delta)
    _loot_timer = maxf(0.0,_loot_timer-delta)
    if loot_enabled and _loot_timer <= 0.0:
        _try_collect_salvage()
        _loot_timer = 0.4

    if role_id == "cargo_hauler":
        _update_station_docking(delta)
    else:
        var goal := station_position
        if role_id == "pirate" and is_instance_valid(target):
            var target_docked := target is NPCShip and (target as NPCShip).is_docked
            goal = station_position+Vector2(0.0,90.0) if target_docked else target.global_position
        elif role_id == "patrol":
            _phase += delta*0.35
            goal = station_position+Vector2(cos(_phase),sin(_phase))*120.0
        var offset := goal-global_position
        if offset.length() > 8.0:
            velocity += offset.normalized()*acceleration*delta
        velocity = velocity.limit_length(maximum_speed)
        velocity *= pow(0.985,delta*60.0)
        move_and_slide()

    if velocity.length_squared() > 1.0:
        rotation = velocity.angle()+PI/2.0
    if can_engage_target():
        _fire_at_target()
    queue_redraw()

func _update_station_docking(delta: float) -> void:
    var offset := station_position-global_position
    var distance := offset.length()
    if is_docked and distance <= station_dock_radius+12.0:
        velocity = Vector2.ZERO
        return
    is_docked = false
    if distance <= station_dock_radius and velocity.length() <= 24.0:
        is_docked = true
        velocity = Vector2.ZERO
        return
    var stopping_speed := sqrt(2.0*acceleration*maxf(0.0,distance-station_dock_radius))
    var desired_speed := minf(maximum_speed,stopping_speed)
    var desired_velocity := offset.normalized()*desired_speed if distance > 0.01 else Vector2.ZERO
    velocity = velocity.move_toward(desired_velocity,acceleration*delta)
    move_and_slide()
    if global_position.distance_to(station_position) <= station_dock_radius and velocity.length() <= 24.0:
        is_docked = true
        velocity = Vector2.ZERO

func can_engage_target() -> bool:
    if role_id != "pirate" or not is_instance_valid(target):
        return false
    if target is NPCShip and (target as NPCShip).is_docked:
        return false
    if global_position.distance_to(station_position) <= station_safety_radius:
        return false
    return global_position.distance_to(target.global_position) < 320.0

func apply_damage(amount: float) -> void:
    var absorbed := minf(armour,maxf(0.0,amount))
    armour -= absorbed
    hull = maxf(0.0,hull-(amount-absorbed))

func _fire_at_target() -> void:
    if _fire_timer > 0.0 or energy < 8.0 or not is_instance_valid(projectile_pool):
        return
    var direction_to_target := (target.global_position-global_position).normalized()
    if not projectile_pool.spawn(global_position,direction_to_target,false,8.0,230.0):
        return
    energy -= 8.0
    _fire_timer = weapon_cooldown

func _try_collect_salvage() -> void:
    if not is_instance_valid(drop_container):
        return
    var used := 0
    for cargo_id: String in cargo:
        used += int(cargo[cargo_id])*int(ContentDatabase.get_item(cargo_id).get("cargo_units_per_item",1))
    for node: Node in drop_container.get_children():
        if not node is CargoDrop:
            continue
        var crate := node as CargoDrop
        if global_position.distance_to(crate.global_position) > 28.0:
            continue
        var item := ContentDatabase.get_item(crate.item_id)
        var volume := int(item.get("cargo_units_per_item",1))*crate.quantity
        if used+volume <= cargo_capacity:
            cargo[crate.item_id] = int(cargo.get(crate.item_id,0))+crate.quantity
            crate.queue_free()
            break

func _drop_cargo() -> void:
    if not is_instance_valid(drop_container):
        return
    for item_id: String in cargo:
        var amount := int(cargo[item_id])
        if amount <= 0:
            continue
        var drop := preload("res://scenes/cargo_drop.tscn").instantiate() as CargoDrop
        drop.item_id = item_id
        drop.quantity = amount
        drop_container.add_child(drop)
        drop.global_position = global_position
    var random := RandomNumberGenerator.new()
    random.randomize()
    if random.randf() < 0.35:
        var weapon_drop := preload("res://scenes/cargo_drop.tscn").instantiate() as CargoDrop
        weapon_drop.item_id = "weapon_salvage"
        weapon_drop.quantity = 1
        drop_container.add_child(weapon_drop)
        weapon_drop.global_position = global_position

func _draw() -> void:
    if is_instance_valid(_sprite):
        return
    var tint := Color(0.92,0.34,0.28) if role_id == "pirate" else Color(0.42,0.78,0.62)
    draw_colored_polygon(PackedVector2Array([Vector2(0,-14),Vector2(10,10),Vector2(0,6),Vector2(-10,10)]),tint)
