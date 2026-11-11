extends Node2D
class_name StarForgeWorld

const STAR_COUNT: int = 110

@onready var player: PlayerShip = $PlayerShip
@onready var hud: TouchHUD = $HUD
@onready var projectile_container: Node2D = $Projectiles

var _stars: PackedVector2Array = PackedVector2Array()

func _ready() -> void:
    get_viewport().size_changed.connect(queue_redraw)
    _create_star_field()
    player.projectile_container = projectile_container
    hud.steering_changed.connect(player.set_steering)
    hud.boost_changed.connect(player.set_boosting)
    hud.fire_changed.connect(player.set_firing)
    _update_hud()

func _process(_delta: float) -> void:
    _update_hud()

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO, get_viewport_rect().size), Color(0.012, 0.02, 0.055))
    for star_position: Vector2 in _stars:
        draw_circle(star_position, 1.0, Color(0.55, 0.67, 0.86, 0.72))

func _create_star_field() -> void:
    var random := RandomNumberGenerator.new()
    random.seed = 73421
    var viewport_size := get_viewport_rect().size
    for _index in range(STAR_COUNT):
        _stars.append(Vector2(
            random.randf_range(0.0, viewport_size.x),
            random.randf_range(0.0, viewport_size.y)
        ))

func _update_hud() -> void:
    if is_instance_valid(player) and is_instance_valid(hud):
        hud.update_ship_status(
            player.hull,
            player.max_hull,
            player.armour,
            player.max_armour,
            player.energy,
            player.max_energy
        )
