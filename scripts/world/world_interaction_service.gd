extends RefCounted
class_name WorldInteractionService

static func find_nearest(position: Vector2, bodies: Array) -> Dictionary:
    var best_distance := INF
    var result := {"action":"","id":"","name":""}
    for node: Node in bodies:
        var action := ""
        var body_id := ""
        var body_name := ""
        var radius := 0.0
        if node is PlanetBody:
            var planet := node as PlanetBody
            action = "LAND"
            body_id = planet.body_id
            body_name = planet.body_name
            radius = planet.interaction_radius
        elif node is StationBody:
            var station := node as StationBody
            action = "DOCK"
            body_id = station.body_id
            body_name = station.body_name
            radius = station.interaction_radius
        else:
            continue
        var distance := position.distance_to(node.global_position)
        if distance <= radius and distance < best_distance:
            best_distance = distance
            result = {"action":action,"id":body_id,"name":body_name,"distance":distance}
    return result
