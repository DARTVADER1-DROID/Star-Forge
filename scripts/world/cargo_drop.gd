extends Area2D
class_name CargoDrop

@export var item_id := "metal"
@export var quantity := 1

func _ready() -> void:
    var shape := CircleShape2D.new()
    shape.radius = 18.0
    var collision := CollisionShape2D.new()
    collision.shape = shape
    add_child(collision)

func _draw() -> void:
    draw_circle(Vector2.ZERO, 8.0, Color(0.96,0.72,0.28))
    draw_arc(Vector2.ZERO, 11.0, 0.0, TAU, 24, Color(1.0,0.9,0.62), 2.0)
