extends Control
class_name TouchHUD

signal steering_changed(direction: Vector2)
signal boost_changed(active: bool)
signal fire_changed(active: bool)
signal action_requested(action: String, value: String)

const JOYSTICK_RADIUS := 54.0
const JOYSTICK_CENTER_X := 126.0
const JOYSTICK_BOTTOM_MARGIN := 112.0
const JOYSTICK_DEADZONE := 0.12
const JOYSTICK_ZONE_WIDTH := 0.44
const JOYSTICK_ZONE_TOP_RATIO := 0.34
const GALAXY_MAP_SCRIPT: Script = preload("res://scripts/ui/galaxy_map.gd")
const SYSTEM_MAP_SCRIPT: Script = preload("res://scripts/ui/system_map.gd")

var _active_touch := -1
var _joystick_origin := Vector2.ZERO
var _joystick_vector := Vector2.ZERO
var _status_label: Label
var _inventory_panel: PanelContainer
var _galaxy_panel: PanelContainer
var _recovery_panel: PanelContainer
var _inventory_rows: VBoxContainer
var _galaxy_map: GalaxyMapView
var _system_map: SystemMapView
var _galaxy_hint: Label
var _galaxy_tab: Button
var _system_tab: Button
var _transit_overlay: ColorRect
var _selected_system := ""
var _jump_button: Button
var _selected_item_id := "metal"
var _selected_quantity := 1
var _quantity_label: Label
var _npc_loot_button: Button
var _land_button: Button
var _planet_services_panel: PanelContainer
var _planet_title_label: Label
var _planet_result_label: Label
var _nearby_planet_id := ""
var _nearby_planet_name := ""
var _gameplay_blocked := false
var _station_access := false

func _ready() -> void:
    set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    mouse_filter = Control.MOUSE_FILTER_IGNORE
    z_index = 100
    _build_status()
    _build_controls()
    _build_inventory()
    _build_galaxy()
    _build_planet_services()
    _build_recovery()
    queue_redraw()
    GameState.inventory_changed.connect(refresh_inventory)
    GameState.system_changed.connect(_refresh_galaxy)

func _build_status() -> void:
    _status_label = Label.new()
    _status_label.position = Vector2(18,10)
    _status_label.size = Vector2(700,48)
    _status_label.add_theme_font_size_override("font_size",13)
    _status_label.add_theme_color_override("font_color",Color(0.78,0.88,1.0))
    _status_label.mouse_filter = Control.MOUSE_FILTER_IGNORE
    add_child(_status_label)

func _build_controls() -> void:
    var boost := _button("BOOST",Vector2(-218,-100),Vector2(96,72),true)
    var fire := _button("FIRE",Vector2(-110,-100),Vector2(96,72),true)
    var loot := _button("LOOT",Vector2(-100,14),Vector2(88,52),true,true)
    var ftl := _button("FTL",Vector2(-108,-184),Vector2(96,60),true)
    _land_button = _button("LAND",Vector2(-218,-184),Vector2(96,60),true)
    _land_button.visible = false
    _land_button.pressed.connect(func(): action_requested.emit("LAND",_nearby_planet_id))
    var bag := _button("BAG",Vector2(-100,134),Vector2(88,52),true,true)
    var save := _button("SAVE",Vector2(-100,194),Vector2(88,52),true,true)
    boost.button_down.connect(func(): boost_changed.emit(true))
    boost.button_up.connect(func(): boost_changed.emit(false))
    fire.button_down.connect(func(): fire_changed.emit(true))
    fire.button_up.connect(func(): fire_changed.emit(false))
    loot.pressed.connect(func(): action_requested.emit("LOOT",""))
    ftl.pressed.connect(func(): show_galaxy_map())
    bag.pressed.connect(func(): toggle_inventory())
    save.pressed.connect(func(): action_requested.emit("SAVE",""))

