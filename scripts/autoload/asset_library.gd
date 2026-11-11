extends Node

# Explicit preload references are required for Android exports.
# The exported PCK contains imported textures, not the original loose PNG files;
# runtime directory scanning therefore cannot reliably discover them.
const ASSETS: Dictionary = {
    "bg_nebula_320x180.png": preload("res://assets/backgrounds/bg_nebula_320x180.png"),
    "star_yellow_64.png": preload("res://assets/planets/star_yellow_64.png"),
    "orbit_ring_96.png": preload("res://assets/planets/orbit_ring_96.png"),
    "ship_player_runner.png": preload("res://assets/ships/ship_player_runner.png"),
    "ship_cargo_hauler.png": preload("res://assets/ships/ship_cargo_hauler.png"),
    "ship_pirate_interceptor.png": preload("res://assets/ships/ship_pirate_interceptor.png"),
    "ship_military_frigate.png": preload("res://assets/ships/ship_military_frigate.png"),
    "station_ring_hub.png": preload("res://assets/stations/station_ring_hub.png"),
    "planet_terran_64.png": preload("res://assets/planets/planet_terran_64.png"),
    "planet_gas_96.png": preload("res://assets/planets/planet_gas_96.png"),
    "planet_moon_32.png": preload("res://assets/planets/planet_moon_32.png"),
    "asteroid_small_01.png": preload("res://assets/asteroids/asteroid_small_01.png"),
    "asteroid_small_02.png": preload("res://assets/asteroids/asteroid_small_02.png"),
    "asteroid_small_03.png": preload("res://assets/asteroids/asteroid_small_03.png"),
    "asteroid_large_01.png": preload("res://assets/asteroids/asteroid_large_01.png"),
    "salvage_container_generic.png": preload("res://assets/salvage/salvage_container_generic.png"),
    "salvage_fuel.png": preload("res://assets/salvage/salvage_fuel.png"),
    "salvage_metal.png": preload("res://assets/salvage/salvage_metal.png"),
    "salvage_luxury.png": preload("res://assets/salvage/salvage_luxury.png"),
    "salvage_equipment.png": preload("res://assets/salvage/salvage_equipment.png"),
    "salvage_weapon.png": preload("res://assets/salvage/salvage_weapon.png"),
    "salvage_food.png": preload("res://assets/salvage/salvage_food.png"),
    "salvage_medicine.png": preload("res://assets/salvage/salvage_medicine.png"),
    "proj_laser_blue.png": preload("res://assets/projectiles/proj_laser_blue.png"),
    "proj_laser_red.png": preload("res://assets/projectiles/proj_laser_red.png"),
    "ui_button_9slice_normal.png": preload("res://assets/ui/ui_button_9slice_normal.png"),
    "ui_button_9slice_pressed.png": preload("res://assets/ui/ui_button_9slice_pressed.png"),
    "ui_btn_boost_up.png": preload("res://assets/ui/ui_btn_boost_up.png"),
    "ui_btn_boost_down.png": preload("res://assets/ui/ui_btn_boost_down.png"),
    "ui_btn_fire_up.png": preload("res://assets/ui/ui_btn_fire_up.png"),
    "ui_btn_fire_down.png": preload("res://assets/ui/ui_btn_fire_down.png"),
    "ui_btn_loot_up.png": preload("res://assets/ui/ui_btn_loot_up.png"),
    "ui_btn_loot_down.png": preload("res://assets/ui/ui_btn_loot_down.png"),
    "ui_joystick_base.png": preload("res://assets/ui/ui_joystick_base.png"),
    "ui_joystick_knob.png": preload("res://assets/ui/ui_joystick_knob.png"),
    "ui_panel_9slice.png": preload("res://assets/ui/ui_panel_9slice.png"),
    "ui_icon_credits.png": preload("res://assets/ui/ui_icon_credits.png"),
    "ui_icon_energy.png": preload("res://assets/ui/ui_icon_energy.png"),
    "ui_icon_fuel.png": preload("res://assets/ui/ui_icon_fuel.png"),
    "ui_icon_health.png": preload("res://assets/ui/ui_icon_health.png"),
    "ui_icon_inventory.png": preload("res://assets/ui/ui_icon_inventory.png"),
    "ui_icon_weapon.png": preload("res://assets/ui/ui_icon_weapon.png")
}

func load_texture(file_name: String) -> Texture2D:
    return ASSETS.get(file_name) as Texture2D

func has_asset(file_name: String) -> bool:
    return ASSETS.has(file_name)
