# gd2c Translation Examples

Each subdirectory demonstrates how specific GDScript constructs translate
to native C/C++ through the gd2c pipeline.

## Structure

```
examples/
  01_basics/          # Variables, functions, print, expressions
  02_inheritance/     # Base/derived classes, virtual dispatch
  03_control_flow/    # if/elif/else, for, while, match, return
  04_types_and_variants/ # Typed variables, casts, type checks
  05_signals_and_coroutines/ # signal, emit, yield (not yet implemented)
  06_collections/     # Array, Dictionary, for-in loops
  07_patterns/        # Common game patterns (state machine, singleton)
```

Each example has:
- `source/*.gd` — The original GDScript
- `expected.c` / `expected.h` — Annotated expected C output (key fragments)
- `README.md` — Explanation of what this example demonstrates

## Running an example

```bash
# 1. Export bytecode from Godot (run inside the example's source/ directory)
godot --no-window -s ../../gd2c.gd -Din"." -Dout"."

# 2. Translate to C (gdnative target)
python3 ../../gd2c.py source/ out/ gdnative

# 3. Translate to C++ (cppnative target)
python3 ../../gd2c.py source/ out_cpp/ cppnative
```

## Translation Reference

| GDScript | C (gdnative) |
|----------|--------------|
| `var x = 5` | `godot_variant _stack0; api10->godot_variant_new_int(&_stack0, 5);` |
| `func foo():` | `godot_variant foo(godot_object *p_instance, ...)` |
| `x + y` | `api10->godot_variant_evaluate(GODOT_VARIANT_OP_ADD, &x, &y, &result, &valid);` |
| `print(x)` | `api11->godot_variant_call(...)` with `"print"` |
| `if cond:` | `if (api10->godot_variant_booleanize(&cond)) { ... }` |
| `for i in arr:` | `OPCODE_ITERATEBEGIN` / `OPCODE_ITERATE` loop |
| `self.member` | `user_data->member` (direct struct access) |
| `emit_signal(...)` | `api10->godot_method_bind_call(...)` |
| `yield(...)` | **Not yet implemented** (coroutine transform) |
