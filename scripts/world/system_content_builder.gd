extends RefCounted
class_name SystemContentBuilder

const ASTEROID_SCENE: PackedScene = preload("res://scenes/asteroid.tscn")
const PLANET_SCENE: PackedScene = preload("res://scenes/planet_body.tscn")
const STATION_SCENE: PackedScene = preload("res://scenes/station.tscn")

static func build_system(container: Node2D, system_id: String, definition: Dictionary) -> Dictionary:
    var station_data: Array = definition.get("station_position",[480,270])
    var station_position := Vector2(float(station_data[0]),float(station_data[1]))
    if bool(definition.get("has_asteroids",false)):
        for point: Vector2 in [Vector2(520,250),Vector2(680,420),Vector2(860,310),Vector2(740,160)]:
            var asteroid := ASTEROID_SCENE.instantiate()
            asteroid.position = point
            container.add_child(asteroid)
    var planet_count := int(definition.get("planet_count",2))
    var moon_count := int(definition.get("moon_count",0))
    var planet_positions: Array[Vector2] = []
    for index in range(planet_count):
        var point := Vector2(150.0+float(index)*280.0,100.0+float(index%2)*330.0)
        planet_positions.append(point)
        var planet := PLANET_SCENE.instantiate() as PlanetBody
        planet.position = point
        planet.body_id = "%s_planet_%02d" % [system_id,index+1]
        planet.body_name = "%s %d" % [str(definition.get("name",system_id)),index+1]
        planet.body_radius = 30.0+float(index%3)*12.0
        planet.interaction_radius = planet.body_radius+76.0
        container.add_child(planet)
    for moon_index in range(moon_count):
        if planet_positions.is_empty():
            break
        var parent_position: Vector2 = planet_positions[moon_index%planet_positions.size()]
        var orbit_angle := float(moon_index)*2.399
        var moon := PLANET_SCENE.instantiate() as PlanetBody
        moon.position = parent_position+Vector2(cos(orbit_angle),sin(orbit_angle))*76.0
        moon.body_id = "%s_moon_%02d" % [system_id,moon_index+1]
        moon.body_name = "%s Moon %d" % [str(definition.get("name",system_id)),moon_index+1]
        moon.body_radius = 11.0+float(moon_index%3)*3.0
        moon.interaction_radius = moon.body_radius+76.0
        moon.body_color = Color(0.48,0.52,0.60)
        container.add_child(moon)
    var station := STATION_SCENE.instantiate() as StationBody
    station.position = station_position
    container.add_child(station)
    return {"station_position":station_position,"station":station}
