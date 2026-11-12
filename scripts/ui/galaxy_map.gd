extends Control
class_name GalaxyMapView

signal system_selected(system_id: String)

const EDGE_COLOR := Color(0.30,0.48,0.66,0.55)
const EDGE_ACTIVE_COLOR := Color(0.36,0.78,1.0,0.92)
const NODE_IDLE_COLOR := Color(0.30,0.43,0.58,0.95)
const NODE_LOCKED_COLOR := Color(0.19,0.25,0.34,0.78)
const NODE_CURRENT_COLOR := Color(0.45,0.86,1.0,1.0)
const NODE_SELECTED_COLOR := Color(1.0,0.76,0.38,1.0)
const MAP_MARGIN := Vector2(42.0,34.0)

var systems: Dictionary = {}
var current_system_id := ""
var selected_system_id := ""
var _pan_offset := Vector2.ZERO
var _zoom := 1.0
const MIN_ZOOM := 1.0
const MAX_ZOOM := 2.6
var _touch_positions: Dictionary = {}
var _active_touch := -1
var _last_touch_position := Vector2.ZERO
var _touch_moved := false
var _pinch_distance := 0.0
var _pinch_midpoint := Vector2.ZERO

func _ready() -> void:
    mouse_filter = Control.MOUSE_FILTER_STOP
    clip_contents = true
    custom_minimum_size = Vector2(460.0,220.0)

func configure(current_id: String, selected_id: String) -> void:
    current_system_id = current_id
    selected_system_id = selected_id
    queue_redraw()

func _system_point(system_id: String) -> Vector2:
    var definition: Dictionary = systems.get(system_id,{})
    var normalized: Array = definition.get("galaxy_position",[0.5,0.5])
    return MAP_MARGIN + Vector2(float(normalized[0]),float(normalized[1])) * (size-MAP_MARGIN*2.0)

func _clamp_pan(offset: Vector2) -> Vector2:
    var scaled_size := size * _zoom
    return Vector2(
        clampf(offset.x, minf(0.0, size.x-scaled_size.x), 0.0),
        clampf(offset.y, minf(0.0, size.y-scaled_size.y), 0.0)
    )

func pan_by(delta: Vector2) -> void:
    _pan_offset = _clamp_pan(_pan_offset + delta)
    queue_redraw()

func set_zoom(value: float, anchor: Vector2 = Vector2.ZERO) -> void:
    var map_anchor := (anchor-_pan_offset)/_zoom
    _zoom = clampf(value,MIN_ZOOM,MAX_ZOOM)
    _pan_offset = _clamp_pan(anchor-map_anchor*_zoom)
    queue_redraw()

func reset_view() -> void:
    _pan_offset = Vector2.ZERO
    _zoom = 1.0
    queue_redraw()

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO,size),Color(0.015,0.035,0.075,0.96),true)
    for grid_x in range(1,5):
        var x := size.x*float(grid_x)/5.0
        draw_line(Vector2(x,12),Vector2(x,size.y-12),Color(0.22,0.37,0.52,0.12),1.0)
    for grid_y in range(1,3):
        var y := size.y*float(grid_y)/3.0
        draw_line(Vector2(12,y),Vector2(size.x-12,y),Color(0.22,0.37,0.52,0.12),1.0)
    draw_set_transform(_pan_offset,0.0,Vector2.ONE*_zoom)
    var current: Dictionary = systems.get(current_system_id,{})
    var links: Array = current.get("links",[])
    for id: String in systems:
        var definition: Dictionary = systems[id]
        for neighbor: String in definition.get("links",[]):
            if not systems.has(neighbor) or id >= neighbor:
                continue
            var adjacent := id == current_system_id or neighbor == current_system_id
            draw_line(_system_point(id),_system_point(neighbor),EDGE_ACTIVE_COLOR if adjacent else EDGE_COLOR,2.4 if adjacent else 1.4)
    for id: String in systems:
        var definition: Dictionary = systems[id]
        var point := _system_point(id)
        var is_current := id == current_system_id
        var is_selected := id == selected_system_id
        var is_adjacent := links.has(id)
        var node_color := NODE_CURRENT_COLOR if is_current else NODE_SELECTED_COLOR if is_selected else NODE_IDLE_COLOR if is_adjacent else NODE_LOCKED_COLOR
        if is_current or is_selected:
            draw_circle(point,17.0,Color(node_color.r,node_color.g,node_color.b,0.16))
        draw_circle(point,10.0 if is_current or is_selected else 7.0,node_color)
        draw_arc(point,10.0 if is_current or is_selected else 7.0,0.0,TAU,32,Color(0.78,0.91,1.0,0.88),1.4)
        draw_string(ThemeDB.fallback_font,point+Vector2(-46.0,25.0),str(definition.get("name",id)),HORIZONTAL_ALIGNMENT_CENTER,92.0,12,Color(0.90,0.96,1.0,1.0) if is_adjacent or is_current else Color(0.54,0.64,0.75,0.78))
    draw_set_transform(Vector2.ZERO,0.0,Vector2.ONE)
    draw_string(ThemeDB.fallback_font,Vector2(12.0,size.y-10.0),"DRAG TO PAN  ·  PINCH TO ZOOM  ·  TAP TO SELECT",HORIZONTAL_ALIGNMENT_LEFT,-1,9,Color(0.72,0.84,0.96,0.9))

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
                _select_system_at((touch.position-_pan_offset)/_zoom)
            _touch_positions.erase(touch.index)
            if _touch_positions.size() < 2:
                _pinch_distance = 0.0
            _active_touch = -1
            if _touch_positions.size() == 1:
                var remaining: Vector2 = _touch_positions.values()[0]
                _active_touch = int(_touch_positions.keys()[0])
                _last_touch_position = remaining
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

func _select_system_at(map_position: Vector2) -> void:
    for id: String in systems:
        if _system_point(id).distance_to(map_position) <= 24.0:
            system_selected.emit(id)
            accept_event()
            return
