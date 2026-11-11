extends StaticBody2D
class_name PlanetBody

@export var body_radius := 38.0
@export var body_color := Color(0.22,0.48,0.68)

var _sprite: Sprite2D

func _ready() -> void:
    var shape := CircleShape2D.new()
    shape.radius = body_radius
    var collision := CollisionShape2D.new()
    collision.shape = shape
    add_child(collision)

    var texture_name := "planet_moon_32.png"
    if body_radius >= 50.0:
        texture_name = "planet_gas_96.png"
    elif body_radius > 18.0:
        texture_name = "planet_terran_64.png"

    var texture := AssetLibrary.load_texture(texture_name)
    if texture != null:
        _sprite = Sprite2D.new()
        _sprite.texture = texture
        _sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
        var diameter := body_radius * 2.0
        var scale_factor := diameter / float(maxi(texture.get_width(),texture.get_height()))
        _sprite.scale = Vector2.ONE * scale_factor
        add_child(_sprite)

func _draw() -> void:
    if is_instance_valid(_sprite):
        return
    draw_circle(Vector2.ZERO,body_radius,body_color)
    draw_arc(Vector2.ZERO,body_radius+2.0,0.0,TAU,48,Color(0.66,0.82,0.96,0.65),2.0)
