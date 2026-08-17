# trackergen preset showcase — plays every built-in instrument preset in turn as
# a short arpeggio, so you can hear the palette. Attach to a Node and run.

extends Node

var stream : AudioStreamTracker
var player : AudioStreamPlayer
var presets : PoolStringArray
var idx := 0

func _pattern() -> TrackerPattern:
	var p := TrackerPattern.new()
	p.set_steps(8)
	# simple ascending arp on one lane (instrument 0), ABSOLUTE MIDI notes.
	var notes := [48, 55, 60, 64, 67, 64, 60, 55]
	for i in range(8):
		p.add_event({ "step": i, "lane": 0, "type": TrackerPattern.NOTE_ABSOLUTE,
			"index": notes[i], "octave": 0, "instrument": 0, "volume": 0.9 })
	return p

func _song_for(preset: String) -> TrackerSong:
	var inst := TrackerInstrument.new()
	inst.load_preset(preset)
	var kit := TrackerKit.new()
	kit.instruments = [inst]
	var song := TrackerSong.new()
	song.kit = kit
	song.patterns = [_pattern()]
	song.order = PoolIntArray([0, 0])
	song.tempo_bpm = 132.0
	return song

func _play_current() -> void:
	var name := presets[idx]
	print("trackergen preset: ", name)
	stream = AudioStreamTracker.new()
	stream.song = _song_for(name)
	player.stream = stream
	player.play()

func _ready() -> void:
	player = AudioStreamPlayer.new()
	add_child(player)
	# get the preset list from a throwaway instrument
	presets = TrackerInstrument.new().get_preset_names()
	_play_current()
	# advance every 2s
	var t := Timer.new()
	t.wait_time = 2.0
	t.autostart = true
	add_child(t)
	t.connect("timeout", self, "_next")

func _next() -> void:
	idx = (idx + 1) % presets.size()
	_play_current()
