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
var _base_map_offset := Vector2.ZERO
var _zoom := 1.0
var _pan_offset := Vector2.ZERO
var _touch_positions: Dictionary = {}
var _active_touch := -1
var _last_touch_position := Vector2.ZERO
var _touch_moved := false
var _pinch_distance := 0.0
var _pinch_midpoint := Vector2.ZERO
var _body_points: Dictionary = {}

func _ready() -> void:
    mouse_filter = Control.MOUSE_FILTER_STOP
    clip_contents = true
    custom_minimum_size = Vector2(460.0,220.0)

func configure(id: String, system_data: Dictionary, ship: Vector2) -> void:
    if system_id != id:
        system_id = id
        definition = system_data
        selected_body = ""
        _zoom = 1.0
        _pan_offset = Vector2.ZERO
        _rebuild_body_points()
    if ship_position.distance_squared_to(ship) > 1.0:
        ship_position = ship
        queue_redraw()

func _rebuild_body_points() -> void:
    _body_points.clear()
    _body_points["STATION"] = SystemLayout.station_position(definition)
    for planet_data: Dictionary in SystemLayout.planet_definitions(definition):
        var planet_name := str(planet_data.get("name",planet_data.get("id","PLANET")))
        var planet_position := _read_position(planet_data.get("position",[480,270]))
        _body_points[planet_name] = planet_position
        for moon_data: Variant in planet_data.get("moons",[]):
            if moon_data is Dictionary:
                var moon_name := str(moon_data.get("name",moon_data.get("id","MOON")))
                _body_points[moon_name] = planet_position+_read_position(moon_data.get("offset",[76,0]))

static func _read_position(raw: Variant) -> Vector2:
    if raw is Array and raw.size() >= 2:
        return Vector2(float(raw[0]),float(raw[1]))
    return Vector2.ZERO

func _update_map_transform() -> void:
    _map_scale = minf((size.x-MAP_MARGIN.x*2.0)/MAP_WIDTH,(size.y-MAP_MARGIN.y*2.0)/MAP_HEIGHT)
    _base_map_offset = (size-Vector2(MAP_WIDTH,MAP_HEIGHT)*_map_scale)*0.5
    var content_size := Vector2(MAP_WIDTH,MAP_HEIGHT)*_map_scale*_zoom
    var min_pan := Vector2(
        minf(0.0,size.x-_base_map_offset.x-content_size.x),
        minf(0.0,size.y-_base_map_offset.y-content_size.y)
    )
    var max_pan := Vector2(minf(0.0,-_base_map_offset.x),minf(0.0,-_base_map_offset.y))
    _pan_offset = Vector2(clampf(_pan_offset.x,min_pan.x,max_pan.x),clampf(_pan_offset.y,min_pan.y,max_pan.y))
    _map_offset = _base_map_offset+_pan_offset

func pan_by(delta: Vector2) -> void:
    _pan_offset += delta
    _update_map_transform()
    queue_redraw()

func set_zoom(value: float, anchor: Vector2 = Vector2.ZERO) -> void:
    _update_map_transform()
    var map_anchor := (anchor-_map_offset)/(_map_scale*_zoom)
    _zoom = clampf(value,1.0,3.0)
    _pan_offset = anchor-map_anchor*(_map_scale*_zoom)-_base_map_offset
    _update_map_transform()
    queue_redraw()

