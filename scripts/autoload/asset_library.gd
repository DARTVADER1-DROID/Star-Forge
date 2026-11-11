extends Node

var _textures: Dictionary = {}
var _paths: Dictionary = {}

func _ready() -> void:
    _scan_directory("res://assets")

func load_texture(file_name: String) -> Texture2D:
    if _textures.has(file_name):
        return _textures[file_name] as Texture2D
    var path := str(_paths.get(file_name, ""))
    if path.is_empty():
        return null
    var texture := load(path) as Texture2D
    if texture == null:
        push_warning("Asset could not be loaded: " + path)
        return null
    _textures[file_name] = texture
    return texture

func has_asset(file_name: String) -> bool:
    return _paths.has(file_name)

func _scan_directory(path: String) -> void:
    var directory := DirAccess.open(path)
    if directory == null:
        return
    directory.list_dir_begin()
    var entry := directory.get_next()
    while not entry.is_empty():
        if not entry.begins_with("."):
            var full_path := path.path_join(entry)
            if directory.current_is_dir():
                _scan_directory(full_path)
            elif entry.get_extension().to_lower() in ["png", "webp", "jpg", "jpeg"]:
                _paths[entry] = full_path
        entry = directory.get_next()
    directory.list_dir_end()
