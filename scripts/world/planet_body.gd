extends StaticBody2D
class_name PlanetBody

@export var body_radius := 38.0
@export var body_color := Color(0.22,0.48,0.68)

func _ready() -> void:
    var shape := CircleShape2D.new()
    shape.radius = body_radius
    var collision := CollisionShape2D.new()
    collision.shape = shape
    add_child(collision)

func _draw() -> void:
    draw_circle(Vector2.ZERO, body_radius, body_color)
    draw_arc(Vector2.ZERO, body_radius + 2.0, 0.0, TAU, 48, Color(0.66,0.82,0.96,0.65), 2.0)
