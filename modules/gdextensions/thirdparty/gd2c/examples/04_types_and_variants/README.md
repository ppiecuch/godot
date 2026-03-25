# 04 — Types and Variants

All GDScript values are `godot_variant` at the C level. Type annotations
help gd2c's (future) optimization passes promote to native types.

## Variant wrapping/unwrapping

```gdscript
var pos: Vector2 = Vector2(10, 20)
```

```c
// OPCODE_CONSTRUCT: Vector2
godot_vector2 _tmp;
api10->godot_vector2_new(&_tmp, 10.0, 20.0);
api10->godot_variant_new_vector2(&user_data->pos, &_tmp);
```

## Type checks (is)

```gdscript
if obj is Node:
```

```c
// OPCODE_EXTENDSTEST
api10->godot_variant_evaluate(GODOT_VARIANT_OP_IS, &obj, &node_type, &result, &valid);
if (api10->godot_variant_booleanize(&result)) { ... }
```

## Cast (int(), float(), str())

```gdscript
var result: int = int(value)
```

```c
// OPCODE_CASTTOBUILTIN with type_id = GODOT_VARIANT_TYPE_INT
godot_int _i = api10->godot_variant_as_int(&value);
api10->godot_variant_new_int(&result, _i);
```

## Method calls on typed values

```gdscript
var normalized = result.normalized()
```

When the compiler knows `result` is Vector2, this could be optimized to:
```c
godot_vector2 v = api10->godot_variant_as_vector2(&result);
godot_vector2 n = api10->godot_vector2_normalized(&v);
api10->godot_variant_new_vector2(&normalized, &n);
```

Currently (without typed arithmetic promotion), it uses generic variant call:
```c
// OPCODE_CALL "normalized"
godot_string method_name;
api10->godot_string_new_with_latin1_chars(&method_name, "normalized");
// ... call via variant API
```

## Future: typed arithmetic promotion

The `promote_typed_arithmetic` transform (not yet implemented) would convert:
```c
// Generic: 3 API calls
api10->godot_variant_evaluate(GODOT_VARIANT_OP_ADD, &a, &b, &result, &valid);

// Promoted (when both operands known to be int):
godot_int _a = api10->godot_variant_as_int(&a);
godot_int _b = api10->godot_variant_as_int(&b);
api10->godot_variant_new_int(&result, _a + _b);  // native arithmetic
```
