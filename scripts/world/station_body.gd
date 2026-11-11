extends StaticBody2D
class_name StationBody

@export var collision_radius := 28.0
var _sprite: Sprite2D

func _ready() -> void:
    var texture := AssetLibrary.load_texture("station_ring_hub.png")
    if texture != null:
        _sprite = Sprite2D.new()
        _sprite.texture = texture
        _sprite.scale = Vector2(0.72,0.72)
        _sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
        add_child(_sprite)
    var shape := CircleShape2D.new()
    shape.radius = collision_radius
    var collision := CollisionShape2D.new()
    collision.shape = shape
    add_child(collision)

func _draw() -> void:
    if is_instance_valid(_sprite):
        return
    draw_circle(Vector2.ZERO,collision_radius,Color(0.18,0.28,0.42))
    draw_arc(Vector2.ZERO,collision_radius+5.0,0.0,TAU,48,Color(0.48,0.78,0.96),3.0)
    draw_arc(Vector2.ZERO,collision_radius*0.58,0.0,TAU,32,Color(0.70,0.82,0.94,0.85),2.0)
    draw_circle(Vector2.ZERO,6.0,Color(0.92,0.72,0.34))
