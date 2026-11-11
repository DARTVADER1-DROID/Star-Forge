extends CharacterBody2D
class_name NPCShip

@export var role_id := "cargo_hauler"
@export var ship_design_id := "starter_freighter"
@export var acceleration := 95.0
@export var maximum_speed := 110.0
@export var weapon_cooldown := 0.85

var hull := 65.0
var armour := 20.0
var energy := 100.0
var cargo: Dictionary = {}
var station_position := Vector2.ZERO
var target: Node2D
var projectile_container: Node2D
var drop_container: Node2D
var loot_enabled := false
var cargo_capacity := 100
var _fire_timer := 0.0
var _loot_timer := 0.0
var _phase := 0.0

func _ready() -> void:
    var role: Dictionary = ContentDatabase.npc_roles.get(role_id, {})
    var design: Dictionary = ContentDatabase.ship_designs.get(ship_design_id, {})
    if role.is_empty() or design.is_empty():
        push_error("NPC has unknown role/design: %s/%s" % [role_id,ship_design_id])
        queue_free()
        return
    hull = float(design.get("base_hull",65.0))
    cargo_capacity = int(design.get("cargo_capacity",50))
    _phase = float(get_instance_id() % 1000) / 1000.0 * TAU
    var manifest: Dictionary = role.get("manifest", {})
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
    energy = minf(100.0,energy + 12.0 * delta)
    _fire_timer = maxf(0.0,_fire_timer-delta)
    _loot_timer = maxf(0.0,_loot_timer-delta)
    if loot_enabled and _loot_timer <= 0.0:
        _try_collect_salvage()
        _loot_timer = 0.4
    var goal := station_position
    if role_id == "pirate" and is_instance_valid(target):
        goal = target.global_position
    elif role_id == "patrol":
        _phase += delta * 0.35
        goal = station_position + Vector2(cos(_phase),sin(_phase))*120.0
    var offset := goal-global_position
    if offset.length() > 8.0:
        velocity += offset.normalized()*acceleration*delta
    velocity = velocity.limit_length(maximum_speed)
    velocity *= pow(0.985,delta*60.0)
    move_and_slide()
    if role_id == "pirate" and is_instance_valid(target) and global_position.distance_to(target.global_position)<320.0:
        _fire_at_target()
    queue_redraw()

func apply_damage(amount: float) -> void:
    var absorbed := minf(armour,maxf(0.0,amount))
    armour -= absorbed
    hull = maxf(0.0,hull-(amount-absorbed))

func _fire_at_target() -> void:
    if _fire_timer>0.0 or energy<8.0 or not is_instance_valid(projectile_container):
        return
    energy -= 8.0
    _fire_timer = weapon_cooldown
    var shot := preload("res://scenes/projectile.tscn").instantiate() as SpaceProjectile
    projectile_container.add_child(shot)
    shot.global_position = global_position
    shot.direction = (target.global_position-global_position).normalized()
    shot.damage = 8.0
    shot.from_player = false
    shot.speed = 230.0

func _try_collect_salvage() -> void:
    if not is_instance_valid(drop_container):
        return
    var used := 0
    for cargo_id: String in cargo:
        used += int(cargo[cargo_id]) * int(ContentDatabase.get_item(cargo_id).get("cargo_units_per_item",1))
    for node: Node in drop_container.get_children():
        if not node is CargoDrop:
            continue
        var crate := node as CargoDrop
        if global_position.distance_to(crate.global_position) > 28.0:
            continue
        var item := ContentDatabase.get_item(crate.item_id)
        var volume := int(item.get("cargo_units_per_item",1)) * crate.quantity
        if used + volume <= cargo_capacity:
            cargo[crate.item_id] = int(cargo.get(crate.item_id,0)) + crate.quantity
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
    var tint := Color(0.92,0.34,0.28) if role_id=="pirate" else Color(0.42,0.78,0.62)
    draw_colored_polygon(PackedVector2Array([Vector2(0,-14),Vector2(10,10),Vector2(0,6),Vector2(-10,10)]),tint)
