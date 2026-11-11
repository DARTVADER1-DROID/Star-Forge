extends Control
class_name TouchHUD

signal steering_changed(direction: Vector2)
signal boost_changed(active: bool)
signal fire_changed(active: bool)
signal action_requested(action: String, value: String)

const JOYSTICK_RADIUS := 54.0
const JOYSTICK_DEADZONE := 0.12

var _active_touch := -1
var _joystick_origin := Vector2.ZERO
var _joystick_vector := Vector2.ZERO
var _status_label: Label
var _inventory_panel: PanelContainer
var _galaxy_panel: PanelContainer
var _recovery_panel: PanelContainer
var _inventory_rows: VBoxContainer
var _galaxy_buttons: VBoxContainer
var _selected_system := ""
var _gameplay_blocked := false

func _ready() -> void:
    mouse_filter = Control.MOUSE_FILTER_IGNORE
    _build_status()
    _build_controls()
    _build_inventory()
    _build_galaxy()
    _build_recovery()
    GameState.inventory_changed.connect(refresh_inventory)
    GameState.system_changed.connect(_refresh_galaxy)

func _build_status() -> void:
    _status_label = Label.new()
    _status_label.position = Vector2(18,12)
    _status_label.add_theme_font_size_override("font_size",16)
    _status_label.add_theme_color_override("font_color",Color(0.78,0.88,1.0))
    _status_label.mouse_filter = Control.MOUSE_FILTER_IGNORE
    add_child(_status_label)

func _build_controls() -> void:
    var boost := _button("BOOST",Vector2(-222,-94),Vector2(94,66),true)
    var fire := _button("FIRE",Vector2(-116,-94),Vector2(94,66),true)
    var loot := _button("LOOT",Vector2(16,-68),Vector2(76,52),false)
    var ftl := _button("FTL",Vector2(16,-128),Vector2(76,52),false)
    var bag := _button("BAG",Vector2(16,-188),Vector2(76,52),false)
    var save := _button("SAVE",Vector2(16,-248),Vector2(76,52),false)
    boost.button_down.connect(func(): boost_changed.emit(true))
    boost.button_up.connect(func(): boost_changed.emit(false))
    fire.button_down.connect(func(): fire_changed.emit(true))
    fire.button_up.connect(func(): fire_changed.emit(false))
    loot.pressed.connect(func(): action_requested.emit("LOOT",""))
    ftl.pressed.connect(func(): show_galaxy_map())
    bag.pressed.connect(func(): toggle_inventory())
    save.pressed.connect(func(): action_requested.emit("SAVE",""))

func _button(caption: String, offset: Vector2, dimensions: Vector2, right: bool) -> Button:
    var control := Button.new()
    control.text = caption
    control.focus_mode = Control.FOCUS_NONE
    control.anchor_left = 1.0 if right else 0.0
    control.anchor_right = control.anchor_left
    control.anchor_top = 1.0
    control.anchor_bottom = 1.0
    control.offset_left = offset.x
    control.offset_top = offset.y
    control.offset_right = offset.x+dimensions.x
    control.offset_bottom = offset.y+dimensions.y
    control.add_theme_font_size_override("font_size",17)
    control.add_theme_color_override("font_color",Color(0.92,0.96,1.0))
    control.add_theme_stylebox_override("normal",_button_style(Color(0.08,0.19,0.32,0.92)))
    control.add_theme_stylebox_override("pressed",_button_style(Color(0.12,0.39,0.58,0.98)))
    add_child(control)
    return control

func _button_style(fill: Color) -> StyleBoxFlat:
    var style := StyleBoxFlat.new()
    style.bg_color = fill
    style.border_color = Color(0.35,0.66,0.88,0.9)
    style.set_border_width_all(2)
    style.set_corner_radius_all(12)
    return style

func _input(event: InputEvent) -> void:
    if _gameplay_blocked:
        return
    if event is InputEventScreenTouch:
        var touch := event as InputEventScreenTouch
        if touch.pressed:
            _start_joystick(touch.index,touch.position)
        elif touch.index == _active_touch:
            _stop_joystick()
    elif event is InputEventScreenDrag:
        var drag := event as InputEventScreenDrag
        if drag.index == _active_touch:
            _move_joystick(drag.position)

func _start_joystick(id: int, point: Vector2) -> void:
    if _active_touch != -1:
        return
    var zone := Rect2(Vector2(0,size.y*0.34),Vector2(size.x*0.44,size.y*0.66))
    if not zone.has_point(point):
        return
    _active_touch = id
    _joystick_origin = point
    _move_joystick(point)

func _move_joystick(point: Vector2) -> void:
    _joystick_vector = (point-_joystick_origin).limit_length(JOYSTICK_RADIUS)/JOYSTICK_RADIUS
    if _joystick_vector.length()<JOYSTICK_DEADZONE:
        _joystick_vector = Vector2.ZERO
    steering_changed.emit(_joystick_vector)
    queue_redraw()

func _stop_joystick() -> void:
    _active_touch = -1
    _joystick_vector = Vector2.ZERO
    steering_changed.emit(Vector2.ZERO)
    queue_redraw()

func update_ship_status(hull: float,hull_max: float,armour: float,armour_max: float,energy: float,energy_max: float,credits: int,_inventory: Dictionary) -> void:
    _status_label.text = "HULL %03d/%03d   ARMOUR %03d/%03d   ENERGY %03d/%03d   CR %d" % [roundi(hull),roundi(hull_max),roundi(armour),roundi(armour_max),roundi(energy),roundi(energy_max),credits]