func _button(caption: String, offset: Vector2, dimensions: Vector2, right: bool, top: bool = false) -> Button:
    var control := Button.new()
    control.name = caption + "Button"
    control.text = caption
    control.focus_mode = Control.FOCUS_NONE
    control.mouse_filter = Control.MOUSE_FILTER_STOP
    control.custom_minimum_size = dimensions
    control.anchor_left = 1.0 if right else 0.0
    control.anchor_right = control.anchor_left
    control.anchor_top = 0.0 if top else 1.0
    control.anchor_bottom = control.anchor_top
    control.offset_left = offset.x
    control.offset_top = offset.y
    control.offset_right = offset.x+dimensions.x
    control.offset_bottom = offset.y+dimensions.y
    control.add_theme_font_size_override("font_size",17)
    control.add_theme_color_override("font_color",Color(0.92,0.96,1.0))
    var normal_asset := "ui_button_9slice_normal.png"
    var pressed_asset := "ui_button_9slice_pressed.png"
    match caption:
        "BOOST":
            normal_asset = "ui_btn_boost_up.png"
            pressed_asset = "ui_btn_boost_down.png"
        "FIRE":
            normal_asset = "ui_btn_fire_up.png"
            pressed_asset = "ui_btn_fire_down.png"
        "LOOT":
            normal_asset = "ui_btn_loot_up.png"
            pressed_asset = "ui_btn_loot_down.png"
    control.add_theme_stylebox_override("normal",_button_texture_style(normal_asset))
    control.add_theme_stylebox_override("pressed",_button_texture_style(pressed_asset))
    control.add_theme_stylebox_override("hover",_button_style(Color(0.10,0.28,0.44,0.96)))
    add_child(control)
    return control

func _button_texture_style(asset_name: String) -> StyleBox:
    var texture := AssetLibrary.load_texture(asset_name)
    if texture == null:
        return _button_style(Color(0.08,0.19,0.32,0.92))
    var style := StyleBoxTexture.new()
    style.texture = texture
    style.texture_margin_left = 9.0
    style.texture_margin_top = 9.0
    style.texture_margin_right = 9.0
    style.texture_margin_bottom = 9.0
    style.axis_stretch_horizontal = StyleBoxTexture.AXIS_STRETCH_MODE_STRETCH
    style.axis_stretch_vertical = StyleBoxTexture.AXIS_STRETCH_MODE_STRETCH
    return style

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
    var center := _joystick_center()
    var zone := Rect2(
        Vector2(0.0,size.y*JOYSTICK_ZONE_TOP_RATIO),
        Vector2(size.x*JOYSTICK_ZONE_WIDTH,size.y*(1.0-JOYSTICK_ZONE_TOP_RATIO))
    )
    if not zone.has_point(point):
        return
    _active_touch = id
    _joystick_origin = center
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

func update_ship_status(hull: float,hull_max: float,armour: float,armour_max: float,energy: float,energy_max: float,credits: int,_inventory: Dictionary,fuel: int,system_name: String,station_distance: float) -> void:
    var can_sell := station_distance <= 90.0
    if can_sell != _station_access:
        _station_access = can_sell
        refresh_inventory()
    var station_hint := "DOCKED RANGE" if can_sell else "STATION %.0f m" % station_distance
    _status_label.text = "HULL %03d/%03d   ARMOUR %03d/%03d   ENERGY %03d/%03d\n%s   FUEL %02d   CR %d   %s" % [roundi(hull),roundi(hull_max),roundi(armour),roundi(armour_max),roundi(energy),roundi(energy_max),system_name,fuel,credits,station_hint]

