"""Tests for IR transformations."""
import unittest
from gd2c.controlflow import ControlFlowGraph, Edge, Block
from gd2c.bytecode import GDScriptOp, OPCODE_LINE, OPCODE_BREAKPOINT, DefineGDScriptOp
from gd2c.address import GDScriptAddress, ADDRESS_MODE_STACKVARIABLE
from gd2c.gdscriptclass import GDScriptFunction
from gd2c import transform


def _make_cfg(*blocks):
    cfg = ControlFlowGraph()
    for b in blocks:
        cfg.add_node(b)
    cfg.entry_node = blocks[0]
    cfg.exit_node = blocks[-1]
    for i in range(len(blocks) - 1):
        cfg.add_edge(Edge(blocks[i], blocks[i + 1]))
    return cfg


def make_func_with_debug_ops():
    entry = Block("entry")
    body = Block("body")
    exit_b = Block("exit")

    body.append_op(GDScriptOp(OPCODE_LINE))
    body.append_op(GDScriptOp(OPCODE_BREAKPOINT))
    addr = GDScriptAddress.create(ADDRESS_MODE_STACKVARIABLE, 0)
    body.append_op(DefineGDScriptOp(addr))
    body.append_op(GDScriptOp(OPCODE_LINE))

    func = GDScriptFunction("test_func", False)
    func._stack_size = 4
    func.cfg = _make_cfg(entry, body, exit_b)
    return func, body


class TestStripDebug(unittest.TestCase):
    def test_removes_debug_ops(self):
        func, body = make_func_with_debug_ops()
        self.assertEqual(len(body.ops), 4)
        changed = transform.strip_debug(func)
        self.assertTrue(changed)
        self.assertEqual(len(body.ops), 1)  # only DefineGDScriptOp remains

    def test_no_change_when_clean(self):
        entry = Block("entry")
        body = Block("body")
        exit_b = Block("exit")
        addr = GDScriptAddress.create(ADDRESS_MODE_STACKVARIABLE, 0)
        body.append_op(DefineGDScriptOp(addr))

        func = GDScriptFunction("clean", False)
        func._stack_size = 2
        func.cfg = _make_cfg(entry, body, exit_b)
        changed = transform.strip_debug(func)
        self.assertFalse(changed)
        self.assertEqual(len(body.ops), 1)


class TestUnimplementedTransforms(unittest.TestCase):
    def _make_ssa_func(self):
        entry = Block("entry")
        exit_b = Block("exit")
        func = GDScriptFunction("f", False)
        func._stack_size = 2
        cfg = _make_cfg(entry, exit_b)
        cfg.is_in_ssa_form = True
        func.cfg = cfg
        return func

    def test_substitute_intrinsics(self):
        self.assertFalse(transform.substitute_intrinsics(self._make_ssa_func()))

    def test_promote_typed_arithmetic(self):
        self.assertFalse(transform.promote_typed_arithmetic(self._make_ssa_func()))

    def test_cse(self):
        self.assertFalse(transform.common_subexpression_elimination(self._make_ssa_func()))

    def test_copy_elimination(self):
        self.assertFalse(transform.copy_elimination(self._make_ssa_func()))

    def test_phi_elimination(self):
        self.assertFalse(transform.redundant_phi_arg_elimination(self._make_ssa_func()))

    def test_dce(self):
        self.assertFalse(transform.dead_code_elimination(self._make_ssa_func()))


class TestMakeCoroutine(unittest.TestCase):
    def test_raises_not_implemented(self):
        entry = Block("entry")
        exit_b = Block("exit")
        func = GDScriptFunction("coro", False)
        func._stack_size = 2
        cfg = _make_cfg(entry, exit_b)
        cfg.is_in_ssa_form = True
        func.cfg = cfg
        with self.assertRaises(NotImplementedError):
            transform.make_coroutine(func)


if __name__ == "__main__":
    unittest.main()
