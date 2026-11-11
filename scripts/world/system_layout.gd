extends RefCounted
class_name SystemLayout

static func planet_definitions(definition: Dictionary) -> Array[Dictionary]:
    var result: Array[Dictionary] = []
    var configured: Variant = definition.get("bodies",[])
    if configured is Array and not configured.is_empty():
        for entry: Variant in configured:
            if entry is Dictionary:
                result.append(entry)
        return result

    var system_id := str(definition.get("id","system"))
    var system_name := str(definition.get("name",system_id))
    var count := int(definition.get("planet_count",2))
    var moon_count := int(definition.get("moon_count",0))
    for index in range(count):
        var moons: Array[Dictionary] = []
        for moon_index in range(moon_count):
            if moon_index % maxi(1,count) != index:
                continue
            var angle := float(moon_index)*2.399
            moons.append({
                "id":"%s_moon_%02d" % [system_id,moon_index+1],
                "name":"%s Moon %d" % [system_name,moon_index+1],
                "offset":[cos(angle)*76.0,sin(angle)*76.0],
                "radius":11.0+float(moon_index%3)*3.0,
                "texture":"planet_moon_32.png"
            })
        result.append({
            "id":"%s_planet_%02d" % [system_id,index+1],
            "name":"%s %d" % [system_name,index+1],
            "position":[150.0+float(index)*280.0,100.0+float(index%2)*330.0],
            "radius":30.0+float(index%3)*12.0,
            "interaction_radius":106.0+float(index%3)*12.0,
            "texture":"planet_terran_64.png" if index%2==0 else "planet_gas_96.png",
            "moons":moons
        })
    return result

static func asteroid_positions(definition: Dictionary) -> Array:
    var configured: Variant = definition.get("asteroid_positions",null)
    if configured is Array:
        return configured
    if not bool(definition.get("has_asteroids",false)):
        return []
    return [[520,250],[680,420],[860,310],[740,160]]

static func station_position(definition: Dictionary) -> Vector2:
    var raw: Array = definition.get("station_position",[480,270])
    return Vector2(float(raw[0]),float(raw[1]))
