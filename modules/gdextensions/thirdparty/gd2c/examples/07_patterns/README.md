# 07 — Common Game Patterns

## State Machine

This is the most common pattern that benefits from gd2c native compilation.
The state machine's `_physics_process` is called 60+ times per second and
involves many variant operations.

### Key translation points

**enum values** become integer constants:
```gdscript
enum State { IDLE, RUNNING, JUMPING, FALLING, DEAD }
```
```c
// Class constants registered as godot_variant (type: INT)
// State.IDLE = 0, State.RUNNING = 1, etc.
```

**match on enum** compiles to sequential integer comparisons:
```c
// match current_state:
godot_int _state = api10->godot_variant_as_int(&user_data->current_state);
if (_state == 0) { state_idle(...); }
else if (_state == 1) { state_running(...); }
// ... etc
```

**Singleton access** (Input.is_action_pressed):
```c
// Input is a named global (singleton)
godot_variant _input = /* resolve "Input" singleton */;
godot_variant _method = /* "is_action_pressed" */;
godot_variant _arg = /* "move_right" */;
// OPCODE_CALL on the singleton
```

**Member writes** (velocity.x = SPEED):
```c
// OPCODE_SETNAMED "x" on member "velocity"
godot_variant _speed;
api10->godot_variant_new_real(&_speed, 200.0);
api10->godot_variant_set_named(&user_data->velocity, &name_x, &_speed, &valid);
```

### Why this benefits from native compilation

In interpreted GDScript, each `match` branch, each `Input.is_action_pressed()`
call, and each `velocity.x = ...` goes through the bytecode interpreter loop
with variant boxing overhead. When compiled to C:

1. Function dispatch is a direct C function call (no interpreter loop)
2. Integer comparisons can use native `==` after a single unbox
3. Method calls use pre-resolved method binds (no string lookup per call)
4. Stack variables are C locals (no variant pool allocation)

Typical performance improvement: **3-10x** for logic-heavy code like state machines.
