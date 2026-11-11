extends Control
class_name TouchHUD

signal steering_changed(direction: Vector2)
signal boost_changed(active: bool)
signal fire_changed(active: bool)
signal status_requested

const JOYSTICK_MAX_RADIUS: float = 58.0
const JOYSTICK_DEADZONE: float = 0.12

var _active_touch: int = -1
var _joystick_origin: Vector2 = Vector2.ZERO
var _joystick_vector: Vector2 = Vector2.ZERO
var _boost_button: Button
var _fire_button: Button
var _status_label: Label

func _ready() -> void:
    mouse_filter = Control.MOUSE_FILTER_IGNORE
    _build_status_label()
    _build_action_buttons()
    set_process_input(true)

func _build_status_label() -> void:
    _status_label = Label.new()
    _status_label.position = Vector2(24.0, 18.0)
    _status_label.add_theme_font_size_override("font_size", 18)
    _status_label.add_theme_color_override("font_color", Color(0.78, 0.88, 1.0))
    _status_label.mouse_filter = Control.MOUSE_FILTER_IGNORE
    add_child(_status_label)

func _build_action_buttons() -> void:
    _boost_button = _make_action_button("BOOST", Vector2(-238.0, -112.0), Vector2(104.0, 76.0))
    _fire_button = _make_action_button("FIRE", Vector2(-122.0, -112.0), Vector2(104.0, 76.0))

    _boost_button.button_down.connect(func() -> void: boost_changed.emit(true))
    _boost_button.button_up.connect(func() -> void: boost_changed.emit(false))
    _fire_button.button_down.connect(func() -> void: fire_changed.emit(true))
    _fire_button.button_up.connect(func() -> void: fire_changed.emit(false))

func _make_action_button(
    caption: String,
    offset: Vector2,
    button_size: Vector2
) -> Button:
    var button := Button.new()
    button.text = caption
    button.focus_mode = Control.FOCUS_NONE
    button.anchor_left = 1.0
    button.anchor_top = 1.0
    button.anchor_right = 1.0
    button.anchor_bottom = 1.0
    button.offset_left = offset.x
    button.offset_top = offset.y
    button.offset_right = offset.x + button_size.x
    button.offset_bottom = offset.y + button_size.y
    button.add_theme_font_size_override("font_size", 20)
    button.add_theme_color_override("font_color", Color(0.92, 0.96, 1.0))
    button.add_theme_stylebox_override("normal", _button_style(Color(0.08, 0.19, 0.32, 0.92)))
    button.add_theme_stylebox_override("pressed", _button_style(Color(0.12, 0.39, 0.58, 0.98)))
    button.add_theme_stylebox_override("hover", _button_style(Color(0.10, 0.28, 0.43, 0.96)))
    add_child(button)
    return button

func _button_style(fill_color: Color) -> StyleBoxFlat:
    var style := StyleBoxFlat.new()
    style.bg_color = fill_color
    style.border_color = Color(0.35, 0.66, 0.88, 0.9)
    style.set_border_width_all(2)
    style.set_corner_radius_all(14)
    style.content_margin_left = 8.0
    style.content_margin_right = 8.0
    return style

func _input(event: InputEvent) -> void:
    if event is InputEventScreenTouch:
        var touch := event as InputEventScreenTouch
        if touch.pressed:
            _begin_joystick(touch.index, touch.position)
        elif touch.index == _active_touch:
            _end_joystick()
    elif event is InputEventScreenDrag:
        var drag := event as InputEventScreenDrag
        if drag.index == _active_touch:
            _update_joystick(drag.position)

func _begin_joystick(touch_id: int, position: Vector2) -> void:
    if _active_touch != -1:
        return

    var joystick_zone := Rect2(
        Vector2(0.0, size.y * 0.38),
        Vector2(size.x * 0.43, size.y * 0.62)
    )
    if not joystick_zone.has_point(position):
        return

    _active_touch = touch_id
    _joystick_origin = position
    _update_joystick(position)

func _update_joystick(position: Vector2) -> void:
    var displacement := position - _joystick_origin
    _joystick_vector = displacement.limit_length(JOYSTICK_MAX_RADIUS) / JOYSTICK_MAX_RADIUS
    if _joystick_vector.length() < JOYSTICK_DEADZONE:
        _joystick_vector = Vector2.ZERO
    steering_changed.emit(_joystick_vector)
    queue_redraw()

func _end_joystick() -> void:
    _active_touch = -1
    _joystick_vector = Vector2.ZERO
    steering_changed.emit(Vector2.ZERO)
    queue_redraw()

func update_ship_status(
    current_hull: float,
    hull_limit: float,
    current_armour: float,
    armour_limit: float,
    current_energy: float,
    energy_limit: float
) -> void:
    if not is_instance_valid(_status_label):
        return

    _status_label.text = "HULL %03d/%03d   ARMOUR %03d/%03d   ENERGY %03d/%03d" % [
        roundi(current_hull),
        roundi(hull_limit),
        roundi(current_armour),
        roundi(armour_limit),
        roundi(current_energy),
        roundi(energy_limit)
    ]

func _draw() -> void:
    if _active_touch == -1:
        return

    draw_circle(_joystick_origin, JOYSTICK_MAX_RADIUS, Color(0.18, 0.38, 0.56, 0.22))
    draw_arc(_joystick_origin, JOYSTICK_MAX_RADIUS, 0.0, TAU, 48, Color(0.48, 0.74, 0.94, 0.78), 3.0)
    draw_circle(
        _joystick_origin + _joystick_vector * JOYSTICK_MAX_RADIUS,
        24.0,
        Color(0.58, 0.82, 1.0, 0.82)
    )
