extends Area2D
class_name SpaceProjectile

@export var lifetime := 1.8

var speed := 360.0
var damage := 24.0
var direction := Vector2.UP
var from_player := true
var active := false
var _age := 0.0
var _sprite: Sprite2D

func _ready() -> void:
    var shape := CircleShape2D.new()
    shape.radius = 4.0
    var collision := CollisionShape2D.new()
    collision.shape = shape
    add_child(collision)
    body_entered.connect(_on_body_entered)
    _sprite = Sprite2D.new()
    _sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
    add_child(_sprite)
    monitoring = false
    visible = false
    set_physics_process(false)

func activate(
    spawn_position: Vector2,
    travel_direction: Vector2,
    fired_by_player: bool,
    projectile_damage: float,
    projectile_speed: float
) -> void:
    global_position = spawn_position
    direction = travel_direction.normalized()
    from_player = fired_by_player
    collision_mask = 1 if from_player else 2
    damage = projectile_damage
    speed = projectile_speed
    _age = 0.0
    if is_instance_valid(_sprite):
        var texture_name := "proj_laser_blue.png" if from_player else "proj_laser_red.png"
        _sprite.texture = AssetLibrary.load_texture(texture_name)
        _sprite.rotation = direction.angle() + PI / 2.0
        _sprite.scale = Vector2(0.65,0.65)
    active = true
    visible = true
    set_physics_process(true)
    set_deferred("monitoring",true)

func deactivate() -> void:
    active = false
    visible = false
    set_physics_process(false)
    set_deferred("monitoring",false)

func _physics_process(delta: float) -> void:
    if not active:
        return
    _age += delta
    if _age >= lifetime:
        deactivate()
        return
    global_position += direction * speed * delta

func _on_body_entered(body: Node2D) -> void:
    if not active:
        return
    if body is SpaceAsteroid or body is PlanetBody or body is StationBody:
        deactivate()
    elif from_player and body is NPCShip:
        (body as NPCShip).apply_damage(damage)
        deactivate()
    elif not from_player and body is PlayerShip:
        (body as PlayerShip).apply_damage(damage)
        deactivate()

func _draw() -> void:
    if not active or (is_instance_valid(_sprite) and _sprite.texture != null):
        return
    draw_line(Vector2.ZERO,-direction*9.0,Color(1.0,0.82,0.38),3.0)
    draw_circle(Vector2.ZERO,2.0,Color(1.0,0.96,0.72))