func _make_panel(title: String, dimensions: Vector2) -> Dictionary:
    var panel := PanelContainer.new()
    panel.set_anchors_and_offsets_preset(Control.PRESET_CENTER)
    panel.custom_minimum_size = dimensions
    panel.position = -dimensions/2.0
    panel.visible = false
    add_child(panel)
    var panel_texture := AssetLibrary.load_texture("ui_panel_9slice.png")
    if panel_texture != null:
        var panel_style := StyleBoxTexture.new()
        panel_style.texture = panel_texture
        panel_style.texture_margin_left = 18.0
        panel_style.texture_margin_top = 18.0
        panel_style.texture_margin_right = 18.0
        panel_style.texture_margin_bottom = 18.0
        panel.add_theme_stylebox_override("panel",panel_style)
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
    var quantity_row := HBoxContainer.new()
    content.add_child(quantity_row)
    var decrease := Button.new()
    decrease.text = "- QTY"
    decrease.pressed.connect(func():
        _selected_quantity = maxi(1,_selected_quantity-1)
        _update_quantity_label()
    )
    quantity_row.add_child(decrease)
    _quantity_label = Label.new()
    _quantity_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    _quantity_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    quantity_row.add_child(_quantity_label)
    var increase := Button.new()
    increase.text = "+ QTY"
    increase.pressed.connect(func():
        _selected_quantity = mini(int(GameState.inventory.get(_selected_item_id,0)),_selected_quantity+1)
        _selected_quantity = maxi(1,_selected_quantity)
        _update_quantity_label()
    )
    quantity_row.add_child(increase)
    var jettison := Button.new()
    jettison.text = "JETTISON SELECTED"
    jettison.pressed.connect(func():
        action_requested.emit("JETTISON","%s:%d" % [_selected_item_id,_selected_quantity])
    )
    content.add_child(jettison)
    _npc_loot_button = Button.new()
    _npc_loot_button.text = "NPC LOOT: OFF"
    _npc_loot_button.toggle_mode = true
    _npc_loot_button.pressed.connect(func(): action_requested.emit("NPC_LOOT",""))
    content.add_child(_npc_loot_button)
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
        var select := Button.new()
        select.text = "SELECT"
        select.button_pressed = id == _selected_item_id
        select.toggle_mode = true
        select.pressed.connect(func():
            _selected_item_id = id
            _selected_quantity = maxi(1,int(GameState.inventory.get(id,0)))
            _update_quantity_label()
            refresh_inventory()
        )
        row.add_child(select)
        var sell := Button.new()
        sell.text = "SELL" if _station_access else "DOCK"
        sell.disabled = int(GameState.inventory[id]) <= 0 or not _station_access
        sell.pressed.connect(func(): action_requested.emit("SELL",id))
        row.add_child(sell)
        _inventory_rows.add_child(row)
    _update_quantity_label()

func _update_quantity_label() -> void:
    if not is_instance_valid(_quantity_label):
        return
    var available := int(GameState.inventory.get(_selected_item_id,0))
    _selected_quantity = clampi(_selected_quantity,1,maxi(1,available))
    _quantity_label.text = "%s  %d/%d" % [_selected_item_id,_selected_quantity,available]

func set_npc_loot_enabled(enabled: bool) -> void:
    if is_instance_valid(_npc_loot_button):
        _npc_loot_button.button_pressed = enabled
        _npc_loot_button.text = "NPC LOOT: ON" if enabled else "NPC LOOT: OFF"

func toggle_inventory() -> void:
    _close_panels()
    _inventory_panel.show()
    _gameplay_blocked = true
    refresh_inventory()

func _build_galaxy() -> void:
    var built := _make_panel("GALAXY NAVIGATION",Vector2(500,360))
    _galaxy_panel = built.panel
    var content: VBoxContainer = built.content
    _galaxy_hint = Label.new()
    _galaxy_hint.text = "Choose an adjacent star system · 5 Fuel per jump"
    _galaxy_hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    content.add_child(_galaxy_hint)
    var tabs := HBoxContainer.new()
    tabs.alignment = BoxContainer.ALIGNMENT_CENTER
    content.add_child(tabs)
    _galaxy_tab = Button.new()
    _galaxy_tab.text = "GALAXY"
    _galaxy_tab.toggle_mode = true
    _galaxy_tab.button_pressed = true
    _galaxy_tab.pressed.connect(_show_galaxy_view)
    tabs.add_child(_galaxy_tab)
    _system_tab = Button.new()
    _system_tab.text = "SYSTEM"
    _system_tab.toggle_mode = true
    _system_tab.pressed.connect(_show_system_view)
    tabs.add_child(_system_tab)
    _galaxy_map = GALAXY_MAP_SCRIPT.new() as GalaxyMapView
    _galaxy_map.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    _galaxy_map.size_flags_vertical = Control.SIZE_EXPAND_FILL
    _galaxy_map.custom_minimum_size = Vector2(460,220)
    _galaxy_map.system_selected.connect(_on_map_system_selected)
    content.add_child(_galaxy_map)
    _system_map = SYSTEM_MAP_SCRIPT.new() as SystemMapView
    _system_map.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    _system_map.size_flags_vertical = Control.SIZE_EXPAND_FILL
    _system_map.custom_minimum_size = Vector2(460,220)
    _system_map.body_selected.connect(_on_map_body_selected)
    _system_map.visible = false
    content.add_child(_system_map)
    var actions := HBoxContainer.new()
    content.add_child(actions)
    var jump := Button.new()
    _jump_button = jump
    jump.text = "JUMP"
    jump.pressed.connect(func(): action_requested.emit("JUMP",_selected_system))
    actions.add_child(jump)
    var close := Button.new()
    close.text = "CLOSE"
    close.pressed.connect(func(): _close_panels())
    actions.add_child(close)
    _refresh_galaxy(GameState.current_system_id)

