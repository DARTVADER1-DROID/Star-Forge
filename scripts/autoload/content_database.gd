extends Node

const DATA_FILES: Dictionary = {
    "items": "res://data/economy/items.json",
    "ships": "res://data/ships/designs.json",
    "roles": "res://data/ships/roles.json",
    "systems": "res://data/world/systems.json"
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
    var file := FileAccess.open(DATA_FILES[key],FileAccess.READ)
    if file == null:
        push_error("Missing data file: " + str(DATA_FILES[key]))
        return {}
    var parsed: Variant = JSON.parse_string(file.get_as_text())
    var collection_key: String = {"items":"items","ships":"ship_designs","roles":"roles","systems":"systems"}[key]
    if not parsed is Dictionary or not (parsed.get(collection_key) is Array):
        push_error("Invalid data schema: " + str(DATA_FILES[key]))
        return {}
    var index: Dictionary = {}
    for entry: Variant in parsed[collection_key]:
        if not entry is Dictionary or not entry.has("id") or index.has(str(entry.get("id",""))):
            push_error("Invalid or duplicate entry in " + str(DATA_FILES[key]))
            continue
        index[str(entry.get("id"))] = entry
    return index

func get_system(id: String) -> Dictionary:
    return systems.get(id,{})

func get_item(id: String) -> Dictionary:
    return items.get(id,{})

func validate_world_catalog() -> Array[String]:
    var failures: Array[String] = []
    for system_id: String in systems:
        var system: Dictionary = systems[system_id]
        if str(system.get("id","")) != system_id:
            failures.append("System key/id mismatch: " + system_id)
        for linked_id: String in system.get("links",[]):
            if not systems.has(linked_id):
                failures.append("System %s links to unknown system %s" % [system_id,linked_id])
        var body_ids: Dictionary = {}
        for planet: Dictionary in SystemLayout.planet_definitions(system):
            var planet_id := str(planet.get("id",""))
            if planet_id.is_empty() or body_ids.has(planet_id):
                failures.append("Missing or duplicate planet ID in system " + system_id)
            body_ids[planet_id] = true
            var texture := str(planet.get("texture",""))
            if not texture.is_empty() and not AssetLibrary.has_asset(texture):
                failures.append("Planet texture is not registered: " + texture)
            for moon: Variant in planet.get("moons",[]):
                if not moon is Dictionary:
                    failures.append("Invalid moon definition in " + system_id)
                    continue
                var moon_id := str(moon.get("id",""))
                if moon_id.is_empty() or body_ids.has(moon_id):
                    failures.append("Missing or duplicate moon ID in system " + system_id)
                body_ids[moon_id] = true
                var moon_texture := str(moon.get("texture",""))
                if not moon_texture.is_empty() and not AssetLibrary.has_asset(moon_texture):
                    failures.append("Moon texture is not registered: " + moon_texture)
    return failures
