from __future__ import annotations
from typing import TYPE_CHECKING, IO, Set, Union
from gd2c.address import *
from gd2c.variant import VariantType
from gd2c.bytecode import *
from gd2c.controlflow import Block
from gd2c.targets._gdnative import builtin as BI
import re
if TYPE_CHECKING:
    from gd2c.targets.gdnative import FunctionContext

def __transpile_signature(function_context: FunctionContext) -> str:
    return f"""\
        godot_variant {function_context.function_identifier}( \
            godot_object* p_instance, \
            void* p_method_data, \
            void* _p_user_data, \
            int p_num_args, \
            godot_variant** p_args)"""

def transpile_signature(function_context: FunctionContext, file: IO):
    file.write(__transpile_signature(function_context))
    file.write(";\n")

def transpile_function(function_context: FunctionContext, file: IO):
    assert function_context
    assert function_context.func
    assert function_context.func.cfg

    file.write(__transpile_signature(function_context))
    file.write(f"""\
        {{
            //printf("Enter: {function_context.function_identifier}\\n");
            struct {function_context.class_context.struct_tag} *p_user_data = (struct {function_context.class_context.struct_tag}*)_p_user_data;
            godot_bool __flag;
            godot_variant_call_error __error;
            godot_variant __return_value;
            api10->godot_variant_new_nil(&__return_value);
        """)

    # Initialize function local constants if first time function was called
    if function_context.func.has_constants:
        file.write(f"""\
            if (0 == {function_context.initialized_local_constants_array_identifier}) {{
        """)
        for const in function_context.func.constants():
            loaded = False
            if const.vtype == VariantType.OBJECT and const.declaration.startswith("Resource("):
                m = re.search("^Resource\(\s+\"([^\"]*)\"\)\s*$", const.declaration)
                if not m is None:
                    resource_path = m.group(1)
                    #TODO: Should actually check the GDScriptProject to see if this script was compiled
                    if resource_path.endswith(".gd"):
                        resource_path += "ns"

                    utf8 = bytes(resource_path, 'UTF-8')
                    file.write(f"""\
                        {{
                            char data[] = {{ {','.join(map(lambda b: str(b), utf8))} }};
                            godot_string resource_path;
                            api10->godot_string_new(&resource_path);
                            api10->godot_string_parse_utf8_with_len(&resource_path, data, {len(utf8)});
                            gd2c10->resource_load(&{function_context.local_constants_array_identifier}[{const.index}], &resource_path);
                            api10->godot_string_destroy(&resource_path);
                        }}
                        """)

                    loaded = True

            if not loaded:
                file.write(f"""\
                    {{
                        uint8_t data[] = {{ {','.join(map(lambda b: str(b), const.data))} }};
                        int bytesRead;
                        gd2c10->variant_decode(&{function_context.local_constants_array_identifier}[{const.index}], data, {len(const.data)}, &bytesRead, true);
                    }}
                """)

        for index, name in enumerate(function_context.func.global_names):
            utf8 = bytes(name, 'UTF-8')
            file.write(f"""\
                {{
                    char data[] = {{ {','.join(map(lambda b: str(b), utf8))} }};
                    api10->godot_string_new(&{function_context.global_strings_identifier}[{index}]);
                    api10->godot_string_parse_utf8_with_len(&{function_context.global_strings_identifier}[{index}], data, {len(utf8)});
                    api10->godot_string_name_new(&{function_context.global_names_identifier}[{index}], &{function_context.global_strings_identifier}[{index}]);
                }}
                """)

        file.write(f"""\
                {function_context.initialized_local_constants_array_identifier} = 1;
            }}
        """)

    for var in function_context.variables.values():
        if var.needs_definition():
            file.write(var.define())

    file.write(f"goto {function_context.func.cfg.entry_node.label};\n")
    __transpile_nodes(function_context, file)

    file.write("_cleanup:\n")

    # Reserved for cleanup of any allocations done outside bytecode
    # Any allocations as a result of bytecode should have coresponding deallocation bytecode

    file.write(f"""\
            //printf("Exit: {function_context.function_identifier}\\n");
            return __return_value;
        }}
    """)

