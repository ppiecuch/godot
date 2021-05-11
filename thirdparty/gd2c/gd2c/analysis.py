from __future__ import annotations
from typing import TYPE_CHECKING, Dict
from gd2c.address import GDScriptAddress, ADDRESS_MODE_PARAMETER, ADDRESS_MODE_SELF, ADDRESS_MODE_STACKVARIABLE, ADDRESS_MODE_STACK
from gd2c.bytecode import CallGDScriptOp, CallReturnGDScriptOp, \
    CallSelfBaseGDScriptOp, CallBuiltinGDScriptOp, OPCODE_YIELD, OPCODE_YIELDSIGNAL

if TYPE_CHECKING:
    from project import Project
    from gdscriptclass import GDScriptFunction, GDScriptClass

def annotate_coroutines(func: GDScriptFunction) -> None:
    """Analizes all function for the yield/yield signal instruction."""
    assert func
    assert func.cfg
    assert not func.cfg.is_in_ssa_form

    yields = False
    for _, op in func.ops():
        if op.opcode in (OPCODE_YIELD, OPCODE_YIELDSIGNAL):
            yields = True
            break

    func.yields = yields


def annotate_assigned_parameters(func: GDScriptFunction) -> None:
    """Analyzes the function and marks parameters as const in methods
    where they are not modified, and not passed to functions that may modify
    them.

    This is used to optimize entry into functions. GDNative functions take
    pointers to arguments, so the function may alter the value. However GDScript
    passes arguments by value which requires a copy. We can eliminate the copy
    if we know that there is no chance the value will be modified.
    """
    assert func
    assert func.cfg
    assert not func.cfg.is_in_ssa_form

    func.cfg.live_variable_analysis()
    for parameter in func.parameters():
        parameter.is_assigned = False

    def visit(node):
        # First is a simple check to see if the value is set in the node.
        # if so then the parameter is assigned
        for p in func.parameters():
            if p.is_assigned:
                continue

            addr = GDScriptAddress.calc_address(ADDRESS_MODE_STACK, p.index)
            if addr in node.defs:
                p.is_assigned = True
            else:
                addr = GDScriptAddress.calc_address(ADDRESS_MODE_STACKVARIABLE, p.index)
                if addr in node.defs:
                    p.is_assigned = True

    func.cfg.visit_nodes(visit)

def annotate_loops(func: GDScriptFunction) -> None:
    """Analyzes the function and annotates loop headers and loop exit nodes.
    TODO: Should this just be a part of cfg generation?
    """
    assert func
    assert func.cfg
    assert not func.cfg.is_in_ssa_form
