# trackergen P1 demo — a full song: patterns x chords x kit, played through the
# sequencer + 8-voice manager. Notes are stored RELATIVE to the chord, so the
# bass/arp re-voice as the i-VI-III-VII progression moves. Mouse X = tension
# (deepens the bass and, later, will drive tempo).
#
# Setup: add an AudioStreamPlayer child named "Player"; run the scene.

extends Node2D

var stream : AudioStreamTracker

func _make_kit() -> TrackerKit:
	var bass := TrackerInstrument.new()
	bass.id = "bass"
	bass.waveform = TrackerInstrument.WAVE_SAW
	bass.harmonics = 16
	bass.filter_cutoff_min = 0.45
	bass.filter_cutoff_max = 0.06
	bass.variant_count = 5
	bass.variant_mode = TrackerInstrument.VARIANT_TENSION   # deep bass follows tension
	bass.attack = 0.005; bass.decay = 0.08; bass.sustain = 0.9; bass.release = 0.12
	bass.gain = 0.9
	bass.priority = 8                                        # protected: survives a busy mix

	var lead := TrackerInstrument.new()
	lead.id = "lead"
	lead.waveform = TrackerInstrument.WAVE_SQUARE
	lead.harmonics = 20
	lead.filter_cutoff_min = 0.5; lead.filter_cutoff_max = 0.5
	lead.variant_count = 1
	lead.attack = 0.005; lead.decay = 0.1; lead.sustain = 0.6; lead.release = 0.15
	lead.gain = 0.5
	lead.priority = 1                                        # low: first to be stolen

	var kit := TrackerKit.new()
	kit.instruments = [bass, lead]                          # index 0=bass, 1=lead
	return kit

func _make_chords() -> ChordProgression:
	var prog := ChordProgression.new()
	prog.key_root = 9                                        # A
	prog.base_octave = 3
	# A minor: i - VI - III - VII, minor/major triads, 1 bar each.
	prog.chords = [
		{ "degree": 0, "quality": PoolIntArray([0, 3, 7]), "bars": 1 },  # i  (Am)
		{ "degree": 5, "quality": PoolIntArray([0, 4, 7]), "bars": 1 },  # VI (F)
		{ "degree": 2, "quality": PoolIntArray([0, 4, 7]), "bars": 1 },  # III(C)
		{ "degree": 6, "quality": PoolIntArray([0, 4, 7]), "bars": 1 },  # VII(G)
	]
	return prog

func _make_pattern() -> TrackerPattern:
	var p := TrackerPattern.new()
	p.set_steps(16)   # one bar of 16th notes (steps_per_beat=4, 4/4)
	# Lane 0 = bass on every quarter note, root of the current chord (octave 0).
	# add_note(step, lane, note_index, octave, instrument) -> CHORD_TONE, volume 1.0.
	for step in [0, 4, 8, 12]:
		p.add_note(step, 0, 0, 0, 0)
	# Lane 1 = lead arpeggio over the chord tones, one octave up (instrument 1).
	var tones := [0, 1, 2, 1]  # root, 3rd, 5th, 3rd
	for i in range(8):
		# add_event gives full control (here: custom volume).
		p.add_event({ "step": i * 2, "lane": 1, "type": TrackerPattern.NOTE_CHORD_TONE,
			"index": tones[i % 4], "octave": 1, "instrument": 1, "volume": 0.8 })
	return p

func _make_sfx_kit() -> TrackerKit:
	# Procedural SFX (share the voice pool). Built straight from presets.
	var blip := TrackerInstrument.new()
	blip.load_preset("blip")
	var zap := TrackerInstrument.new()
	zap.load_preset("fm_bell")   # bright "ping" one-shot

	var kit := TrackerKit.new()
	kit.instruments = [blip, zap]   # queue_sfx(0)=blip, queue_sfx(1)=ping
	return kit

func _ready() -> void:
	var song := TrackerSong.new()
	song.kit = _make_kit()
	song.chords = _make_chords()
	song.patterns = [_make_pattern()]
	song.order = PoolIntArray([0, 0, 0, 0])   # loop the pattern under all 4 chords
	song.tempo_bpm = 128.0
	song.tempo_min = 110.0
	song.tempo_max = 160.0
	song.tempo_reacts = true                  # P2: tension drives tempo
	song.steps_per_beat = 4
	song.beats_per_bar = 4

	stream = AudioStreamTracker.new()
	stream.song = song
	stream.sfx_kit = _make_sfx_kit()          # P2: SFX share the voice pool
	stream.voices = 8
	stream.tension = 0.0

	var player : AudioStreamPlayer = $Player
	player.stream = stream
	player.play()

func _process(_delta: float) -> void:
	var w := get_viewport().size.x
	if w > 0:
		stream.tension = clamp(get_viewport().get_mouse_position().x / w, 0.0, 1.0)

func _input(event: InputEvent) -> void:
	# Click to fire an SFX (blip = index 0) — lock-free, steals a music voice if full.
	if event is InputEventMouseButton and event.pressed:
		stream.queue_sfx(0, 880.0, 1.0)
