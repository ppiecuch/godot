# Converted .it songs (trackergen GDScript)

Auto-generated from the Impulse Tracker modules in the churris-x/trackers folders
(`favorites`, `miscellaneous`, `tracker-music-playlist-from-1999`) by
`../../tools/it_to_trackergen.py`. Each `.gd` rebuilds one song as a `TrackerSong`
and plays it.

**What is preserved:** musical *structure* — order list, patterns (note /
instrument slot / volume per row+channel), and initial tempo/speed.
**What is not:** the original sample PCM and effects. Timbres are **procedural**
(one synthesized `TrackerInstrument` per used instrument slot, waveform cycled
saw/square/triangle/pulse). So these sound "chip-ish", not identical to the source
— they are the *compositions ported into trackergen*, not faithful renders.

## Usage

Attach any file to a `Node` and run its scene (it builds the song and plays it via
an `AudioStreamPlayer` in `_ready`). Or call it programmatically:

```gdscript
var song := load("res://.../it_converted/twilight.gd").new().build_song()
var s := AudioStreamTracker.new()
s.song = song
s.voices = 16
$AudioStreamPlayer.stream = s
$AudioStreamPlayer.play()
# ... or render offline:
s.save_to_wav("user://twilight.wav", 30.0, 44100)
```

## Regenerate

```
python3 tools/it_to_trackergen.py <in.it> examples/it_converted/<name>.gd
```

## Notes
- 19 of 20 `.it` modules converted; `space_debris.it` was skipped (its header is not
  a standard `IMPM` signature).
- Non-IT formats in those folders (`.xm`, `.mod`, `.s3m`, `.669`) are **not** converted
  here — that needs a multi-format parser (libopenmpt).
- Timing: row duration = `2.5*speed/tempo`, mapped to `tempo_bpm = 6*tempo/speed`,
  `steps_per_beat = 4` (1 IT row = 1 trackergen step).
- Note mapping: IT note value → ABSOLUTE MIDI directly.

## License / usage caveat
These are **third-party demoscene compositions** by their original authors, with no
stated license in the source repository. The generated `.gd` encodes their note data.
Treat them as local study/reference material; do **not** ship them in a released game
without permission from the original authors.
