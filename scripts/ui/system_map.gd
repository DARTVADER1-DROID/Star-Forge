extends Control
class_name SystemMapView

signal body_selected(body_name: String)

const MAP_WIDTH := 960.0
const MAP_HEIGHT := 540.0
const MAP_MARGIN := Vector2(12.0,10.0)

var system_id := ""
var definition: Dictionary = {}
var ship_position := Vector2(480.0,270.0)
var selected_body := ""
var _map_scale := 0.35
var _map_offset := Vector2.ZERO
var _body_points: Dictionary = {}

func _ready() -> void:
    mouse_filter = Control.MOUSE_FILTER_STOP
    custom_minimum_size = Vector2(460.0,220.0)

func configure(id: String, system_data: Dictionary, ship: Vector2) -> void:
    system_id = id
    definition = system_data
    ship_position = ship
    _rebuild_body_points()
    queue_redraw()

func _rebuild_body_points() -> void:
    _body_points.clear()
    var station_data: Array = definition.get("station_position",[480,270])
    _body_points["STATION"] = Vector2(float(station_data[0]),float(station_data[1]))
    var planet_count := int(definition.get("planet_count",2))
    var moon_count := int(definition.get("moon_count",0))
    var planets: Array[Vector2] = []
    for index in range(planet_count):
        var point := Vector2(150.0+float(index)*280.0,100.0+float(index%2)*330.0)
        planets.append(point)
        _body_points["PLANET %d" % (index+1)] = point
    for moon_index in range(moon_count):
        if planets.is_empty():
            break
        var parent: Vector2 = planets[moon_index % planets.size()]
        var angle := float(moon_index)*2.399
        _body_points["MOON %d" % (moon_index+1)] = parent+Vector2(cos(angle),sin(angle))*76.0

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO,size),Color(0.015,0.035,0.075,0.92),true)
    for x_index in range(1,5):
        var x := size.x*float(x_index)/5.0
        draw_line(Vector2(x,8),Vector2(x,size.y-8),Color(0.22,0.37,0.52,0.12),1.0)
    for y_index in range(1,3):
        var y := size.y*float(y_index)/3.0
        draw_line(Vector2(8,y),Vector2(size.x-8,y),Color(0.22,0.37,0.52,0.12),1.0)

    _map_scale = minf((size.x-MAP_MARGIN.x*2.0)/MAP_WIDTH,(size.y-MAP_MARGIN.y*2.0)/MAP_HEIGHT)
    _map_offset = (size-Vector2(MAP_WIDTH,MAP_HEIGHT)*_map_scale)*0.5
    draw_set_transform(_map_offset,0.0,Vector2(_map_scale,_map_scale))
    draw_circle(Vector2(480,270),24.0,Color(1.0,0.72,0.32,0.16))
    draw_circle(Vector2(480,270),12.0,Color(1.0,0.82,0.48,1.0))
    draw_arc(Vector2(480,270),12.0,0.0,TAU,32,Color(1.0,0.92,0.72,1.0),2.0)

    var planet_count := int(definition.get("planet_count",2))
    var planets: Array[Vector2] = []
    for index in range(planet_count):
        var point := Vector2(150.0+float(index)*280.0,100.0+float(index%2)*330.0)
        planets.append(point)
        draw_arc(Vector2(480,270),point.distance_to(Vector2(480,270)),0.0,TAU,72,Color(0.35,0.55,0.72,0.18),1.0)
        var radius := 30.0+float(index%3)*12.0
        var body_color := Color(0.32,0.70,0.82) if index%2==0 else Color(0.68,0.50,0.88)
        draw_circle(point,radius,body_color)
        draw_arc(point,radius,0.0,TAU,32,Color(0.86,0.94,1.0,0.92),2.0)
        if selected_body == "PLANET %d" % (index+1):
            draw_arc(point,radius+8.0,0.0,TAU,32,Color(1.0,0.78,0.36,1.0),2.0)
        draw_string(ThemeDB.fallback_font,point+Vector2(-42.0,radius+18.0),"PLANET %d" % (index+1),HORIZONTAL_ALIGNMENT_CENTER,84.0,12,Color(0.88,0.94,1.0,0.95))

    var moon_count := int(definition.get("moon_count",0))
    for moon_index in range(moon_count):
        if planets.is_empty():
            break
        var parent: Vector2 = planets[moon_index % planets.size()]
        var angle := float(moon_index)*2.399
        var moon_point := parent+Vector2(cos(angle),sin(angle))*76.0
        draw_circle(moon_point,11.0+float(moon_index%3)*3.0,Color(0.52,0.58,0.68,1.0))
        if selected_body == "MOON %d" % (moon_index+1):
            draw_arc(moon_point,20.0,0.0,TAU,24,Color(1.0,0.78,0.36,1.0),2.0)

    if bool(definition.get("has_asteroids",false)):
        for asteroid_point in [Vector2(520,250),Vector2(680,420),Vector2(860,310),Vector2(740,160)]:
            draw_circle(asteroid_point,7.0,Color(0.58,0.60,0.64,0.95))

    var station_data: Array = definition.get("station_position",[480,270])
    var station := Vector2(float(station_data[0]),float(station_data[1]))
    draw_circle(station,20.0,Color(0.28,0.76,0.72,0.24))
    draw_arc(station,17.0,0.0,TAU,32,Color(0.42,0.94,0.84,1.0),3.0)
    draw_string(ThemeDB.fallback_font,station+Vector2(-35.0,31.0),"STATION",HORIZONTAL_ALIGNMENT_CENTER,70.0,12,Color(0.58,0.98,0.90,1.0))
    draw_circle(ship_position,8.0,Color(0.46,0.86,1.0,1.0))
    draw_arc(ship_position,14.0,0.0,TAU,32,Color(0.46,0.86,1.0,0.9),2.0)
    draw_set_transform(Vector2.ZERO,0.0,Vector2.ONE)
    draw_string(ThemeDB.fallback_font,Vector2(10.0,size.y-9.0),"LOCAL SYSTEM · TAP A PLANET OR STATION FOR DETAILS",HORIZONTAL_ALIGNMENT_LEFT,-1,9,Color(0.62,0.75,0.88,0.85))

func _gui_input(event: InputEvent) -> void:
    if not event is InputEventScreenTouch:
        return
    var touch := event as InputEventScreenTouch
    if not touch.pressed:
        return
    var world_point := (touch.position-_map_offset)/_map_scale
    var closest_name := ""
    var closest_distance := 38.0
    for body_name: String in _body_points:
        var distance := world_point.distance_to(_body_points[body_name])
        if distance < closest_distance:
            closest_distance = distance
            closest_name = body_name
    if not closest_name.is_empty():
        selected_body = closest_name
        body_selected.emit(closest_name)
        queue_redraw()
        accept_event()