func _make_panel(title: String, dimensions: Vector2) -> Dictionary:
    var panel := PanelContainer.new()
    panel.set_anchors_and_offsets_preset(Control.PRESET_CENTER)
    panel.custom_minimum_size = dimensions
    panel.position = -dimensions/2.0
    panel.visible = false
    add_child(panel)
    var content := VBoxContainer.new()
    panel.add_child(content)
    var heading := Label.new()
    heading.text = title
    heading.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    content.add_child(heading)
    return {"panel":panel,"content":content}

func _build_inventory() -> void:
    var built := _make_panel("CARGO HOLD",Vector2(390,330))
    _inventory_panel = built.panel
    var content: VBoxContainer = built.content
    _inventory_rows = VBoxContainer.new()
    content.add_child(_inventory_rows)
    var close := Button.new()
    close.text = "CLOSE"
    close.pressed.connect(func(): _close_panels())
    content.add_child(close)
    refresh_inventory()

func refresh_inventory() -> void:
    if not is_instance_valid(_inventory_rows):
        return
    for child: Node in _inventory_rows.get_children():
        child.queue_free()
    for id: String in GameState.inventory:
        var item := ContentDatabase.get_item(id)
        var row := HBoxContainer.new()
        var label := Label.new()
        label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        label.text = "%s  x%d" % [item.get("name",id),GameState.inventory[id]]
        row.add_child(label)
        var sell := Button.new()
        sell.text = "SELL"
        sell.disabled = int(GameState.inventory[id]) <= 0
        sell.pressed.connect(func(): action_requested.emit("SELL",id))
        row.add_child(sell)
        _inventory_rows.add_child(row)

func toggle_inventory() -> void:
    _close_panels()
    _inventory_panel.show()
    _gameplay_blocked = true
    refresh_inventory()

func _build_galaxy() -> void:
    var built := _make_panel("GALAXY NAVIGATION",Vector2(500,360))
    _galaxy_panel = built.panel
    var content: VBoxContainer = built.content
    var hint := Label.new()
    hint.text = "Select a connected system. Each jump costs 5 Fuel."
    content.add_child(hint)
    _galaxy_buttons = VBoxContainer.new()
    content.add_child(_galaxy_buttons)
    var actions := HBoxContainer.new()
    content.add_child(actions)
    var jump := Button.new()
    jump.text = "JUMP"
    jump.pressed.connect(func(): action_requested.emit("JUMP",_selected_system))
    actions.add_child(jump)
    var close := Button.new()
    close.text = "CLOSE"
    close.pressed.connect(func(): _close_panels())
    actions.add_child(close)
    _refresh_galaxy(GameState.current_system_id)

func _refresh_galaxy(_system_id: String) -> void:
    if not is_instance_valid(_galaxy_buttons):
        return
    for child: Node in _galaxy_buttons.get_children():
        child.queue_free()
    var current := ContentDatabase.get_system(GameState.current_system_id)
    for id: String in ContentDatabase.systems:
        var definition := ContentDatabase.get_system(id)
        var button := Button.new()
        var connected := Array(current.get("links",[])).has(id)
        button.text = "%s%s" % [definition.get("name",id),"  • CURRENT" if id==GameState.current_system_id else ""]
        button.disabled = not connected
        button.button_pressed = id == _selected_system
        button.toggle_mode = true
        button.pressed.connect(func(): _selected_system = id)
        _galaxy_buttons.add_child(button)

func show_galaxy_map() -> void:
    _close_panels()
    _selected_system = ""
    _refresh_galaxy(GameState.current_system_id)
    _galaxy_panel.show()
    _gameplay_blocked = true

func _build_recovery() -> void:
    var built := _make_panel("SHIP DESTROYED",Vector2(320,190))
    _recovery_panel = built.panel
    var content: VBoxContainer = built.content
    var message := Label.new()
    message.text = "Emergency recovery restores a damaged ship."
    content.add_child(message)
    var recover := Button.new()
    recover.text = "RECOVER SHIP"
    recover.pressed.connect(func():
        _recovery_panel.hide()
        _gameplay_blocked = false
        action_requested.emit("RECOVER","")
    )
    content.add_child(recover)

func show_recovery_panel() -> void:
    _close_panels()
    _recovery_panel.show()
    _gameplay_blocked = true
    fire_changed.emit(false)
    boost_changed.emit(false)
    steering_changed.emit(Vector2.ZERO)

func _close_panels() -> void:
    if is_instance_valid(_inventory_panel): _inventory_panel.hide()
    if is_instance_valid(_galaxy_panel): _galaxy_panel.hide()
    if is_instance_valid(_recovery_panel): _recovery_panel.hide()
    _gameplay_blocked = false

func _draw() -> void:
    if _active_touch == -1:
        return
    draw_circle(_joystick_origin,JOYSTICK_RADIUS,Color(0.18,0.38,0.56,0.22))
    draw_arc(_joystick_origin,JOYSTICK_RADIUS,0.0,TAU,48,Color(0.48,0.74,0.94,0.78),3.0)
    draw_circle(_joystick_origin+_joystick_vector*JOYSTICK_RADIUS,23.0,Color(0.58,0.82,1.0,0.82))
