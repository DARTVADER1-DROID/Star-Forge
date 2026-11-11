extends StaticBody2D
class_name SpaceAsteroid

@export var collision_radius := 24.0

func _ready() -> void:
    var shape := CircleShape2D.new()
    shape.radius = collision_radius
    var collision := CollisionShape2D.new()
    collision.shape = shape
    add_child(collision)

func _draw() -> void:
    draw_colored_polygon(PackedVector2Array([
        Vector2(-20,-10),Vector2(-7,-23),Vector2(13,-18),
        Vector2(23,-2),Vector2(12,19),Vector2(-11,20),Vector2(-23,3)
    ]), Color(0.40,0.44,0.53))
