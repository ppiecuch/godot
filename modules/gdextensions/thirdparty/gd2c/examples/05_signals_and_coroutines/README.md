# 05 — Signals and Coroutines

## Signals

```gdscript
signal health_changed(new_health)
signal died()
```

Signals are registered during `nativescript_init`:

```c
// In nativescript_init()
{
    godot_signal signal;
    signal.name = api10->godot_string_chars_to_utf8("health_changed");
    signal.num_args = 1;
    signal.num_default_args = 0;
    // ... set up arg types
    nativescript10->godot_nativescript_register_signal(handle, "ClassName", &signal);
}
```

## emit_signal

```gdscript
emit_signal("health_changed", health)
```

Translates to a method call on self:

```c
// OPCODE_CALLSELF "emit_signal"
godot_variant _args[2];
// _args[0] = "health_changed" (string variant)
// _args[1] = health (variant)
godot_variant *_argptrs[2] = { &_args[0], &_args[1] };
api10->godot_method_bind_call(
    emit_signal_method_bind,
    p_instance,
    (const godot_variant **)_argptrs,
    2,
    &call_error);
```

## Coroutines (yield) — NOT IMPLEMENTED

The `analysis.annotate_coroutines()` pass detects `yield` opcodes
(`OPCODE_YIELD`, `OPCODE_YIELDSIGNAL`) and marks functions as coroutines.

The `transform.make_coroutine()` would transform a yielding function into
a state machine, but this is **not yet implemented** and raises
`NotImplementedError`.

When implemented, a yield would translate to:
1. Save current stack state to a coroutine object
2. Return the coroutine object to the caller
3. On resume, restore stack and continue from the yield point
