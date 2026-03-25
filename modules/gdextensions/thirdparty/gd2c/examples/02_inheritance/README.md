# 02 — Inheritance

Demonstrates how class hierarchy and virtual dispatch translate to C.

## GDScript

```gdscript
# base.gd
extends Node
func method_a(): print("base.method_a")
func method_b(): print("base.method_b")

# derived.gd
extends "res://base.gd"
func method_b(): print("derived.method_b")  # override
func method_c(): print("derived.method_c")  # new method
```

## Translation: Virtual Table (vtable)

Each class gets a vtable struct with function pointers for all methods.
Derived classes copy the base vtable and override specific entries.

```c
// Base vtable
struct gd2c_class_vtable base_vtable = {
    .method_a = &base_method_a,
    .method_b = &base_method_b,
};

// Derived vtable — inherits method_a, overrides method_b, adds method_c
struct gd2c_class_vtable derived_vtable = {
    .method_a = &base_method_a,     // inherited
    .method_b = &derived_method_b,  // overridden
    .method_c = &derived_method_c,  // new
};
```

## Translation: Struct layout

```c
// Base class struct
struct Base {
    struct gd2c_class_vtable *__vtable;
    godot_variant __self;
    // (no members in this example)
};

// Derived class struct — includes base members first
struct Derived {
    struct gd2c_class_vtable *__vtable;
    godot_variant __self;
    // base members would go here
    // derived-only members follow
};
```

## Translation: Virtual dispatch

When calling `obj.method_b()`, the generated code looks up the vtable:

```c
// OPCODE_CALLSELF "method_b"
user_data->__vtable->method_b(p_instance, p_method_data, user_data, 0, NULL);
```

This ensures the correct override is called regardless of the declared type.

## Key concept: type_id

Each class is assigned a unique `type_id` for `is` checks:

```c
// Registration
nativescript10->godot_nativescript_register_class(handle, "Base", "Node", ...);
nativescript10->godot_nativescript_register_class(handle, "Derived", "Base", ...);
```
