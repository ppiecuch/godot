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

    raise NotImplementedError()

def substitute_intrinsics(func: GDScriptFunction) -> bool:
    """Identifies calls and/or sequences of operations that can be
    substituted with more optimized versions. These intrinsics are
    target-agnostic. Specific targets may implement their own
    set of intrinsic operations.
    """
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form

    print("substitute_intrinsics not implemented")
    return False

def promote_typed_arithmetic(func: GDScriptFunction) -> bool:
    """Evaluates arithmetic operations for opportunities to promote
    to typed arithmetic over using variant arithmetic. Type annotations
    from GDScript are assumed to be accurate. When identified, assignments
    and arithmetic operations are converted to native ones. Unboxing and
    boxing operations are inserted where necessary.

    An operation will be promoted to typed arithmetic if it meets the
    following criteria:
    1. The type of all operands is known
    2. The operation is a basic arithmetic operation (+, -, *, /, etc.)
    3. The result of the operation can be reused at least once before
       being boxed back into a variant.

    TODO: The above should be profiled, probably on a target basis

    An example of an operation that would not be promoted to typed
    arithmetic could be as follows:

    func mul(a: int, b: int):
        return a * b

    It may seem that the expression a * b is a perfect candidate for
    promotion because both operands are of known type, however to
    to do typed arithmetic may incur additional cost.

    (GDNative output)

    int a = gdnative->godot_variant_as_int(p_arg[0]);
    int b = gdnative->godot_variant_as_int(p_arg[1]);
    int ri = a * b
    godot_variant rv;
    gdnative->godot_variant_new_int(&rv, ri);
    return rv;

    In the above case continuing to use variants may be more
    efficient because it requires fewer calls into the gdnative api.

    godot_variant rv;
    bool valid;
    gdnative->godot_variant_evaluate(8, p_arg[0], p_arg[1], &rv, &valid);
    return rv;
    """
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    print("promote_typed_arithmetic not implemented")
    return False

def common_subexpression_elimination(func: GDScriptFunction, max_iterations: int = 0) -> bool:
    """Identifies expressions which are being performed more than once
    which are guaranteed to have the same result. The redundant calculations
    will be eliminated and the result of the first instance reused.

    Often performing this iteratively can further eliminate duplicate expressions.

    Arguments:
    func - Function to optimize. Its cfg must be populated.

    """
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    print("common_subexpression_elimination not implemented")
    return False

def copy_elimination(func: GDScriptFunction) -> bool:
    """Identifies and eliminates unnecessary copies.
    TODO: Is this even necessary? Won't cse elimination take care of this?
    """
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    print("copy_elimination not implemented")
    return False


def redundant_phi_arg_elimination(func: GDScriptFunction) -> bool:
    """Identifies phi ops who have the same def repeated in its operands.
    """
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    print("redundant_phi_arg_elimination not implemented")
    return False

def dead_code_elimination(func: GDScriptFunction) -> bool:
    """Removes unreachable code"""
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form
    print("dead_code_elimination not implemented")
    return False

