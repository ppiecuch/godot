# 01 — Basics

Demonstrates translation of fundamental GDScript constructs.

## Constructs covered

| GDScript | gd2c Translation |
|----------|-----------------|
| `var health: int = 100` | Struct member + godot_variant_new_int in constructor |
| `func add(a, b) -> int` | C function with godot_variant params, unbox/box for return |
| `health -= amount` | OPCODE_OPERATOR with subtract, result assigned back |
| `if health <= 0:` | OPCODE_OPERATOR compare → OPCODE_JUMPIFNOT |
| `print("text", var)` | OPCODE_CALLBUILTIN with function index for `print` |
| `return a + b` | OPCODE_OPERATOR add → OPCODE_RETURN |
| `str(health)` | OPCODE_CALLBUILTIN for `str` conversion |
| `name + " HP:"` | OPCODE_OPERATOR string concatenation |

## How variables become struct members

```gdscript
var health: int = 100
var speed: float = 3.5
```

Translates to a C struct:

```c
struct Class_N {
    struct gd2c_class_vtable *__vtable;
    godot_variant __self;
    godot_variant health;   // member index 0
    godot_variant speed;    // member index 1
    godot_variant name;     // member index 2
};
```

The constructor initializes each member:

```c
void *ctor(godot_object *p_instance, void *p_method_data) {
    struct Class_N *user_data = api10->godot_alloc(sizeof(struct Class_N));
    // health = 100
    api10->godot_variant_new_int(&user_data->health, 100);
    // speed = 3.5
    api10->godot_variant_new_real(&user_data->speed, 3.5);
    // name = "Player"
    godot_string s;
    api10->godot_string_new_with_latin1_chars(&s, "Player");
    api10->godot_variant_new_string(&user_data->name, &s);
    api10->godot_string_destroy(&s);
    return user_data;
}
```

## How functions translate

```gdscript
func add(a: int, b: int) -> int:
    return a + b
```

Becomes:

```c
godot_variant add(godot_object *p_instance, void *p_method_data,
                  void *_p_user_data, int p_num_args, godot_variant **p_args) {
    // Stack allocation for temporaries
    godot_variant _stack0;  // result of a + b

    // OPCODE_OPERATOR: ADD
    bool _valid;
    api10->godot_variant_evaluate(
        GODOT_VARIANT_OP_ADD,
        p_args[0],    // a
        p_args[1],    // b
        &_stack0,
        &_valid);

    // OPCODE_RETURN
    return _stack0;
}
```
