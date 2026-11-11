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

func _ready() -> void:
    mouse_filter = Control.MOUSE_FILTER_STOP
    custom_minimum_size = Vector2(460.0,220.0)

func configure(current_id: String, selected_id: String) -> void:
    current_system_id = current_id
    selected_system_id = selected_id
    queue_redraw()

func _system_point(system_id: String) -> Vector2:
    var definition: Dictionary = systems.get(system_id,{})
    var normalized: Array = definition.get("galaxy_position",[0.5,0.5])
    var usable := size - MAP_MARGIN * 2.0
    return MAP_MARGIN + Vector2(float(normalized[0]),float(normalized[1])) * usable

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO,size),Color(0.015,0.035,0.075,0.92),true)
    for grid_x in range(1,5):
        var x := size.x * float(grid_x) / 5.0
        draw_line(Vector2(x,12),Vector2(x,size.y-12),Color(0.22,0.37,0.52,0.12),1.0)
    for grid_y in range(1,3):
        var y := size.y * float(grid_y) / 3.0
        draw_line(Vector2(12,y),Vector2(size.x-12,y),Color(0.22,0.37,0.52,0.12),1.0)

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
        var name := str(definition.get("name",id))
        var label_color := Color(0.90,0.96,1.0,1.0) if is_adjacent or is_current else Color(0.54,0.64,0.75,0.78)
        draw_string(ThemeDB.fallback_font,point+Vector2(-46.0,25.0),name,HORIZONTAL_ALIGNMENT_CENTER,92.0,12,label_color)

    draw_string(ThemeDB.fallback_font,Vector2(12.0,size.y-10.0),"CYAN: CURRENT   BLUE: LINKED   GREY: OUT OF RANGE",HORIZONTAL_ALIGNMENT_LEFT,-1,9,Color(0.62,0.75,0.88,0.85))

func _gui_input(event: InputEvent) -> void:
    if not event is InputEventScreenTouch:
        return
    var touch := event as InputEventScreenTouch
    if not touch.pressed:
        return
    var current: Dictionary = systems.get(current_system_id,{})
    var links: Array = current.get("links",[])
    for id: String in systems:
        if id == current_system_id or not links.has(id):
            continue
        if _system_point(id).distance_to(touch.position) <= 24.0:
            system_selected.emit(id)
            accept_event()
            return
