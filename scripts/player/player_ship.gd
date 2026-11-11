extends CharacterBody2D
class_name PlayerShip

const PROJECTILE_SCENE: PackedScene = preload("res://scenes/projectile.tscn")

@export var normal_acceleration: float = 190.0
@export var boost_acceleration: float = 340.0
@export var normal_max_speed: float = 190.0
@export var boost_max_speed: float = 270.0
@export var turn_speed: float = 4.2
@export var weapon_energy_cost: float = 8.0
@export var energy_regen_rate: float = 12.0
@export var weapon_cooldown: float = 0.22

var hull: float = 100.0
var max_hull: float = 100.0
var armour: float = 60.0
var max_armour: float = 60.0
var energy: float = 100.0
var max_energy: float = 100.0

var projectile_container: Node2D
var _steering: Vector2 = Vector2.ZERO
var _boosting: bool = false
var _firing: bool = false
var _fire_timer: float = 0.0
var _facing: Vector2 = Vector2.UP

func set_steering(direction: Vector2) -> void:
    _steering = direction.limit_length(1.0)

func set_boosting(active: bool) -> void:
    _boosting = active

func set_firing(active: bool) -> void:
    _firing = active

func _physics_process(delta: float) -> void:
    if hull <= 0.0:
        velocity = Vector2.ZERO
        return

    energy = minf(max_energy, energy + energy_regen_rate * delta)
    _fire_timer = maxf(0.0, _fire_timer - delta)

    if _steering.length_squared() > 0.0025:
        var desired_direction := _steering.normalized()
        var angle_difference := wrapf(
            desired_direction.angle() - _facing.angle(),
            -PI,
            PI
        )
        _facing = _facing.rotated(clampf(
            angle_difference,
            -turn_speed * delta,
            turn_speed * delta
        )).normalized()

        var acceleration := boost_acceleration if _boosting else normal_acceleration
        velocity += _facing * acceleration * _steering.length() * delta

    velocity *= pow(0.985, delta * 60.0)
    var speed_limit := boost_max_speed if _boosting else normal_max_speed
    velocity = velocity.limit_length(speed_limit)
    move_and_slide()
    queue_redraw()

    if _firing and _fire_timer <= 0.0 and energy >= weapon_energy_cost:
        _fire_weapon()

func _fire_weapon() -> void:
    if not is_instance_valid(projectile_container):
        return

    energy -= weapon_energy_cost
    _fire_timer = weapon_cooldown

    var projectile := PROJECTILE_SCENE.instantiate() as SpaceProjectile
    projectile.global_position = global_position + _facing * 20.0
    projectile.direction = _facing
    projectile_container.add_child(projectile)

func _draw() -> void:
    var points := PackedVector2Array([
        Vector2(0.0, -19.0),
        Vector2(13.0, 15.0),
        Vector2(0.0, 9.0),
        Vector2(-13.0, 15.0)
    ])
    draw_colored_polygon(points, Color(0.42, 0.82, 1.0))
    draw_polyline(
        PackedVector2Array([points[0], points[1], points[2], points[3], points[0]]),
        Color(0.82, 0.94, 1.0),
        2.0
    )
    if _boosting and _steering.length_squared() > 0.01:
        draw_colored_polygon(
            PackedVector2Array([
                Vector2(-5.0, 13.0),
                Vector2(0.0, 27.0 + randf_range(0.0, 4.0)),
                Vector2(5.0, 13.0)
            ]),
            Color(1.0, 0.56, 0.19)
        )
