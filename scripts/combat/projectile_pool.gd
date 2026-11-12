extends Node2D
class_name ProjectilePool

const PROJECTILE_SCENE: PackedScene = preload("res://scenes/projectile.tscn")
const DEFAULT_CAPACITY := 64

@export_range(16,128,1) var capacity := DEFAULT_CAPACITY

var _projectiles: Array[SpaceProjectile] = []

func _ready() -> void:
    _projectiles.resize(capacity)
    for index in range(capacity):
        var projectile := PROJECTILE_SCENE.instantiate() as SpaceProjectile
        add_child(projectile)
        projectile.deactivate()
        _projectiles[index] = projectile

func spawn(
    spawn_position: Vector2,
    direction: Vector2,
    fired_by_player: bool,
    damage: float,
    speed: float,
    safe_zone_center: Vector2 = Vector2.ZERO,
    safe_zone_radius: float = 0.0
) -> bool:
    for projectile: SpaceProjectile in _projectiles:
        if not projectile.active:
            projectile.activate(spawn_position,direction,fired_by_player,damage,speed,safe_zone_center,safe_zone_radius)
            return true
    return false

func clear_active() -> void:
    for projectile: SpaceProjectile in _projectiles:
        if projectile.active:
            projectile.deactivate()

func get_active_count() -> int:
    var count := 0
    for projectile: SpaceProjectile in _projectiles:
        if projectile.active:
            count += 1
    return count
