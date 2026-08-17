# trackergen P0 demo — build a procedural kit in code, play one instrument as a
# sustained note, and sweep `tension` to hear the wavetable variants swap.
#
# Setup: add an AudioStreamPlayer named "Player" as a child of a node running
# this script, then run the scene. Move the mouse left/right to change tension.

extends Node2D

var stream : AudioStreamTracker

func _ready() -> void:
	# --- Instrument A: deep bass that darkens with tension (5 variants) ---
	var bass := TrackerInstrument.new()
	bass.id = "bass"
	bass.waveform = TrackerInstrument.WAVE_SAW
	bass.harmonics = 16
	bass.harmonic_falloff = 1.0
	bass.filter_cutoff_min = 0.45   # variant 0 = bright
	bass.filter_cutoff_max = 0.06   # last variant = dark/deep
	bass.filter_resonance = 0.3
	bass.variant_count = 5
	bass.variant_mode = TrackerInstrument.VARIANT_TENSION
	bass.attack = 0.01
	bass.decay = 0.1
	bass.sustain = 0.85
	bass.release = 0.3
	bass.gain = 0.9

	# --- Instrument B: dubstep-ish wobble (LFO swaps 8 cutoff tables) ---
	var wobble := TrackerInstrument.new()
	wobble.id = "wobble"
	wobble.waveform = TrackerInstrument.WAVE_SQUARE
	wobble.harmonics = 24
	wobble.filter_cutoff_min = 0.05
	wobble.filter_cutoff_max = 0.5
	wobble.filter_resonance = 0.5
	wobble.variant_count = 8
	wobble.variant_mode = TrackerInstrument.VARIANT_LFO
	wobble.lfo_hz = 4.0
	wobble.sustain = 1.0

	var kit := TrackerKit.new()
	kit.instruments = [bass, wobble]

	stream = AudioStreamTracker.new()
	stream.kit = kit
	stream.test_instrument = 0        # 0 = bass, 1 = wobble
	stream.test_frequency = 110.0     # A2
	stream.tension = 0.0

	var player : AudioStreamPlayer = $Player
	player.stream = stream
	player.play()

func _process(_delta: float) -> void:
	# Map mouse X to tension 0..1 (drives the bass variant / "depth").
	var w := get_viewport().size.x
	if w > 0:
		stream.tension = clamp(get_viewport().get_mouse_position().x / w, 0.0, 1.0)
