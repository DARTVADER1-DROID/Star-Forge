extends ParallaxBackground
class_name ParallaxStarfield

func _ready() -> void:
    _add_layer("bg_nebula_320x180.png",Vector2(0.025,0.025),0.24)
    _add_layer("stars_far_128.png",Vector2(0.08,0.08),0.48)
    _add_layer("stars_mid_128.png",Vector2(0.24,0.24),0.62)
    _add_layer("stars_near_128.png",Vector2(0.62,0.62),0.72)

func _add_layer(file_name: String, motion: Vector2, opacity: float) -> void:
    var texture := AssetLibrary.load_texture(file_name)
    if texture == null:
        push_warning("Parallax layer skipped; texture is not registered: " + file_name)
        return
    var layer := ParallaxLayer.new()
    layer.motion_scale = motion
    layer.motion_mirroring = Vector2(4096.0,4096.0)
    add_child(layer)
    var sprite := Sprite2D.new()
    sprite.texture = texture
    sprite.region_enabled = true
    sprite.region_rect = Rect2(-2048.0,-2048.0,4096.0,4096.0)
    sprite.texture_repeat = CanvasItem.TEXTURE_REPEAT_ENABLED
    sprite.modulate = Color(1.0,1.0,1.0,opacity)
    sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
    layer.add_child(sprite)
