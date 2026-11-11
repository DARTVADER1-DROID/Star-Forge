extends CharacterBody2D
class_name PlayerShip

@export var normal_acceleration := 190.0
@export var boost_acceleration := 340.0
@export var normal_max_speed := 190.0
@export var boost_max_speed := 270.0
@export var turn_speed := 4.2
@export var weapon_energy_cost := 8.0
@export var energy_regen_rate := 12.0
@export var weapon_cooldown := 0.22

var hull := 100.0
var max_hull := 100.0
var armour := 60.0
var max_armour := 60.0
var energy := 100.0
var max_energy := 100.0
var projectile_pool: ProjectilePool
var _steering := Vector2.ZERO
var _boosting := false
var _firing := false
var _fire_timer := 0.0
var _facing := Vector2.UP
var _sprite: Sprite2D

func set_steering(direction: Vector2) -> void:
    _steering = direction.limit_length(1.0)

func set_boosting(active: bool) -> void:
    _boosting = active

func set_firing(active: bool) -> void:
    _firing = active

func _ready() -> void:
    var texture := AssetLibrary.load_texture("ship_player_runner.png")
    if texture != null:
        _sprite = Sprite2D.new()
        _sprite.texture = texture
        _sprite.scale = Vector2(0.55,0.55)
        _sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
        add_child(_sprite)

func _physics_process(delta: float) -> void:
    if hull <= 0.0:
        velocity = Vector2.ZERO
        return
    energy = minf(max_energy,energy+energy_regen_rate*delta)
    _fire_timer = maxf(0.0,_fire_timer-delta)
    if _steering.length_squared() > 0.0025:
        var desired := _steering.normalized()
        var difference := wrapf(desired.angle()-_facing.angle(),-PI,PI)
        _facing = _facing.rotated(clampf(difference,-turn_speed*delta,turn_speed*delta)).normalized()
        var thrust := boost_acceleration if _boosting else normal_acceleration
        velocity += _facing*thrust*_steering.length()*delta
    velocity *= pow(0.985,delta*60.0)
    velocity = velocity.limit_length(boost_max_speed if _boosting else normal_max_speed)
    rotation = _facing.angle()+PI/2.0
    move_and_slide()
    if _firing and _fire_timer <= 0.0 and energy >= weapon_energy_cost:
        _fire_weapon()
    queue_redraw()

func _fire_weapon() -> void:
    if not is_instance_valid(projectile_pool):
        return
    if not projectile_pool.spawn(global_position+_facing*20.0,_facing,true,24.0,360.0):
        return
    energy -= weapon_energy_cost
    _fire_timer = weapon_cooldown

func apply_damage(amount: float) -> void:
    if amount <= 0.0 or hull <= 0.0:
        return
    var absorbed := minf(armour,amount)
    armour -= absorbed
    hull = maxf(0.0,hull-(amount-absorbed))
    if hull <= 0.0:
        GameState.player_destroyed.emit()

func _draw() -> void:
    if is_instance_valid(_sprite):
        return
    var points := PackedVector2Array([Vector2(0,-19),Vector2(13,15),Vector2(0,9),Vector2(-13,15)])
    draw_colored_polygon(points,Color(0.42,0.82,1.0))
    draw_polyline(PackedVector2Array([points[0],points[1],points[2],points[3],points[0]]),Color(0.82,0.94,1.0),2.0)
    if _boosting and _steering.length_squared()>0.01:
        draw_colored_polygon(PackedVector2Array([Vector2(-5,13),Vector2(0,28),Vector2(5,13)]),Color(1.0,0.56,0.19))
