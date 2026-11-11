extends Node

const DATA_FILES := {
    "items": "res://data/items.json",
    "ships": "res://data/ship_designs.json",
    "roles": "res://data/npc_roles.json",
    "systems": "res://data/systems.json"
}
var items: Dictionary = {}
var ship_designs: Dictionary = {}
var npc_roles: Dictionary = {}
var systems: Dictionary = {}

func _ready() -> void:
    items = _load_index("items")
    ship_designs = _load_index("ships")
    npc_roles = _load_index("roles")
    systems = _load_index("systems")

func _load_index(key: String) -> Dictionary:
    var file := FileAccess.open(DATA_FILES[key], FileAccess.READ)
    if file == null:
        push_error("Missing data file: " + DATA_FILES[key])
        return {}
    var parsed: Variant = JSON.parse_string(file.get_as_text())
    var collection_key := {"items":"items", "ships":"ship_designs", "roles":"roles", "systems":"systems"}[key]
    if not parsed is Dictionary or not parsed.get(collection_key) is Array:
        push_error("Invalid data schema: " + DATA_FILES[key])
        return {}
    var index: Dictionary = {}
    for entry: Variant in parsed[collection_key]:
        if not entry is Dictionary or not entry.has("id") or index.has(str(entry.id)):
            push_error("Invalid or duplicate entry in " + DATA_FILES[key])
            continue
        index[str(entry.id)] = entry
    return index

func get_system(id: String) -> Dictionary:
    return systems.get(id, {})

func get_item(id: String) -> Dictionary:
    return items.get(id, {})
