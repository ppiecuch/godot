"""Tests for SSA form conversion."""
import unittest
from gd2c.controlflow import ControlFlowGraph, Edge, Block
from gd2c.domtree import build_domtree_naive
from gd2c.bytecode import DefineGDScriptOp
from gd2c.address import GDScriptAddress, ADDRESS_MODE_STACKVARIABLE
from gd2c.gdscriptclass import GDScriptFunction
from gd2c import ssa


def make_simple_function_with_cfg():
    """Minimal function: entry -> body -> exit. Body defines STACKVARIABLE#0."""
    func = GDScriptFunction("test_func", False)
    func._stack_size = 4

    entry = Block("entry")
    body = Block("body")
    exit_b = Block("exit")

    addr = GDScriptAddress.create(ADDRESS_MODE_STACKVARIABLE, 0)
    body.append_op(DefineGDScriptOp(addr))

    cfg = ControlFlowGraph()
    cfg.add_node(entry)
    cfg.add_node(body)
    cfg.add_node(exit_b)
    cfg.entry_node = entry
    cfg.exit_node = exit_b
    cfg.add_edge(Edge(entry, body))
    cfg.add_edge(Edge(body, exit_b))

    func.cfg = cfg
    return func


class TestSSAConversion(unittest.TestCase):
    def test_to_and_from_ssa(self):
        func = make_simple_function_with_cfg()
        self.assertFalse(func.cfg.is_in_ssa_form)
        ssa.to_ssa_form(func)
        self.assertTrue(func.cfg.is_in_ssa_form)
        ssa.from_ssa_form(func)
        self.assertFalse(func.cfg.is_in_ssa_form)

    def test_double_to_ssa_asserts(self):
        func = make_simple_function_with_cfg()
        ssa.to_ssa_form(func)
        with self.assertRaises(AssertionError):
            ssa.to_ssa_form(func)

    def test_from_ssa_without_ssa_asserts(self):
        func = make_simple_function_with_cfg()
        with self.assertRaises(AssertionError):
            ssa.from_ssa_form(func)


if __name__ == "__main__":
    unittest.main()