def __transpile_nodes(function_context: FunctionContext, file: IO):
    assert function_context.func
    assert function_context.func.cfg
    cfg = function_context.func.cfg
    worklist = [cfg.entry_node]
    visited: Set[Block] = set()
    while any(worklist):
        node = cast(Block, worklist.pop())
        if node in visited:
            continue

        visited.add(node)
        worklist.extend(cfg.succs(node))

        file.write(f"{node.label}:\n")
        file.write(f"""printf("%i : {node.label}\\n", __LINE__);\n""")
        for op in node.ops:
            __transpile_op(function_context, node, op, file)

        # The exit node probably will not have any sort of branch instruction
        # coded in the block. We need to make sure it skips ahead to our
        # cleanup routine
        if node == cfg.exit_node:
            file.write(f"goto _cleanup;\n")

def __transpile_op(function_context: FunctionContext, node: Block, op: GDScriptOp, file: IO):
    FC = function_context
    assert FC
    assert FC.func
    assert FC.func.cfg

    def opcode_jump(op: JumpGDScriptOp):
        nonlocal FC
        assert FC.func
        assert FC.func.cfg
        branch = FC.func.cfg.node_from_address(op.branch)
        assert branch
        file.write(f"goto {branch.label};\n")

    def opcode_jumpif(op: JumpIfGDScriptOp):
        nonlocal FC
        assert FC.func
        assert FC.func.cfg
        branch = FC.func.cfg.node_from_address(op.branch)
        assert branch
        fallthrough = FC.func.cfg.node_from_address(op.fallthrough)
        assert fallthrough
        file.write(f"""\
            __flag = api10->godot_variant_as_bool({FC.variables[op.condition].address_of()});
            if (__flag) goto {branch.label};
            goto {fallthrough.label};
        """)

    def opcode_jumpifnot(op: JumpIfNotGDScriptOp):
        nonlocal FC
        assert FC.func
        assert FC.func.cfg
        branch = FC.func.cfg.node_from_address(op.branch)
        assert branch
        fallthrough = FC.func.cfg.node_from_address(op.fallthrough)
        assert fallthrough
        file.write(f"""\
            __flag = api10->godot_variant_as_bool({FC.variables[op.condition].address_of()});
            if (!__flag) goto {branch.label};
            goto {fallthrough.label};
        """)

    def opcode_line(op: LineGDScriptOp):
        nonlocal FC
        # Ignore
        pass

    def opcode_assign(op: AssignGDScriptOp):
        nonlocal FC
        file.write(f"""\
            api10->godot_variant_new_copy({FC.variables[op.dest].address_of()}, {FC.variables[op.source].address_of()});
        """)

    def opcode_assigntrue(op: AssignTrueGDScriptOp):
        nonlocal FC
        file.write(f"""\
            api10->godot_variant_new_bool({FC.variables[op.dest].address_of()}, true);
        """)

    def opcode_assignfalse(op: AssignFalseGDScriptOp):
        nonlocal FC
        file.write(f"""\
            api10->godot_variant_new_bool({FC.variables[op.dest].address_of()}, false);
        """)

    def opcode_operator(op: OperatorGDScriptOp):
        nonlocal FC
        file.write(\
            f"api11->godot_variant_evaluate({op.op}, " \
                f"{FC.variables[op.operand1].address_of()}, " \
                f"{FC.variables[op.operand2].address_of()}, " \
                f"{FC.variables[op.dest].address_of()}, " \
                f"&__flag);\n")

    def opcode_return(op: ReturnGDScriptOp):
        nonlocal FC
        assert FC
        assert FC.func
        assert FC.func.cfg
        assert FC.func.cfg.exit_node

        file.write(f"""\
            api10->godot_variant_new_copy(&__return_value, {FC.variables[op.source].address_of()});
            goto {FC.func.cfg.exit_node.label};
        """)

    def opcode_destroy(op: DestroyGDScriptOp):
        nonlocal FC
        file.write(f"""\
            api10->godot_variant_destroy({FC.variables[op.address].address_of()});
        """)

    def opcode_initialize(op: InitializeGDScriptOp):
        nonlocal FC
        file.write(f"""\
            api10->godot_variant_new_nil({FC.variables[op.address].address_of()});
        """)

    def opcode_call(op: Union[CallGDScriptOp, CallReturnGDScriptOp, CallSelfBaseGDScriptOp]):
        nonlocal FC
        call_base = isinstance(op, CallSelfBaseGDScriptOp)

        if call_base:
            receiver = "p_user_data->__self"
            try_vtable = True
        else:
            receiver_address = GDScriptAddress(op.receiver) # type: ignore
            receiver = FC.variables[receiver_address.address].address_of()
            try_vtable = receiver_address.mode == ADDRESS_MODE_SELF

        vtable_entry = None
        if try_vtable:
            method_name = FC.func.global_names[op.name_index]
            ctx = FC.class_context.base_context if call_base else FC.class_context
            if ctx is None:
                if method_name == "_init":
                    file.write(f"""\
                        //
                        // Call to base _init is always ignored
                        //

                        """)
                    return
                else:
                    assert ctx

            vtable_entry = ctx.get_member_vtable_entry(method_name)

        # use local scope for args[]
        file.write("{\n")

        # args array if necessary, otherwise just use a null ptr
        if op.arg_count > 0:
            file.write(f"""\
                godot_variant *args[] = {{ {", ".join([
                    FC.variables[addr].address_of() for addr in op.args
                ])} }};
            """)
            args = "args"
        else:
            args = "(void*)0"

        # if method is in vtable we can call with function pointer,
        # otherwise we will have to call with godot_variant_call
        if isinstance(op, CallReturnGDScriptOp):
            file.write("godot_variant call_result = ")

        if vtable_entry:
            base_chain = "base->" if call_base else ""
            file.write(\
                f"p_user_data->__vtable->{base_chain}methods[{vtable_entry.index}](" \
                    f"p_instance, " \
                    f"p_method_data, " \
                    f"p_user_data, " \
                    f"{op.arg_count}, " \
                    f"{args});\n")
        else:
            file.write(\
                f"api10->godot_variant_call(" \
                    f"{receiver}, " \
                    f"&{FC.global_strings_identifier}[{op.name_index}], " \
                    f"(const godot_variant **){args}, " \
                    f"{op.arg_count}, " \
                    f"&__error);\n")

        if isinstance(op, CallReturnGDScriptOp):
            file.write(f"""\
                api10->godot_variant_new_copy({FC.variables[op.dest].address_of()}, &call_result);
                api10->godot_variant_destroy(&call_result);
            """)

        file.write("}\n")

    def opcode_jumptodefaultargument(op: JumpToDefaultArgumentGDScriptOp):
        nonlocal FC
        assert FC.func.cfg
        file.write(f"""\
            int defarg = {FC.func.len_parameters} - p_num_args;
            switch (defarg) {{
            """)
        for i in range(len(op.jump_table)):
            file.write(f"""\
            case {i}: goto {FC.func.cfg.node_label_from_address(op.jump_table[i])};
            """)

        file.write(f"""\
            default: goto {FC.func.cfg.node_label_from_address(op.fallthrough)};
        }}
        """)

    def opcode_end(op: EndGDScriptOp):
        nonlocal FC
        file.write(f"goto {FC.func.cfg.exit_node.label};\n") # type: ignore

    def opcode_set(op: SetGDScriptOp):
        nonlocal FC
        file.write(f"""\
            gd2c10->variant_set({FC.variables[op.array_address].address_of()}, {FC.variables[op.index_address].address_of()}, {FC.variables[op.source_address].address_of()}, &__flag);
            """)

    def opcode_get(op: GetGDScriptOp):
        nonlocal FC
        file.write(f"""\
            gd2c10->variant_get({FC.variables[op.array_address].address_of()}, {FC.variables[op.index_address].address_of()}, {FC.variables[op.dest].address_of()}, &__flag);
            """)

    def opcode_setnamed(op: SetNamedGDScriptOp):
        nonlocal FC
        file.write(f"""\
            gd2c10->variant_set_named(\
                {FC.variables[op.dest].address_of()}, \
                &{FC.global_names_identifier}[{op.name_index}], \
                {FC.variables[op.source].address_of()}, \
                &__flag);
            """)

    def opcode_getnamed(op: GetNamedGDScriptOp):
        nonlocal FC
        file.write(f"""\
            gd2c10->variant_get_named( \
                {FC.variables[op.source].address_of()}, \
                &{FC.global_names_identifier}[{op.name_index}],\
                {FC.variables[op.dest].address_of()}, \
                &__flag);
            """)

    def opcode_setmember(op: SetMemberGDScriptOp):
        nonlocal FC
        file.write(f"""\
            gd2c10->object_set_property( \
                p_instance, \
                &{FC.global_names_identifier}[{op.name_index}], \
                {FC.variables[op.source].address_of()});
            """)

    def opcode_getmember(op: GetMemberGDScriptOp):
        nonlocal FC
        file.write(f"""\
            gd2c10->object_get_property(\
                {FC.variables[op.dest].address_of()}, \
                p_instance, \
                &{FC.global_names_identifier}[{op.name_index}]);
            """)

    def opcode_constructarray(op: ConstructArrayGDScriptOp):
        nonlocal FC
        file.write(f"""\
            {{
            godot_array arr;
            api10->godot_array_new(&arr);
            api10->godot_array_resize(&arr, {op.item_count});
        """)

        for i, addr in enumerate(op.item_addresses):
            file.write(f"""\
                api10->godot_array_set(&arr, {i}, {FC.variables[addr].address_of()});
            """)

        file.write(f"""\
            api10->godot_variant_new_array({FC.variables[op.dest].address_of()}, &arr);
            api10->godot_array_destroy(&arr);
        }}\n""")

    def opcode_callbuiltin(op: CallBuiltinGDScriptOp):
        BI.call_builtin(FC, op, file)

    def opcode_assigntypedbuiltin(op: AssignTypedBuiltinGDScriptOp):
        file.write(f"""\
            api10->godot_variant_new_copy({FC.variables[op.dest].address_of()}, {FC.variables[op.source].address_of()});
            """)

    def opcode_iteratebegin(op: IterateBeginGDScriptOp):
        nonlocal FC
        assert FC.func
        assert FC.func.cfg
        branch = FC.func.cfg.node_from_address(op.branch)
        assert branch
        fallthrough = FC.func.cfg.node_from_address(op.fallthrough)
        assert fallthrough
        file.write(f"""\
            if(!gd2c10->variant_iter_init({FC.variables[op.container].address_of()}, {FC.variables[op.counter].address_of()}, &__flag)) {{
                goto {branch.label};
            }} else {{
                gd2c10->variant_iter_get({FC.variables[op.container].address_of()}, {FC.variables[op.counter].address_of()}, {FC.variables[op.iterator].address_of()}, &__flag);
                goto {fallthrough.label};
            }}
        """)

    def opcode_iterate(op: IterateGDScriptOp):
        nonlocal FC
        assert FC.func
        assert FC.func.cfg
        branch = FC.func.cfg.node_from_address(op.branch)
        assert branch
        fallthrough = FC.func.cfg.node_from_address(op.fallthrough)
        assert fallthrough
        file.write(f"""\
            if(!gd2c10->variant_iter_next({FC.variables[op.container].address_of()}, {FC.variables[op.counter].address_of()}, &__flag)) {{
                goto {branch.label};
            }} else {{
                gd2c10->variant_iter_get({FC.variables[op.container].address_of()}, {FC.variables[op.counter].address_of()}, {FC.variables[op.iterator].address_of()}, &__flag);
                goto {fallthrough.label};
            }}
        """)

    def opcode_extendstest(op: ExtendsTestGDScriptOp):
        nonlocal FC
        file.write(f"""\
            gd2c10->extends_test({FC.variables[op.a].address_of()}, {FC.variables[op.b].address_of()}, {FC.variables[op.dest].address_of()});
        """)

    def opcode_constructdictionary(op: ConstructDictionaryGDScriptOp):
        nonlocal FC
        file.write(f"""\
            {{
                godot_dictionary dict;
                api10->godot_dictionary_new(&dict);
            """)
        for i in range(op.item_count):
            file.write(f"""\
                api10->godot_dictionary_set(&dict, {FC.variables[op.key_addresses[i]].address_of()}, {FC.variables[op.value_addresses[i]].address_of()});
                """)
        file.write(f"""\
                api10->godot_variant_new_dictionary({FC.variables[op.dest].address_of()}, &dict);
                api10->godot_dictionary_destroy(&dict);
            }}
            """)

    def opcode_construct(op: ConstructGDScriptOp):
        nonlocal FC
        file.write(f"{{\n")
        if op.arg_count > 0:
            file.write(f"""\
                godot_variant *args[] = {{ {", ".join([
                    FC.variables[addr].address_of() for addr in op.args
                ])} }};
            """)
            args = "args"
        else:
            args = "(void*)0"

        file.write(f"""\
            gd2c10->variant_construct({FC.variables[op.dest].address_of()}, {op.vtype.value}, {op.arg_count}, (const godot_variant **){args}, &__error);
            }}
            """)

    def opcode_copyparameter(op: CopyParameterGDScriptOp):
        nonlocal FC
        file.write(f"""\
            if (p_num_args > {op.parameter.address.offset})
                api10->godot_variant_new_copy({FC.variables[op.dest].address_of()}, {FC.variables[op.parameter.address.address].address_of()});
            """)

    def opcode_isbuiltin(op: IsBuiltInGDScriptOp):
        nonlocal FC
        file.write(f"""\
            {{
                godot_variant_type vt = api10->godot_variant_get_type({FC.variables[op.a].address_of()});
                api10->godot_variant_new_bool({FC.variables[op.dest].address_of()}, (int)vt == {op.type_code});
            }}
            """)

    #file.write(f"""printf("C LINE %i\\n", __LINE__);\n""")

    if op.opcode == OPCODE_OPERATOR:
        opcode_operator(op) # type: ignore
    elif op.opcode == OPCODE_SET:
        opcode_set(op) # type: ignore
    elif op.opcode == OPCODE_GET:
        opcode_get(op) # type: ignore
    elif op.opcode == OPCODE_SETNAMED:
        opcode_setnamed(op) # type: ignore
    elif op.opcode == OPCODE_GETNAMED:
        opcode_getnamed(op) # type: ignore
    elif op.opcode == OPCODE_ASSIGN:
        opcode_assign(op) # type: ignore
    elif op.opcode == OPCODE_ASSIGNTRUE:
        opcode_assigntrue(op) # type: ignore
    elif op.opcode == OPCODE_ASSIGNFALSE:
        opcode_assignfalse(op) # type: ignore
    elif op.opcode == OPCODE_CALL:
        opcode_call(op) # type: ignore
    elif op.opcode == OPCODE_CALLRETURN:
        opcode_call(op) # type: ignore
    elif op.opcode == OPCODE_CALLSELFBASE:
        opcode_call(op) # type: ignore
    elif op.opcode == OPCODE_JUMP:
        opcode_jump(op) # type: ignore
    elif op.opcode == OPCODE_JUMPIF:
        opcode_jumpif(op) # type: ignore
    elif op.opcode == OPCODE_JUMPIFNOT:
        opcode_jumpifnot(op) # type: ignore
    elif op.opcode == OPCODE_JUMPTODEFAULTARGUMENT:
        opcode_jumptodefaultargument(op) # type: ignore
    elif op.opcode == OPCODE_RETURN:
        opcode_return(op) # type: ignore
    elif op.opcode == OPCODE_LINE:
        opcode_line(op) # type: ignore
    elif op.opcode == OPCODE_END:
        opcode_end(op) # type: ignore
    elif op.opcode == OPCODE_DESTROY:
        opcode_destroy(op) # type: ignore
    elif op.opcode == OPCODE_INITIALIZE:
        opcode_initialize(op) # type: ignore
    elif op.opcode == OPCODE_SETMEMBER:
        opcode_setmember(op) # type: ignore
    elif op.opcode == OPCODE_GETMEMBER:
        opcode_getmember(op) # type: ignore
    elif op.opcode == OPCODE_CONSTRUCTARRAY:
        opcode_constructarray(op) # type: ignore
    elif op.opcode == OPCODE_CALLSELF:
        pass
    elif op.opcode == OPCODE_CALLBUILTIN:
        opcode_callbuiltin(op) # type: ignore
    elif op.opcode == OPCODE_ASSIGNTYPEDBUILTIN:
        opcode_assigntypedbuiltin(op) # type: ignore
    elif op.opcode == OPCODE_ITERATEBEGIN:
        opcode_iteratebegin(op) # type: ignore
    elif op.opcode == OPCODE_ITERATE:
        opcode_iterate(op) # type: ignore
    elif op.opcode == OPCODE_EXTENDSTEST:
        opcode_extendstest(op) # type: ignore
    elif op.opcode == OPCODE_CONSTRUCTDICTIONARY:
        opcode_constructdictionary(op) # type: ignore
    elif op.opcode == OPCODE_CONSTRUCT:
        opcode_construct(op) # type: ignore
    elif op.opcode == OPCODE_PARAMETER_COPY:
        opcode_copyparameter(op) # type: ignore
    elif op.opcode == OPCODE_ISBUILTIN:
        opcode_isbuiltin(op) # type: ignore

    elif op.opcode == OPCODE_ASSIGNTYPEDNATIVE:
        file.write(f"""// OPCODE_ASSIGNTYPEDNATIVE\n""")
    elif op.opcode == OPCODE_ASSIGNTYPEDSCRIPT:
        file.write(f"""// OPCODE_ASSIGNTYPEDSCRIPT\n""")
    elif op.opcode == OPCODE_CASTTOBUILTIN:
        file.write(f"""// OPCODE_CASTTOBUILTIN\n""")
    elif op.opcode == OPCODE_CASTTONATIVE:
        file.write(f"""// OPCODE_CASTTONATIVE\n""")
    elif op.opcode == OPCODE_CASTTOSCRIPT:
        file.write(f"""// OPCODE_CASTTOSCRIPT\n""")

    elif op.opcode == OPCODE_YIELD:
        file.write(f"""// OPCODE_YIELD\n""")
    elif op.opcode == OPCODE_YIELDSIGNAL:
        file.write(f"""// OPCODE_YIELDSIGNAL\n""")
    elif op.opcode == OPCODE_YIELDRESUME:
        file.write(f"""// OPCODE_YIELDRESUME\n""")

    elif op.opcode == OPCODE_ASSERT:
        file.write(f"""// OPCODE_ASSERT\n""")
    elif op.opcode == OPCODE_BREAKPOINT:
        file.write(f"""// OPCODE_BREAKPOINT\n""")
    else:
        file.write(f"// {str(op)};\n")


