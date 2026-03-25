"""Tests for bytecode opcode definitions and instruction classes."""
import unittest
from gd2c.bytecode import (
    OPCODE_OPERATOR, OPCODE_SET, OPCODE_GET, OPCODE_SETNAMED, OPCODE_GETNAMED,
    OPCODE_ASSIGN, OPCODE_ASSIGNTRUE, OPCODE_ASSIGNFALSE,
    OPCODE_CONSTRUCT, OPCODE_CONSTRUCTARRAY, OPCODE_CONSTRUCTDICTIONARY,
    OPCODE_CALL, OPCODE_CALLRETURN, OPCODE_CALLBUILTIN, OPCODE_CALLSELF,
    OPCODE_YIELD, OPCODE_YIELDSIGNAL, OPCODE_YIELDRESUME,
    OPCODE_JUMP, OPCODE_JUMPIF, OPCODE_JUMPIFNOT, OPCODE_JUMPTODEFAULTARGUMENT,
    OPCODE_RETURN, OPCODE_ITERATEBEGIN, OPCODE_ITERATE,
    OPCODE_ASSERT, OPCODE_BREAKPOINT, OPCODE_LINE, OPCODE_END,
    OPCODE_NOOP, OPCODE_BOX, OPCODE_DESTROY, OPCODE_UNBOX,
    OPCODE_DEFINE, OPCODE_INITIALIZE, OPCODE_PARAMETER, OPCODE_PHI,
    GDScriptOp, JumpGDScriptOp, JumpIfGDScriptOp, JumpIfNotGDScriptOp,
    ReturnGDScriptOp, EndGDScriptOp, DefineGDScriptOp,
    JumpToDefaultArgumentGDScriptOp, PhiGDScriptOp, ParameterGDScriptOp,
)
from gd2c.address import GDScriptAddress, ADDRESS_MODE_STACKVARIABLE


class TestOpcodeConstants(unittest.TestCase):
    def test_godot_opcodes_sequential(self):
        self.assertEqual(OPCODE_OPERATOR, 0)
        self.assertEqual(OPCODE_ASSIGN, 9)
        self.assertEqual(OPCODE_CALL, 21)
        self.assertEqual(OPCODE_JUMP, 31)
        self.assertEqual(OPCODE_RETURN, 35)
        self.assertEqual(OPCODE_BREAKPOINT, 39)
        self.assertEqual(OPCODE_LINE, 40)
        self.assertEqual(OPCODE_END, 41)

    def test_pseudo_opcodes_are_high(self):
        self.assertGreaterEqual(OPCODE_NOOP, 1000)
        self.assertGreaterEqual(OPCODE_BOX, 1000)
        self.assertGreaterEqual(OPCODE_DESTROY, 1000)
        self.assertGreaterEqual(OPCODE_DEFINE, 1000)
        self.assertGreaterEqual(OPCODE_PHI, 1000)

    def test_yield_opcodes(self):
        self.assertEqual(OPCODE_YIELD, 28)
        self.assertEqual(OPCODE_YIELDSIGNAL, 29)
        self.assertEqual(OPCODE_YIELDRESUME, 30)

    def test_iterator_opcodes(self):
        self.assertEqual(OPCODE_ITERATEBEGIN, 36)
        self.assertEqual(OPCODE_ITERATE, 37)


class TestGDScriptOp(unittest.TestCase):
    def test_create(self):
        op = GDScriptOp(OPCODE_ASSIGN)
        self.assertEqual(op.opcode, OPCODE_ASSIGN)

    def test_line_op(self):
        op = GDScriptOp(OPCODE_LINE)
        self.assertEqual(op.opcode, OPCODE_LINE)

    def test_breakpoint_op(self):
        op = GDScriptOp(OPCODE_BREAKPOINT)
        self.assertEqual(op.opcode, OPCODE_BREAKPOINT)

    def test_reached_default_false(self):
        op = GDScriptOp(OPCODE_NOOP)
        self.assertFalse(op.reached)


class TestJumpOps(unittest.TestCase):
    def test_jump_op(self):
        op = JumpGDScriptOp(100)
        self.assertEqual(op.opcode, OPCODE_JUMP)

    def test_jump_if_op(self):
        op = JumpIfGDScriptOp(100, 200, 300)
        self.assertEqual(op.opcode, OPCODE_JUMPIF)
        self.assertEqual(op.branch, 100)

    def test_jump_if_not_op(self):
        op = JumpIfNotGDScriptOp(100, 200, 300)
        self.assertEqual(op.opcode, OPCODE_JUMPIFNOT)


class TestSpecialOps(unittest.TestCase):
    def test_return_op(self):
        addr = GDScriptAddress.create(ADDRESS_MODE_STACKVARIABLE, 0)
        op = ReturnGDScriptOp(addr)
        self.assertEqual(op.opcode, OPCODE_RETURN)

    def test_end_op(self):
        op = EndGDScriptOp()
        self.assertEqual(op.opcode, OPCODE_END)

    def test_define_op(self):
        addr = GDScriptAddress.create(ADDRESS_MODE_STACKVARIABLE, 1)
        op = DefineGDScriptOp(addr)
        self.assertEqual(op.opcode, OPCODE_DEFINE)

    def test_phi_op(self):
        addr = GDScriptAddress.create(ADDRESS_MODE_STACKVARIABLE, 0)
        op = PhiGDScriptOp(addr)
        self.assertEqual(op.opcode, OPCODE_PHI)


if __name__ == "__main__":
    unittest.main()