func _refresh_galaxy(_system_id: String) -> void:
    if not is_instance_valid(_galaxy_map):
        return
    var fuel := int(GameState.inventory.get("fuel",0))
    _galaxy_hint.text = "CURRENT: %s    ·    FUEL: %d    ·    JUMP COST: %d" % [
        str(ContentDatabase.get_system(GameState.current_system_id).get("name",GameState.current_system_id)),
        fuel,
        GameState.FTL_FUEL_COST
    ]
    _galaxy_map.systems = ContentDatabase.systems
    _galaxy_map.configure(GameState.current_system_id,_selected_system)
    update_system_map(Vector2(480.0,270.0))
    if is_instance_valid(_jump_button):
        _jump_button.disabled = not GameState.can_jump_to(_selected_system)

func _on_map_system_selected(system_id: String) -> void:
    _selected_system = system_id
    _refresh_galaxy(GameState.current_system_id)

func _show_galaxy_view() -> void:
    _galaxy_map.show()
    _system_map.hide()
    _galaxy_tab.button_pressed = true
    _system_tab.button_pressed = false
    _galaxy_hint.text = "Select a connected system · 5 Fuel per jump"

func _show_system_view() -> void:
    _galaxy_map.hide()
    _system_map.show()
    _galaxy_tab.button_pressed = false
    _system_tab.button_pressed = true
    update_system_map(Vector2(480.0,270.0))

func update_system_map(ship_position: Vector2) -> void:
    if not is_instance_valid(_system_map):
        return
    _system_map.configure(GameState.current_system_id,ContentDatabase.get_system(GameState.current_system_id),ship_position)

func _on_map_body_selected(body_name: String) -> void:
    _galaxy_hint.text = "SELECTED: " + body_name

func show_galaxy_map() -> void:
    _close_panels()
    _selected_system = ""
    _show_galaxy_view()
    _refresh_galaxy(GameState.current_system_id)
    _galaxy_panel.show()
    _gameplay_blocked = true

func hide_galaxy_map() -> void:
    if is_instance_valid(_galaxy_panel):
        _galaxy_panel.hide()
    _gameplay_blocked = false

func set_nearby_planet(planet_id: String, planet_name: String) -> void:
    _nearby_planet_id = planet_id
    _nearby_planet_name = planet_name
    if is_instance_valid(_land_button):
        _land_button.visible = not planet_id.is_empty()
        _land_button.tooltip_text = "Land at " + planet_name if not planet_id.is_empty() else ""

func _build_planet_services() -> void:
    var built := _make_panel("PLANETARY SERVICES",Vector2(470,340))
    _planet_services_panel = built.panel
    _planet_services_panel.name = "PlanetServicesPanel"
    var content: VBoxContainer = built.content
    _planet_title_label = Label.new()
    _planet_title_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    content.add_child(_planet_title_label)
    var description := Label.new()
    description.text = "Choose a service while docked on this world."
    description.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    content.add_child(description)
    var services := GridContainer.new()
    services.columns = 2
    content.add_child(services)
    var refuel := Button.new()
    refuel.text = "BUY 10 FUEL · %d CR" % (10 * int(ContentDatabase.get_item("fuel").get("base_price",14)))
    refuel.custom_minimum_size = Vector2(190,58)
    refuel.pressed.connect(func(): action_requested.emit("REFUEL",_nearby_planet_id))
    services.add_child(refuel)
    for service_name: String in ["WEAPONS MARKET","TRADE CENTER","MISSIONS","SHIPYARD"]:
        var service := Button.new()
        service.text = service_name + "\nCOMING SOON"
        service.custom_minimum_size = Vector2(190,58)
        service.disabled = true
        services.add_child(service)
    _planet_result_label = Label.new()
    _planet_result_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    content.add_child(_planet_result_label)
    var close := Button.new()
    close.text = "TAKE OFF"
    close.pressed.connect(func(): _close_panels())
    content.add_child(close)

