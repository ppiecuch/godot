#!/usr/bin/env python3
"""Convert Impulse Tracker (.it) modules into trackergen GDScript.

Offline, dependency-free. Extracts the musical STRUCTURE only — orders, patterns
(note / instrument / volume per row+channel), and initial tempo/speed — and emits
a .gd that rebuilds it as a TrackerSong using PROCEDURAL instruments (one per used
instrument slot). Sample PCM is NOT embedded (it would bloat the .gd and needs the
IT sample codec); timbres are synthesized. This is a "get the composition into our
engine" converter, not a faithful renderer.

Usage:  it_to_trackergen.py <in.it> <out.gd>
Note mapping: IT note value v (0..119, C-5 = 60) -> ABSOLUTE MIDI = v.
Timing: row duration = 2.5*speed/tempo  ->  tempo_bpm = 6*tempo/speed, steps_per_beat = 4.
"""

import struct
import sys
import os


def u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


NOTE_OFF_VALS = (253, 254, 255)  # fade / cut / keyoff -> our NOTE_OFF


def parse_it(data):
    if data[:4] != b"IMPM":
        raise ValueError("not an Impulse Tracker module (missing IMPM)")

    title = data[4:30].split(b"\x00")[0].decode("latin-1", "replace").strip()
    ord_num = u16(data, 0x20)
    ins_num = u16(data, 0x22)
    smp_num = u16(data, 0x24)
    pat_num = u16(data, 0x26)
    speed = data[0x32] or 6
    tempo = data[0x33] or 125

    o = 0xC0
    orders = list(data[o:o + ord_num]); o += ord_num
    ins_off = [u32(data, o + 4 * i) for i in range(ins_num)]; o += 4 * ins_num
    smp_off = [u32(data, o + 4 * i) for i in range(smp_num)]; o += 4 * smp_num
    pat_off = [u32(data, o + 4 * i) for i in range(pat_num)]; o += 4 * pat_num

    # Build the play order out of real pattern indices (skip 254 '+++' / 255 end).
    play = [p for p in orders if p < 254 and p < pat_num]

    used_pats = sorted(set(play))
    remap = {p: i for i, p in enumerate(used_pats)}
    our_order = [remap[p] for p in play]

    patterns = []  # list of (rows, events[]) where event = (step, chan, midi_or_-1, inst_idx, vol)
    max_inst = 0
    for p in used_pats:
        off = pat_off[p]
        if off == 0 or off + 8 > len(data):
            patterns.append((1, []))
            continue
        length = u16(data, off)
        rows = u16(data, off + 2)
        body = data[off + 8: off + 8 + length]
        try:
            evs, mi = _decode_pattern(body, rows)
        except Exception:
            evs, mi = [], 0
        max_inst = max(max_inst, mi)
        patterns.append((rows if rows > 0 else 1, evs))

    num_inst = max(max_inst, 1)
    bpm = 6.0 * tempo / max(1, speed)
    bpm = max(20.0, min(400.0, bpm))
    return {
        "title": title,
        "bpm": bpm,
        "order": our_order,
        "patterns": patterns,
        "num_inst": num_inst,
    }


def _decode_pattern(body, rows):
    """Unpack IT packed pattern data into note events."""
    pos = 0
    row = 0
    n = len(body)
    last_mask = {}
    last_note = {}
    last_inst = {}
    last_vol = {}
    events = []
    max_inst = 0
    while row < rows and pos < n:
        b = body[pos]; pos += 1
        if b == 0:
            row += 1
            continue
        chan = (b - 1) & 63
        if b & 128:
            if pos >= n:
                break
            mask = body[pos]; pos += 1
            last_mask[chan] = mask
        else:
            mask = last_mask.get(chan, 0)

        note = None
        inst = None
        vol = None

        if mask & 1:
            note = body[pos]; pos += 1
            last_note[chan] = note
        if mask & 2:
            inst = body[pos]; pos += 1
            last_inst[chan] = inst
        if mask & 4:
            vol = body[pos]; pos += 1
            last_vol[chan] = vol
        if mask & 8:
            pos += 2  # effect cmd+param, ignored
        if mask & 16:
            note = last_note.get(chan)
        if mask & 32:
            inst = last_inst.get(chan)
        if mask & 64:
            vol = last_vol.get(chan)

        # Only rows that (re)start a note produce an event.
        if note is None:
            continue
        if note in NOTE_OFF_VALS:
            events.append((row, chan, -1, 0, 0))
            continue
        if note >= 120:
            continue  # other specials
        inst_idx = (inst - 1) if (inst and inst > 0) else 0
        if inst_idx < 0:
            inst_idx = 0
        max_inst = max(max_inst, inst_idx + 1)
        v = vol if (vol is not None and vol <= 64) else 64
        events.append((row, chan, note, inst_idx, v))
    return events, max_inst


