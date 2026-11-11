extends Node

signal inventory_changed
signal credits_changed
signal system_changed(system_id: String)
signal player_destroyed

const SAVE_PATH := "user://star_forge_save.json"
const FTL_FUEL_COST := 5
const CARGO_CAPACITY := 100
var current_system_id := "solace"
var has_ftl_drive := true
var credits := 250
var inventory := {"metal":0,"fuel":25,"food":0,"medicine":0,"weapon_salvage":0}
var system_states: Dictionary = {}
var player_ship_state := {"hull":100.0,"armour":60.0,"energy":100.0,"position_x":480.0,"position_y":270.0,"velocity_x":0.0,"velocity_y":0.0}

func cargo_used() -> int:
    var used := 0
    for id: String in inventory:
        used += int(inventory[id]) * int(ContentDatabase.get_item(id).get("cargo_units_per_item", 1))
    return used

func can_add_item(id: String, amount: int) -> bool:
    return amount > 0 and ContentDatabase.get_item(id).size() > 0 and cargo_used() + amount * int(ContentDatabase.get_item(id).get("cargo_units_per_item", 1)) <= CARGO_CAPACITY

func add_item(id: String, amount: int) -> bool:
    if not can_add_item(id, amount):
        return false
    inventory[id] = int(inventory.get(id, 0)) + amount
    inventory_changed.emit()
    return true

func remove_item(id: String, amount: int) -> bool:
    if amount <= 0 or int(inventory.get(id, 0)) < amount:
        return false
    inventory[id] = int(inventory[id]) - amount
    inventory_changed.emit()
    return true

func sell_item(id: String, amount: int) -> bool:
    if not remove_item(id, amount):
        return false
    credits += int(ContentDatabase.get_item(id).get("base_price", 0)) * amount
    credits_changed.emit()
    return true

func can_jump_to(id: String) -> bool:
    var current := ContentDatabase.get_system(current_system_id)
    return has_ftl_drive and id != current_system_id and Array(current.get("links", [])).has(id) and int(inventory.fuel) >= FTL_FUEL_COST

func jump_to_system(id: String) -> bool:
    if not can_jump_to(id):
        return false
    inventory.fuel = int(inventory.fuel) - FTL_FUEL_COST
    current_system_id = id
    player_ship_state.position_x = 480.0
    player_ship_state.position_y = 270.0
    player_ship_state.velocity_x = 0.0
    player_ship_state.velocity_y = 0.0
    inventory_changed.emit()
    system_changed.emit(id)
    save_game()
    return true

func apply_player_damage(amount: float) -> void:
    if amount <= 0.0 or float(player_ship_state.hull) <= 0.0:
        return
    var absorbed := minf(float(player_ship_state.armour), amount)
    player_ship_state.armour = float(player_ship_state.armour) - absorbed
    player_ship_state.hull = maxf(0.0, float(player_ship_state.hull) - (amount - absorbed))
    if float(player_ship_state.hull) <= 0.0:
        player_destroyed.emit()

func save_game() -> bool:
    var payload := {"version":1,"current_system_id":current_system_id,"has_ftl_drive":has_ftl_drive,"credits":credits,"inventory":inventory,"system_states":system_states,"player_ship_state":player_ship_state}
    var file := FileAccess.open(SAVE_PATH, FileAccess.WRITE)
    if file == null:
        push_error("Could not open save file.")
        return false
    file.store_string(JSON.stringify(payload))
    return file.get_error() == OK

func load_game() -> bool:
    if not FileAccess.file_exists(SAVE_PATH):
        return false
    var file := FileAccess.open(SAVE_PATH, FileAccess.READ)
    if file == null:
        return false
    var parsed: Variant = JSON.parse_string(file.get_as_text())
    if not parsed is Dictionary or int(parsed.get("version", 0)) != 1:
        push_error("Save data version is invalid.")
        return false
    var saved_system := str(parsed.get("current_system_id", "solace"))
    if not ContentDatabase.systems.has(saved_system):
        push_error("Save references unknown system.")
        return false
    current_system_id = saved_system
    has_ftl_drive = bool(parsed.get("has_ftl_drive", true))
    credits = maxi(0, int(parsed.get("credits", 0)))
    var saved_inventory: Dictionary = parsed.get("inventory", {})
    for id: String in inventory:
        inventory[id] = maxi(0, int(saved_inventory.get(id, 0)))
    system_states = parsed.get("system_states", {})
    var saved_ship: Dictionary = parsed.get("player_ship_state", {})
    for key: String in player_ship_state:
        if saved_ship.has(key):
            player_ship_state[key] = saved_ship[key]
    inventory_changed.emit()
    credits_changed.emit()
    system_changed.emit(current_system_id)
    return true
