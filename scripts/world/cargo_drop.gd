extends Area2D
class_name CargoDrop

@export var item_id := "metal"
@export var quantity := 1

var _sprite: Sprite2D

func _ready() -> void:
    var shape := CircleShape2D.new()
    shape.radius = 18.0
    var collision := CollisionShape2D.new()
    collision.shape = shape
    add_child(collision)

    var texture_name := "salvage_container_generic.png"
    match item_id:
        "fuel":
            texture_name = "salvage_fuel.png"
        "metal":
            texture_name = "salvage_metal.png"
        "luxury":
            texture_name = "salvage_luxury.png"
        "equipment":
            texture_name = "salvage_equipment.png"
        "weapon_salvage":
            texture_name = "salvage_weapon.png"
        "food":
            texture_name = "salvage_food.png"
        "medicine":
            texture_name = "salvage_medicine.png"

    var texture := AssetLibrary.load_texture(texture_name)
    if texture != null:
        _sprite = Sprite2D.new()
        _sprite.texture = texture
        _sprite.scale = Vector2(0.72,0.72)
        _sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
        add_child(_sprite)

func _draw() -> void:
    if is_instance_valid(_sprite):
        return
    draw_circle(Vector2.ZERO,8.0,Color(0.96,0.72,0.28))
    draw_arc(Vector2.ZERO,11.0,0.0,TAU,24,Color(1.0,0.9,0.62),2.0)
