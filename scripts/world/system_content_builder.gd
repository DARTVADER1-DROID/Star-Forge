extends RefCounted
class_name SystemContentBuilder

const ASTEROID_SCENE: PackedScene = preload("res://scenes/asteroid.tscn")
const PLANET_SCENE: PackedScene = preload("res://scenes/planet_body.tscn")
const STATION_SCENE: PackedScene = preload("res://scenes/station.tscn")

static func build_system(container: Node2D, system_id: String, definition: Dictionary) -> Dictionary:
    var station_position := SystemLayout.station_position(definition)
    for raw_point: Variant in SystemLayout.asteroid_positions(definition):
        if not raw_point is Array or raw_point.size() < 2:
            continue
        var asteroid := ASTEROID_SCENE.instantiate()
        asteroid.position = Vector2(float(raw_point[0]),float(raw_point[1]))
        container.add_child(asteroid)

    for planet_data: Dictionary in SystemLayout.planet_definitions(definition):
        var planet := PLANET_SCENE.instantiate() as PlanetBody
        planet.body_id = str(planet_data.get("id","%s_planet" % system_id))
        planet.body_name = str(planet_data.get("name",planet.body_id))
        planet.position = _read_position(planet_data.get("position",[480,270]))
        planet.body_radius = float(planet_data.get("radius",32.0))
        planet.interaction_radius = float(planet_data.get("interaction_radius",planet.body_radius+76.0))
        planet.texture_name = str(planet_data.get("texture","planet_terran_64.png"))
        container.add_child(planet)

        for moon_data: Variant in planet_data.get("moons",[]):
            if not moon_data is Dictionary:
                continue
            var moon := PLANET_SCENE.instantiate() as PlanetBody
            moon.body_id = str(moon_data.get("id","%s_moon" % planet.body_id))
            moon.body_name = str(moon_data.get("name",moon.body_id))
            moon.position = planet.position+_read_position(moon_data.get("offset",[76,0]))
            moon.body_radius = float(moon_data.get("radius",12.0))
            moon.interaction_radius = float(moon_data.get("interaction_radius",moon.body_radius+76.0))
            moon.texture_name = str(moon_data.get("texture","planet_moon_32.png"))
            container.add_child(moon)

    var station := STATION_SCENE.instantiate() as StationBody
    station.position = station_position
    container.add_child(station)
    return {"station_position":station_position,"station":station}

static func _read_position(raw: Variant) -> Vector2:
    if raw is Array and raw.size() >= 2:
        return Vector2(float(raw[0]),float(raw[1]))
    return Vector2.ZERO
