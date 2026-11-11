extends StaticBody2D
class_name StationBody

@export var collision_radius := 28.0

func _ready() -> void:
    var shape := CircleShape2D.new()
    shape.radius = collision_radius
    var collision := CollisionShape2D.new()
    collision.shape = shape
    add_child(collision)

func _draw() -> void:
    draw_circle(Vector2.ZERO,collision_radius,Color(0.18,0.28,0.42))
    draw_arc(Vector2.ZERO,collision_radius+5.0,0.0,TAU,48,Color(0.48,0.78,0.96),3.0)
    draw_arc(Vector2.ZERO,collision_radius*0.58,0.0,TAU,32,Color(0.70,0.82,0.94,0.85),2.0)
    draw_circle(Vector2.ZERO,6.0,Color(0.92,0.72,0.34))
