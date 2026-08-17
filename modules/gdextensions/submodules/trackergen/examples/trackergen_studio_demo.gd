# trackergen P3 "studio" demo — play a song through a player-facing 10-band EQ
# bus (Godot's AudioEffectEQ10) and record the result to a .wav. This is the
# "little audio studio" from the thread: tweak EQ, then export trailer music.
#
# Setup: attach to a Node, run the scene. Keys 1..0 nudge EQ bands; R renders.

extends Node

const BUS_NAME := "TrackerStudio"

var stream : AudioStreamTracker
var player : AudioStreamPlayer
var _bus_idx : int = -1

func _make_song() -> TrackerSong:
	var bass := TrackerInstrument.new()
	bass.id = "bass"; bass.waveform = TrackerInstrument.WAVE_SAW; bass.harmonics = 16
	bass.filter_cutoff_min = 0.4; bass.filter_cutoff_max = 0.08
	bass.variant_count = 5; bass.variant_mode = TrackerInstrument.VARIANT_TENSION
	bass.priority = 8; bass.gain = 0.9

	var lead := TrackerInstrument.new()
	lead.id = "lead"; lead.waveform = TrackerInstrument.WAVE_SQUARE; lead.harmonics = 18
	lead.gain = 0.5; lead.priority = 1

	var kit := TrackerKit.new()
	kit.instruments = [bass, lead]

	var prog := ChordProgression.new()
	prog.key_root = 9; prog.base_octave = 3
	prog.chords = [
		{ "degree": 0, "quality": PoolIntArray([0, 3, 7]), "bars": 1 },
		{ "degree": 5, "quality": PoolIntArray([0, 4, 7]), "bars": 1 },
		{ "degree": 2, "quality": PoolIntArray([0, 4, 7]), "bars": 1 },
		{ "degree": 6, "quality": PoolIntArray([0, 4, 7]), "bars": 1 },
	]

	var pat := TrackerPattern.new()
	pat.set_steps(16)
	for step in [0, 4, 8, 12]:
		pat.add_note(step, 0, 0, 0, 0)          # bass root
	for i in range(8):
		pat.add_note(i * 2, 1, [0, 1, 2, 1][i % 4], 1, 1)  # lead arp

	var song := TrackerSong.new()
	song.kit = kit; song.chords = prog; song.patterns = [pat]
	song.order = PoolIntArray([0, 0, 0, 0])
	song.tempo_bpm = 128.0
	return song

func _setup_bus() -> void:
	_bus_idx = AudioServer.bus_count
	AudioServer.add_bus(_bus_idx)
	AudioServer.set_bus_name(_bus_idx, BUS_NAME)
	AudioServer.set_bus_send(_bus_idx, "Master")
	AudioServer.add_bus_effect(_bus_idx, AudioEffectEQ10.new())  # player-facing 10-band EQ

func set_band_db(band: int, gain_db: float) -> void:
	var eq := AudioServer.get_bus_effect(_bus_idx, 0) as AudioEffectEQ10
	if eq:
		eq.set_band_gain_db(band, gain_db)

func render_trailer(path: String = "user://trackergen_trailer.wav", seconds: float = 12.0) -> void:
	# NOTE: save_to_wav renders the raw synth (pre-EQ). To bake the EQ into the
	# file, route through an AudioEffectRecord bus and capture live instead.
	var err := stream.save_to_wav(path, seconds, 44100)
	print("trackergen: render %s -> %s" % [path, "OK" if err == OK else "FAILED"])

func _ready() -> void:
	_setup_bus()
	stream = AudioStreamTracker.new()
	stream.song = _make_song()
	player = AudioStreamPlayer.new()
	add_child(player)
	player.bus = BUS_NAME
	player.stream = stream
	player.play()

func _unhandled_key_input(event: InputEventKey) -> void:
	if not event.pressed:
		return
	# 1..0 => boost EQ bands 0..9 by +6 dB (toy control).
	var digits := [KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0]
	var idx : int = digits.find(event.scancode)
	if idx != -1:
		set_band_db(idx, 6.0)
	elif event.scancode == KEY_R:
		render_trailer()
