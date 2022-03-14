from __future__ import annotations
from typing import TYPE_CHECKING, Set, List, Dict
import itertools
from gd2c.domtree import build_domtree_naive, DomTree
from gd2c.bytecode import ParameterGDScriptOp, PhiGDScriptOp, DefineGDScriptOp
from gd2c.controlflow import Value, Block
from gd2c.address import *

if TYPE_CHECKING:
    from gd2c.controlflow import ControlFlowGraph, Block
    from gd2c.gdscriptclass import GDScriptFunction
    from gd2c.domtree import DomTree

def to_ssa_form(func: GDScriptFunction):
    assert func
    assert func.cfg
    assert not func.cfg.is_in_ssa_form

    func.cfg.live_variable_analysis()

    _insert_phi_ops(func)
    _rename_variables(func)

    func.cfg.is_in_ssa_form = True

def from_ssa_form(func: GDScriptFunction):
    assert func
    assert func.cfg
    assert func.cfg.is_in_ssa_form

    func.cfg.is_in_ssa_form = False

def _insert_phi_ops(func: GDScriptFunction):
    assert func.cfg
    assert func.cfg.entry_node
    cfg = func.cfg

    cfg.live_variable_analysis()
    dom = build_domtree_naive(cfg)

    defs: Set[int] = set()
    for block in cfg.nodes():
        defs = defs | block.defs

    for variable in defs:
        worklist = set([block for block in cfg.nodes() if variable in block.defs])
        has_phi: Set[Block] = set()
        visited = set(worklist)

        while any(worklist):
            block = worklist.pop()
            for d in dom.frontier(block):
                if d not in has_phi and variable in d.ins:
                    has_phi.add(d)
                    phi = PhiGDScriptOp(variable)
                    if d.first_op:
                        d.insert_ops_before(d.first_op, [phi])
                    else:
                        d.append_op(phi)

                    if d not in visited:
                        worklist.add(d)
                        visited.add(d)

def _rename_variables(func: GDScriptFunction):
    assert func
    assert func.cfg
    cfg = func.cfg
    assert cfg.entry_node

    visited: Set[Block] = set()
    stacks: Dict[int, List[int]] = {}
    values: Dict[int, List[Value]] = {}

    dom = build_domtree_naive(cfg)

    def new_value(address: int):
        nonlocal visited
        nonlocal stacks
        nonlocal values
        nonlocal func

        if address not in values:
            value = Value(address, 0)
            stacks[address] = [0]
            values[address] = [value]
            return value
        else:
            addrvalues = values[address]
            value = Value(address, len(addrvalues))
            stacks[address].append(value.version)
            addrvalues.append(value)
            return value

    def _rename_func_variables(block: Block):
        nonlocal visited
        nonlocal stacks
        nonlocal values
        nonlocal dom
        nonlocal cfg
        nonlocal func

        if block in visited:
            return

        visited.add(block)

        for op in block.ops:
            if isinstance(op, PhiGDScriptOp):
                value = new_value(op.address)
                op.ssa_dest = value

        for op in block.ops:
            for addr in op.reads:
                op.set_rhs_ssa(addr, values[addr][stacks[addr][-1]])
            for addr in op.writes:
                value = new_value(addr)
                op.set_lhs_ssa(addr, value)

        for succ in cfg.succs(block):
            for op in succ.ops:
                if isinstance(op, PhiGDScriptOp):
                    value = values[op.address][stacks[op.address][-1]]
                    op.ssa_values[block.label] = value

        for dn in dom.node(block).children():
            _rename_func_variables(dn.block)

        for op in block.ops:
            for addr in op.writes:
                stacks[addr].pop()


    _rename_func_variables(cfg.entry_node)
