extends Area2D
class_name SpaceProjectile

@export var speed := 360.0
@export var lifetime := 1.8
@export var damage := 24.0
var direction := Vector2.UP
var from_player := true
var _age := 0.0

func _ready() -> void:
    var shape := CircleShape2D.new()
    shape.radius = 4.0
    var collision := CollisionShape2D.new()
    collision.shape = shape
    add_child(collision)
    body_entered.connect(_on_body_entered)

func _process(delta: float) -> void:
    _age += delta
    if _age >= lifetime:
        queue_free()
        return
    global_position += direction.normalized() * speed * delta

func _on_body_entered(body: Node2D) -> void:
    if body is SpaceAsteroid or body is PlanetBody:
        queue_free()
    elif from_player and body is NPCShip:
        (body as NPCShip).apply_damage(damage)
        queue_free()
    elif not from_player and body is PlayerShip:
        (body as PlayerShip).apply_damage(damage)
        queue_free()

func _draw() -> void:
    draw_line(Vector2.ZERO, -direction.normalized() * 9.0, Color(1.0,0.82,0.38), 3.0)
    draw_circle(Vector2.ZERO, 2.0, Color(1.0,0.96,0.72))
