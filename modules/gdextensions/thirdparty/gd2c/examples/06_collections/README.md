# 06 — Collections

## Array construction

```gdscript
var arr = [1, 2, 3]
```

```c
// OPCODE_CONSTRUCTARRAY with 3 elements
godot_variant _elems[3];
api10->godot_variant_new_int(&_elems[0], 1);
api10->godot_variant_new_int(&_elems[1], 2);
api10->godot_variant_new_int(&_elems[2], 3);
godot_array _arr;
api10->godot_array_new(&_arr);
for (int i = 0; i < 3; i++) {
    api10->godot_array_append(&_arr, &_elems[i]);
}
api10->godot_variant_new_array(&arr, &_arr);
```

## Dictionary construction

```gdscript
var data = {"name": "Goblin", "health": 50}
```

```c
// OPCODE_CONSTRUCTDICTIONARY with 2 key-value pairs
godot_dictionary _dict;
api10->godot_dictionary_new(&_dict);
// ... set key "name" -> "Goblin", "health" -> 50
api10->godot_variant_new_dictionary(&data, &_dict);
```

## Indexing (get/set)

```gdscript
data["armor"] = 5      # OPCODE_SET
var name = data["name"] # OPCODE_GET
```

```c
// OPCODE_SET
api10->godot_variant_set(&data, &key_armor, &val_5, &valid);

// OPCODE_GET
api10->godot_variant_get(&data, &key_name, &valid);
```

## for-in iteration

Same `ITERATEBEGIN`/`ITERATE` pattern as for arrays (see example 03).
For dictionaries, the iterator yields keys.
