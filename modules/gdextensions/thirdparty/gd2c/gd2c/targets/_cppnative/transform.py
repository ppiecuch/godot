from __future__ import annotations
from typing import TYPE_CHECKING, Dict, List, Set, Tuple
from gd2c.controlflow import Block
from gd2c.bytecode import *
from gd2c.gdscriptclass import GDScriptFunction
from gd2c.address import *
from gd2c import analysis

def insert_initializers_transformation(func: GDScriptFunction):
    """Inserts Initialize ops for all implicit stack variables."""
    assert func.cfg
    assert func.cfg.entry_node

    all_addr: Set[int] = set()
    for block in func.cfg.nodes():
        all_addr.update(block.defs)

    initialize_ops: List[GDScriptOp] = []

    for addr in all_addr:
        address = GDScriptAddress(addr)
        if address.mode in (ADDRESS_MODE_STACK, ADDRESS_MODE_STACKVARIABLE):
            if address.offset >= func.len_parameters:
                initialize_ops.append(InitializeGDScriptOp(addr))

    node = func.cfg.entry_node
    node.insert_ops_before(node.last_op, initialize_ops)

def insert_destructors_transformation(func: GDScriptFunction):
    """Inserts destructor calls for variants that were created in the
    function.
    """
    assert func.cfg
    assert func.cfg.exit_node

    all_addr: Set[int] = set()
    for block in func.cfg.nodes():
        all_addr.update(block.defs)

    ops: List[GDScriptOp] = []

    for addr in all_addr:
        address = GDScriptAddress(addr)
        if address.mode == ADDRESS_MODE_STACKVARIABLE or address.mode == ADDRESS_MODE_STACK:
            if address.offset >= func.len_parameters:
                ops.append(DestroyGDScriptOp(addr))

    node = func.cfg.exit_node
    if node.last_op:
        node.insert_ops_before(node.last_op, ops)
    else:
        node.append_ops(ops)

def replace_init_calls_with_noop_transformation(func: GDScriptFunction):
    """Eliminates calls to _init. The GDScript engine ignores these calls,
    so we should do the same.
    """
    def visit(node):
        remove: List[GDScriptOp] = []
        for op in node.ops:
            name = ""
            if isinstance(op, CallGDScriptOp):
                name = func_context.global_names[op.name_index]
            elif isinstance(op, CallSelfBaseGDScriptOp):
                name = func_context.func.global_names[op.name_index]

            if name == "_init":
                remove.append(op)

        for op in remove:
            node.block.remove_op(op)

    assert func
    assert func.cfg
    func.cfg.visit_nodes(visit)

def insert_parameter_copies(func: GDScriptFunction):
    """Inserts a copy operation for parameters that are written to in the function to
    imitate GDScript's pass by value semantics.
    """
    assert func
    assert func.cfg

    # We want to emit the parameters in reverse order because
    # the C compiler tends to emit better code when starting from
    # the top index
    parameters = sorted(func.parameters(), key=lambda p: p.index, reverse=True)

    for parameter in parameters:
        # Create new variable to for copy
        stack_var = func.new_stack_variable(parameter.vtype)
        func.cfg.entry_node.insert_ops_before(func.cfg.entry_node.last_op, [
            CopyParameterGDScriptOp(parameter, stack_var.address)
        ])

        # Update all references to parameter to new variable
        for block in func.cfg.nodes():
            if block == func.cfg.entry_node:
                pass
            for op in block.ops:
                addr = 0
                # We are always using STACK variables since STACKVARIABLE and STACK overlap in memory
                if parameter.address.mode == ADDRESS_MODE_STACKVARIABLE:
                    addr = GDScriptAddress.create(ADDRESS_MODE_STACK, parameter.address.offset).address
                else:
                    addr = parameter.address.address
                op.replace_address(addr, stack_var.address, False)

    func.cfg.live_variable_analysis()




