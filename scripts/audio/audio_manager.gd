extends Node
class_name StarForgeAudioManager

const PLAYER_COUNT := 8
var effects_enabled := true
var effects_volume_db := 0.0
var _streams: Dictionary = {}
var _volumes: Dictionary = {}
var _players: Array[AudioStreamPlayer] = []
var _next_player := 0

func _ready() -> void:
    for index in range(PLAYER_COUNT):
        var player := AudioStreamPlayer.new()
        player.name = "SFXPlayer%d" % index
        player.bus = "Master"
        add_child(player)
        _players.append(player)

func register_sound(sound_id: String, stream: AudioStream, volume_db: float = 0.0) -> bool:
    if sound_id.is_empty() or stream == null:
        return false
    _streams[sound_id] = stream
    _volumes[sound_id] = volume_db
    return true

func register_sound_file(sound_id: String, resource_path: String, volume_db: float = 0.0) -> bool:
    if not ResourceLoader.exists(resource_path):
        push_warning("Optional sound asset is missing: " + resource_path)
        return false
    return register_sound(sound_id,load(resource_path) as AudioStream,volume_db)

func has_sound(sound_id: String) -> bool:
    return _streams.has(sound_id)

func play_sound(sound_id: String, pitch_scale: float = 1.0) -> void:
    if not effects_enabled or not _streams.has(sound_id) or _players.is_empty():
        return
    var player := _players[_next_player]
    _next_player = (_next_player+1)%_players.size()
    player.stop()
    player.stream = _streams[sound_id]
    player.volume_db = effects_volume_db+float(_volumes.get(sound_id,0.0))
    player.pitch_scale = clampf(pitch_scale,0.5,2.0)
    player.play()

func stop_all() -> void:
    for player: AudioStreamPlayer in _players:
        player.stop()

func set_effects_enabled(enabled: bool) -> void:
    effects_enabled = enabled
    if not enabled:
        stop_all()

func set_effects_volume_db(value: float) -> void:
    effects_volume_db = clampf(value,-40.0,6.0)

func get_registered_sound_count() -> int:
    return _streams.size()
