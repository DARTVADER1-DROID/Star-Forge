extends Node2D
class_name SpaceProjectile

@export var speed: float = 360.0
@export var lifetime: float = 1.8

var direction: Vector2 = Vector2.UP
var _age: float = 0.0

func _process(delta: float) -> void:
    _age += delta
    if _age >= lifetime:
        queue_free()
        return

    global_position += direction * speed * delta

func _draw() -> void:
    draw_line(Vector2.ZERO, -direction * 9.0, Color(1.0, 0.82, 0.38), 3.0)
    draw_circle(Vector2.ZERO, 2.0, Color(1.0, 0.96, 0.72))