func show_planet_services(planet_id: String, planet_name: String) -> void:
    _close_panels()
    _nearby_planet_id = planet_id
    _nearby_planet_name = planet_name
    _planet_title_label.text = planet_name
    _planet_result_label.text = ""
    _planet_services_panel.show()
    _gameplay_blocked = true
    boost_changed.emit(false)
    fire_changed.emit(false)
    steering_changed.emit(Vector2.ZERO)

func show_planet_service_result(message: String) -> void:
    if is_instance_valid(_planet_result_label):
        _planet_result_label.text = message

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

func play_ftl_transition() -> void:
    _close_panels()
    _gameplay_blocked = true
    _transit_overlay = ColorRect.new()
    _transit_overlay.color = Color(0.01,0.02,0.07,0.0)
    _transit_overlay.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    _transit_overlay.mouse_filter = Control.MOUSE_FILTER_STOP
    _transit_overlay.z_index = 120
    add_child(_transit_overlay)
    var fade_in := create_tween()
    fade_in.tween_property(_transit_overlay,"color",Color(0.01,0.02,0.07,1.0),0.48).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN)
    await fade_in.finished
    await get_tree().create_timer(0.22).timeout

func finish_ftl_transition() -> void:
    if not is_instance_valid(_transit_overlay):
        _gameplay_blocked = false
        return
    var fade_out := create_tween()
    fade_out.tween_property(_transit_overlay,"color",Color(0.01,0.02,0.07,0.0),0.62).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
    await fade_out.finished
    _transit_overlay.queue_free()
    _transit_overlay = null
    _gameplay_blocked = false

func _close_panels() -> void:
    if _active_touch != -1:
        _active_touch = -1
        _joystick_vector = Vector2.ZERO
        steering_changed.emit(Vector2.ZERO)
        queue_redraw()
    boost_changed.emit(false)
    fire_changed.emit(false)
    if is_instance_valid(_inventory_panel): _inventory_panel.hide()
    if is_instance_valid(_galaxy_panel): _galaxy_panel.hide()
    if is_instance_valid(_recovery_panel): _recovery_panel.hide()
    if is_instance_valid(_planet_services_panel): _planet_services_panel.hide()
    _gameplay_blocked = false

func _joystick_center() -> Vector2:
    return Vector2(JOYSTICK_CENTER_X,size.y-JOYSTICK_BOTTOM_MARGIN)

func _draw() -> void:
    var base_texture := AssetLibrary.load_texture("ui_joystick_base.png")
    var knob_texture := AssetLibrary.load_texture("ui_joystick_knob.png")
    var base_position := _joystick_center() if _active_touch == -1 else _joystick_origin
    var joystick_vector := Vector2.ZERO if _active_touch == -1 else _joystick_vector
    if base_texture != null:
        draw_texture_rect(base_texture,Rect2(base_position-Vector2(64,64),Vector2(128,128)),false)
    else:
        draw_circle(base_position,JOYSTICK_RADIUS,Color(0.18,0.38,0.56,0.22))
        draw_arc(base_position,JOYSTICK_RADIUS,0.0,TAU,48,Color(0.48,0.74,0.94,0.78),3.0)
    # Direction ticks make the movement control understandable before the
    # player touches it; the knob remains centered until a finger is active.
    var tick_color := Color(0.72,0.88,1.0,0.58)
    draw_line(base_position+Vector2(0,-43),base_position+Vector2(0,-31),tick_color,2.0)
    draw_line(base_position+Vector2(0,43),base_position+Vector2(0,31),tick_color,2.0)
    draw_line(base_position+Vector2(-43,0),base_position+Vector2(-31,0),tick_color,2.0)
    draw_line(base_position+Vector2(43,0),base_position+Vector2(31,0),tick_color,2.0)
    var knob_position := base_position+joystick_vector*JOYSTICK_RADIUS
    if knob_texture != null:
        draw_texture_rect(knob_texture,Rect2(knob_position-Vector2(24,24),Vector2(48,48)),false)
    else:
        draw_circle(knob_position,23.0,Color(0.58,0.82,1.0,0.82))


    var move_label := ThemeDB.fallback_font
    if move_label != null:
        draw_string(move_label,base_position+Vector2(-23,78),"MOVE",HORIZONTAL_ALIGNMENT_CENTER,-1,12,Color(0.82,0.91,1.0,0.82))
