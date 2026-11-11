extends StaticBody2D
class_name SpaceAsteroid

@export var collision_radius := 24.0
var _sprite: Sprite2D

func _ready() -> void:
    var sprite_names := ["asteroid_small_01.png","asteroid_small_02.png","asteroid_small_03.png","asteroid_large_01.png"]
    var random := RandomNumberGenerator.new()
    random.randomize()
    var texture := AssetLibrary.load_texture(sprite_names[random.randi_range(0,sprite_names.size()-1)])
    if texture != null:
        _sprite = Sprite2D.new()
        _sprite.texture = texture
        _sprite.scale = Vector2(0.55,0.55)
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
    draw_colored_polygon(PackedVector2Array([
        Vector2(-20,-10),Vector2(-7,-23),Vector2(13,-18),
        Vector2(23,-2),Vector2(12,19),Vector2(-11,20),Vector2(-23,3)
    ]), Color(0.40,0.44,0.53))
