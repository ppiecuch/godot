from __future__ import annotations
from typing import TYPE_CHECKING, List
from gd2c.targets._cppnative.context import FunctionContext
from gd2c.bytecode import CallBuiltinGDScriptOp
from gd2c import builtin as BI
from gd2c.address import GDScriptAddress
from gd2c import variant

if TYPE_CHECKING:
    from typing import IO

disallowed_builtins: List[int] = [
    BI.OBJ_WEAKREF, # Not sure yet
    BI.FUNC_FUNCREF, # Not sure yet
    BI.GET_STACK, # Should probably just return empty array
]


def call_builtin(function_context: FunctionContext, op: CallBuiltinGDScriptOp, file: IO) -> None:
    FC = function_context

    # First check for built-ins implemented via intrinsic
    if op.function_index == BI.TYPE_CONVERT:
        type_convert(function_context, op, file)
    elif op.function_index == BI.RESOURCE_LOAD:
        resource_load(function_context, op, file)
    else:
        # If no intrinsic then check to see if we can use call_gdscript_builtin
        if not op.function_index in disallowed_builtins:
            dest = function_context.variables[op.dest]
            file.write(f"""\
                {{
                    godot_variant *args[] = {{ {", ".join([
                        FC.variables[addr].address_of() for addr in op.args
                    ])} }};
                    godot_variant_call_error err;
                    gd2c10->call_gdscript_builtin({op.function_index}, (const godot_variant **)args, {op.arg_count}, {dest.address_of()}, &err);
                }}
                """)
        else:
            # Else...?
            file.write(f""" // builtin call {op.function_index} not supported\n""")

def resource_load(function_context: FunctionContext, op: CallBuiltinGDScriptOp, file: IO) -> None:
    assert op.arg_count == 1
    file.write(f"""\
        {{
            godot_string extension = api10->godot_string_chars_to_utf8(".gd");
            godot_string resource_path = api10->godot_variant_as_string({function_context.variables[op.args[0]].address_of()});
            if (api10->godot_string_ends_with(&resource_path, &extension)) {{
                godot_string ns = api10->godot_string_chars_to_utf8("ns");
                godot_string new_resource_path = api10->godot_string_operator_plus(&resource_path, &ns);
                api10->godot_string_destroy(&resource_path);
                resource_path = new_resource_path;
                api10->godot_string_destroy(&ns);
            }}
            gd2c10->resource_load({function_context.variables[op.dest].address_of()}, &resource_path);
            api10->godot_string_destroy(&resource_path);
            api10->godot_string_destroy(&extension);
        }}
        """)

def type_convert(function_context: FunctionContext, op: CallBuiltinGDScriptOp, file: IO) -> None:
    source = function_context.variables[op.args[0]]
    dest = function_context.variables[op.dest]
    vtype = function_context.variables[op.args[1]]
    file.write(f"""\
        {{
            int t = api10->godot_variant_as_int({vtype.address_of()});
            godot_variant_call_error err;
            godot_variant *args[] = {{ {source.address_of()} }};
            gd2c10->variant_convert({dest.address_of()}, t, (const godot_variant **)args, 1, &err);
        }}
        """)