def _pool_int(vals):
    return "PoolIntArray([%s])" % ",".join(str(v) for v in vals)


def emit_gd(song, src_name):
    title = song["title"] or src_name
    lines = []
    a = lines.append
    a("# Auto-generated from %s by it_to_trackergen.py — DO NOT EDIT BY HAND." % src_name)
    a("# Song: %s" % title.replace("\n", " "))
    a("# Structure only (orders/patterns/notes/tempo); timbres are procedural.")
    a("extends Node")
    a("")
    a("const TEMPO_BPM := %.3f" % song["bpm"])
    a("const STEPS_PER_BEAT := 4")
    a("const NUM_INST := %d" % song["num_inst"])
    a("const ORDER := %s" % _pool_int(song["order"]))
    a("const PAT_STEPS := %s" % _pool_int([rows for rows, _ in song["patterns"]]))
    a("const PAT_EVENTS := [")
    for rows, evs in song["patterns"]:
        flat = []
        for (step, chan, midi, inst_idx, vol) in evs:
            flat += [step, chan, midi, inst_idx, vol]
        a("\t%s," % _pool_int(flat))
    a("]")
    a("")
    a("# Procedural instruments cycle through musical presets (one per IT slot).")
    a('const PRESETS := ["saw_lead", "square_lead", "pluck", "reese_bass", "fm_epiano", "organ", "pad", "fm_bell"]')
    a("")
    a("func build_song() -> TrackerSong:")
    a("\tvar kit := TrackerKit.new()")
    a("\tvar insts := []")
    a("\tfor i in range(NUM_INST):")
    a("\t\tvar it := TrackerInstrument.new()")
    a("\t\tit.load_preset(PRESETS[i % PRESETS.size()])")
    a("\t\tinsts.append(it)")
    a("\tkit.instruments = insts")
    a("\tvar song := TrackerSong.new()")
    a("\tsong.kit = kit")
    a("\tsong.tempo_bpm = TEMPO_BPM")
    a("\tsong.steps_per_beat = STEPS_PER_BEAT")
    a("\tsong.beats_per_bar = 4")
    a("\tvar pats := []")
    a("\tfor pi in range(PAT_EVENTS.size()):")
    a("\t\tvar p := TrackerPattern.new()")
    a("\t\tp.set_steps(int(PAT_STEPS[pi]))")
    a("\t\tvar e : PoolIntArray = PAT_EVENTS[pi]")
    a("\t\tvar i := 0")
    a("\t\twhile i < e.size():")
    a("\t\t\tvar midi : int = e[i + 2]")
    a("\t\t\tif midi < 0:")
    a("\t\t\t\tp.add_event({\"step\": e[i], \"lane\": e[i + 1], \"type\": TrackerPattern.NOTE_OFF, \"index\": 0, \"octave\": 0, \"instrument\": 0, \"volume\": 0.0})")
    a("\t\t\telse:")
    a("\t\t\t\tp.add_event({\"step\": e[i], \"lane\": e[i + 1], \"type\": TrackerPattern.NOTE_ABSOLUTE, \"index\": midi, \"octave\": 0, \"instrument\": e[i + 3], \"volume\": float(e[i + 4]) / 64.0})")
    a("\t\t\ti += 5")
    a("\t\tpats.append(p)")
    a("\tsong.patterns = pats")
    a("\tsong.order = ORDER")
    a("\treturn song")
    a("")
    a("func _ready() -> void:")
    a("\tvar s := AudioStreamTracker.new()")
    a("\ts.song = build_song()")
    a("\ts.voices = 16")
    a("\tvar pl := AudioStreamPlayer.new()")
    a("\tadd_child(pl)")
    a("\tpl.stream = s")
    a("\tpl.play()")
    a("")
    return "\n".join(lines)


def main():
    if len(sys.argv) != 3:
        print("usage: it_to_trackergen.py <in.it> <out.gd>", file=sys.stderr)
        return 2
    src, dst = sys.argv[1], sys.argv[2]
    with open(src, "rb") as f:
        data = f.read()
    song = parse_it(data)
    gd = emit_gd(song, os.path.basename(src))
    with open(dst, "w") as f:
        f.write(gd)
    ev = sum(len(evs) for _, evs in song["patterns"])
    print("%s -> %s  (title=%r bpm=%.1f pats=%d order=%d inst=%d events=%d)" % (
        os.path.basename(src), os.path.basename(dst), song["title"], song["bpm"],
        len(song["patterns"]), len(song["order"]), song["num_inst"], ev))
    return 0


if __name__ == "__main__":
    sys.exit(main())
