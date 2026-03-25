from __future__ import annotations
from typing import List, Optional
from gd2c.project import Project
from gd2c.gdscriptclass import GDScriptFunction
from gd2c.controlflow import build_control_flow_graph, Block
from gd2c.bytecode import OPCODE_BREAKPOINT, OPCODE_LINE, GDScriptOp, JumpToDefaultArgumentGDScriptOp

def strip_debug(func: GDScriptFunction) -> bool:
    """Strips debug instructions from bytecode.

    Arguments:
    func -- the function to strip. The function must have its control-flow graph set.
    """
    assert func
    assert func.cfg
    assert not func.cfg.is_in_ssa_form

    made_changes = False

    def visitor(block: Block):
        nonlocal made_changes
        remove: List[GDScriptOp] = []
        for op in block.ops:
            if op.opcode in (OPCODE_BREAKPOINT, OPCODE_LINE):
                remove.append(op)
                made_changes = True

        for op in remove:
            block.remove_op(op)

    func.cfg.visit_nodes(visitor)
    return made_changes

def make_coroutine(func: GDScriptFunction):
    """Transforms the function into a coroutine that can be yielded and
    resumed. The transformed function will return a coroutine script class
    instance which may or may not be completed when control is passed back
    to the caller.

    Arguments:
    func -- Any function that yields or that calls a function that yields.
    """
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form

    raise NotImplementedError("Coroutine transformation is not yet implemented")

def substitute_intrinsics(func: GDScriptFunction) -> bool:
    """Identifies calls and/or sequences of operations that can be
    substituted with more optimized versions."""
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    # TODO: implement intrinsic substitution
    return False

def promote_typed_arithmetic(func: GDScriptFunction) -> bool:
    """Evaluates arithmetic operations for opportunities to promote
    to typed arithmetic over using variant arithmetic. Type annotations
    from GDScript are assumed to be accurate. When identified, assignments
    and arithmetic operations are converted to native ones. Unboxing and
    boxing operations are inserted where necessary."""
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    # TODO: implement typed arithmetic promotion
    return False

def common_subexpression_elimination(func: GDScriptFunction, max_iterations: int = 0) -> bool:
    """Identifies expressions which are being performed more than once
    which are guaranteed to have the same result. The redundant calculations
    will be eliminated and the result of the first instance reused."""
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    # TODO: implement CSE
    return False

def copy_elimination(func: GDScriptFunction) -> bool:
    """Identifies and eliminates unnecessary copies."""
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    # TODO: implement copy elimination
    return False

def redundant_phi_arg_elimination(func: GDScriptFunction) -> bool:
    """Identifies phi ops who have the same def repeated in its operands."""
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    # TODO: implement redundant phi arg elimination
    return False

def dead_code_elimination(func: GDScriptFunction) -> bool:
    """Removes unreachable code."""
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    # TODO: implement dead code elimination
    return False