func reset_view() -> void:
    _zoom = 1.0
    _pan_offset = Vector2.ZERO
    _update_map_transform()
    queue_redraw()

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO,size),Color(0.015,0.035,0.075,0.92),true)
    for x_index in range(1,5):
        var x := size.x*float(x_index)/5.0
        draw_line(Vector2(x,8),Vector2(x,size.y-8),Color(0.22,0.37,0.52,0.12),1.0)
    for y_index in range(1,3):
        var y := size.y*float(y_index)/3.0
        draw_line(Vector2(8,y),Vector2(size.x-8,y),Color(0.22,0.37,0.52,0.12),1.0)

    _update_map_transform()
    draw_set_transform(_map_offset,0.0,Vector2(_map_scale*_zoom,_map_scale*_zoom))
    var star_texture := AssetLibrary.load_texture("star_yellow_64.png")
    if star_texture != null:
        draw_texture_rect(star_texture,Rect2(Vector2(448,238),Vector2(64,64)),false)
    else:
        draw_circle(Vector2(480,270),24.0,Color(1.0,0.72,0.32,0.16))
        draw_circle(Vector2(480,270),12.0,Color(1.0,0.82,0.48,1.0))
        draw_arc(Vector2(480,270),12.0,0.0,TAU,32,Color(1.0,0.92,0.72,1.0),2.0)

    for planet_data: Dictionary in SystemLayout.planet_definitions(definition):
        var point := _read_position(planet_data.get("position",[480,270]))
        var radius := float(planet_data.get("radius",32.0))
        var planet_name := str(planet_data.get("name",planet_data.get("id","PLANET")))
        var orbit_radius := point.distance_to(Vector2(480,270))
        var orbit_texture := AssetLibrary.load_texture("orbit_ring_96.png")
        if orbit_texture != null:
            draw_texture_rect(orbit_texture,Rect2(Vector2(480-orbit_radius,270-orbit_radius),Vector2(orbit_radius*2.0,orbit_radius*2.0)),false)
        else:
            draw_arc(Vector2(480,270),orbit_radius,0.0,TAU,72,Color(0.35,0.55,0.72,0.18),1.0)
        var planet_texture := AssetLibrary.load_texture(str(planet_data.get("texture","planet_terran_64.png")))
        if planet_texture != null:
            draw_texture_rect(planet_texture,Rect2(point-Vector2(radius,radius),Vector2(radius*2.0,radius*2.0)),false)
        else:
            draw_circle(point,radius,Color(0.32,0.70,0.82))
            draw_arc(point,radius,0.0,TAU,32,Color(0.86,0.94,1.0,0.92),2.0)
        if selected_body == planet_name:
            draw_arc(point,radius+8.0,0.0,TAU,32,Color(1.0,0.78,0.36,1.0),2.0)
        draw_string(ThemeDB.fallback_font,point+Vector2(-110.0,radius+30.0),planet_name,HORIZONTAL_ALIGNMENT_CENTER,220.0,28,Color(0.88,0.94,1.0,0.95))
        for moon_data: Variant in planet_data.get("moons",[]):
            if not moon_data is Dictionary:
                continue
            var moon_name := str(moon_data.get("name",moon_data.get("id","MOON")))
            var moon_point := point+_read_position(moon_data.get("offset",[76,0]))
            var moon_size := float(moon_data.get("radius",12.0))*2.0
            var moon_texture := AssetLibrary.load_texture(str(moon_data.get("texture","planet_moon_32.png")))
            if moon_texture != null:
                draw_texture_rect(moon_texture,Rect2(moon_point-Vector2.ONE*moon_size*0.5,Vector2.ONE*moon_size),false)
            else:
                draw_circle(moon_point,moon_size*0.5,Color(0.52,0.58,0.68,1.0))
            if selected_body == moon_name:
                draw_arc(moon_point,moon_size*0.5+7.0,0.0,TAU,24,Color(1.0,0.78,0.36,1.0),2.0)

    var asteroid_texture := AssetLibrary.load_texture("asteroid_small_01.png")
    for raw_point: Variant in SystemLayout.asteroid_positions(definition):
        var asteroid_point := _read_position(raw_point)
        if asteroid_texture != null:
            draw_texture_rect(asteroid_texture,Rect2(asteroid_point-Vector2(10,10),Vector2(20,20)),false)
        else:
            draw_circle(asteroid_point,7.0,Color(0.58,0.60,0.64,0.95))

    var station := SystemLayout.station_position(definition)
    var station_texture := AssetLibrary.load_texture("station_ring_hub.png")
    if station_texture != null:
        draw_texture_rect(station_texture,Rect2(station-Vector2(30,30),Vector2(60,60)),false)
    else:
        draw_circle(station,20.0,Color(0.28,0.76,0.72,0.24))
        draw_arc(station,17.0,0.0,TAU,32,Color(0.42,0.94,0.84,1.0),3.0)
    draw_string(ThemeDB.fallback_font,station+Vector2(-90.0,52.0),"STATION",HORIZONTAL_ALIGNMENT_CENTER,180.0,28,Color(0.58,0.98,0.90,1.0))
    draw_circle(ship_position,8.0,Color(0.46,0.86,1.0,1.0))
    draw_arc(ship_position,14.0,0.0,TAU,32,Color(0.46,0.86,1.0,0.9),2.0)
    draw_set_transform(Vector2.ZERO,0.0,Vector2.ONE)
    draw_string(ThemeDB.fallback_font,Vector2(10.0,size.y-9.0),"LOCAL SYSTEM · TAP A PLANET OR STATION FOR DETAILS",HORIZONTAL_ALIGNMENT_LEFT,-1,9,Color(0.62,0.75,0.88,0.85))

func _gui_input(event: InputEvent) -> void:
    if event is InputEventScreenTouch:
        var touch := event as InputEventScreenTouch
        if touch.pressed:
            _touch_positions[touch.index] = touch.position
            if _touch_positions.size() == 1:
                _active_touch = touch.index
                _last_touch_position = touch.position
                _touch_moved = false
            elif _touch_positions.size() == 2:
                var points: Array = _touch_positions.values()
                var first_point: Vector2 = points[0]
                var second_point: Vector2 = points[1]
                _pinch_distance = first_point.distance_to(second_point)
                _pinch_midpoint = (first_point+second_point)*0.5
            accept_event()
        else:
            if touch.index == _active_touch and _touch_positions.size() == 1 and not _touch_moved:
                _select_body_at(touch.position)
            _touch_positions.erase(touch.index)
            if _touch_positions.size() < 2:
                _pinch_distance = 0.0
            _active_touch = -1
            if _touch_positions.size() == 1:
                _active_touch = int(_touch_positions.keys()[0])
                _last_touch_position = _touch_positions.values()[0]
                _touch_moved = true
            accept_event()
    elif event is InputEventScreenDrag:
        var drag := event as InputEventScreenDrag
        if not _touch_positions.has(drag.index):
            return
        _touch_positions[drag.index] = drag.position
        if _touch_positions.size() >= 2:
            var points: Array = _touch_positions.values()
            var first_point: Vector2 = points[0]
            var second_point: Vector2 = points[1]
            var midpoint := (first_point+second_point)*0.5
            var distance := first_point.distance_to(second_point)
            if _pinch_distance > 1.0 and distance > 1.0:
                set_zoom(_zoom*distance/_pinch_distance,_pinch_midpoint)
            pan_by(midpoint-_pinch_midpoint)
            _pinch_distance = distance
            _pinch_midpoint = midpoint
            _touch_moved = true
        elif drag.index == _active_touch:
            var movement := drag.position-_last_touch_position
            if movement.length() > 2.0:
                _touch_moved = true
                pan_by(movement)
            _last_touch_position = drag.position
        accept_event()
    elif event is InputEventMagnifyGesture:
        var gesture := event as InputEventMagnifyGesture
        set_zoom(_zoom*gesture.factor,gesture.position)
        accept_event()

func _select_body_at(screen_point: Vector2) -> void:
    _update_map_transform()
    var world_point := (screen_point-_map_offset)/(_map_scale*_zoom)
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
