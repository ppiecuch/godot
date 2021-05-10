from __future__ import annotations
from typing import Set, List, Iterable, Callable, Dict, Optional, ClassVar, TYPE_CHECKING, cast
from gd2c.variant import VariantType
from gd2c.address import GDScriptAddress, ADDRESS_MODE_STACKVARIABLE, ADDRESS_MODE_STACK

if TYPE_CHECKING:
    from gd2c.gdscriptclass import GDScriptFunction, GDScriptFunctionParameter
    from gd2c.controlflow import Value

OPCODE_OPERATOR = 0
OPCODE_EXTENDSTEST = 1
OPCODE_ISBUILTIN = 2
OPCODE_SET = 3
OPCODE_GET = 4
OPCODE_SETNAMED = 5
OPCODE_GETNAMED = 6
OPCODE_SETMEMBER = 7
OPCODE_GETMEMBER = 8
OPCODE_ASSIGN = 9
OPCODE_ASSIGNTRUE = 10
OPCODE_ASSIGNFALSE = 11
OPCODE_ASSIGNTYPEDBUILTIN = 12
OPCODE_ASSIGNTYPEDNATIVE = 13
OPCODE_ASSIGNTYPEDSCRIPT = 14
OPCODE_CASTTOBUILTIN = 15
OPCODE_CASTTONATIVE = 16
OPCODE_CASTTOSCRIPT = 17
OPCODE_CONSTRUCT = 18
OPCODE_CONSTRUCTARRAY = 19
OPCODE_CONSTRUCTDICTIONARY = 20
OPCODE_CALL = 21
OPCODE_CALLRETURN = 22
OPCODE_CALLBUILTIN = 23
OPCODE_CALLSELF = 24
OPCODE_CALLSELFBASE = 25
OPCODE_STACK = 26
OPCODE_STACK_RETURN = 27
OPCODE_YIELD = 28
OPCODE_YIELDSIGNAL = 29
OPCODE_YIELDRESUME = 30
OPCODE_JUMP = 31
OPCODE_JUMPIF = 32
OPCODE_JUMPIFNOT = 33
OPCODE_JUMPTODEFAULTARGUMENT = 34
OPCODE_RETURN = 35
OPCODE_ITERATEBEGIN = 36
OPCODE_ITERATE = 37
OPCODE_ASSERT = 38
OPCODE_BREAKPOINT = 39
OPCODE_LINE = 40
OPCODE_END = 41
# The following are not defined by Godot but are special instructions
# for the transpiler
OPCODE_NOOP = 1000
OPCODE_BOX = 1001
OPCODE_DESTROY = 1002
OPCODE_UNBOX = 1003
OPCODE_DEFINE = 1004
OPCODE_INITIALIZE = 1005
OPCODE_PARAMETER = 1006
OPCODE_PHI = 1007
OPCODE_PARAMETER_COPY = 1008

# Comparison
OPERATOR_EQUAL = 0
OPERATOR_NOTEQUAL = 1
OPERATOR_LESS = 2
OPERATOR_LESSEQUAL = 3
OPERATOR_GREATER = 4
OPERATOR_GREATEREQUAL = 5
# Mathematic
OPERATOR_ADD = 6
OPERATOR_SUBTRACT = 7
OPERATOR_MULTIPLY = 8
OPERATOR_DIVIDE = 9
OPERATOR_NEGATE = 10
OPERATOR_POSITIVE = 11
OPERATOR_MODULE = 12
OPERATOR_STRINGCONCAT = 13
# Bitwise
OPERATOR_SHIFTLEFT = 14
OPERATOR_SHIFTRIGHT = 15
OPERATOR_BITAND = 16
OPERATOR_BITOR = 17
OPERATOR_BITXOR = 18
OPERATOR_BITNEGATE = 19
# Logic
OPERATOR_AND = 20
OPERATOR_OR = 21
OPERATOR_XOR = 22
OPERATOR_NOT = 23
# Containment
OPERATOR_IN = 24
OPERATOR_MAX = 25

OperatorToken = {
    OPERATOR_EQUAL: "==",
    OPERATOR_NOTEQUAL: "!=",
    OPERATOR_LESS: "<",
    OPERATOR_LESSEQUAL: "<=",
    OPERATOR_GREATER: ">",
    OPERATOR_GREATEREQUAL: ">=",
    OPERATOR_ADD: "+",
    OPERATOR_SUBTRACT: "-",
    OPERATOR_MULTIPLY: "*",
    OPERATOR_DIVIDE: "/",
    OPERATOR_NEGATE: "-",
    OPERATOR_POSITIVE: "+",
    OPERATOR_MODULE: "%",
    OPERATOR_STRINGCONCAT: "+",
    OPERATOR_SHIFTLEFT: "<<",
    OPERATOR_SHIFTRIGHT: ">>",
    OPERATOR_BITAND: "&",
    OPERATOR_BITOR: "|",
    OPERATOR_BITXOR: "^",
    OPERATOR_BITNEGATE: "~",
    OPERATOR_AND: "&&",
    OPERATOR_OR: "||",
    OPERATOR_XOR: "^",
    OPERATOR_NOT: "!",
    OPERATOR_IN: "in",
    OPERATOR_MAX: "max"
}

def sv2stack(addr: int):
    """Translate ADDRESS_MODE_STACKVARIABLE to ADDRESS_MODE_STACK"""
    gdaddr = GDScriptAddress(addr)
    if gdaddr.mode == ADDRESS_MODE_STACKVARIABLE:
        return GDScriptAddress.create(ADDRESS_MODE_STACK, gdaddr.offset).address
    else:
        return addr

class GDScriptOp:
    _opcode: int
    _reads: Set[int]
    _writes: Set[int]
    reached: bool

    def __init__(self, opcode: int):
        self._opcode = opcode
        self._reads: Set[int] = set()
        self._writes: Set[int] = set()
        self.reached = False

    def set_rhs_ssa(self, addr: int, value: Value):
        pass

    def set_lhs_ssa(self, addr: int, value: Value):
        pass

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        raise NotImplementedError

    @property
    def opcode(self):
        return self._opcode

    @property
    def reads(self):
        return self._reads
    @reads.setter
    def reads(self, value: Set[int]):
        if value is None:
            raise "value may not be None"
        self._reads = value

    @property
    def writes(self):
        return self._writes
    @writes.setter
    def writes(self, value: Set[int]):
        if value is None:
            raise "value may not be None"
        self._writes = value

    @property
    def stride(self) -> int:
        raise Exception("not implemented")

    @property
    def has_side_effects(self) -> bool:
        return False

    @property
    def is_branch(self) -> bool:
        return False

class NoopGDScriptOp(GDScriptOp):
    def __init__(self):
        super().__init__(OPCODE_NOOP)

    def __str__(self):
        return "NOOP"

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        pass

    @property
    def stride(self) -> int:
        return 1

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'NoopGDScriptOp':
        return NoopGDScriptOp()

class ExtendsTestGDScriptOp(GDScriptOp):
    def __init__(self, a: int, b: int, dest: int):
        super().__init__(OPCODE_EXTENDSTEST)
        self.a = a
        self.b = b
        self.dest = dest
        self.ssa_a = None
        self.ssa_b = None
        self.ssa_dest = None
        self._reads = set([a, b])
        self._writes = set([dest])

    def __str__(self):
        return "EXTENDS_TEST"

    @property
    def stride(self) -> int:
        return 4

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.a == old_address:
            self.a = new_address
            dirty = True
        if self.b == old_address:
            self.b = new_address
            dirty = True
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if update_def_use and dirty:
            self._reads = set([self.a, self.b])
            self._writes = set([self.dest])

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'ExtendsTestGDScriptOp':
        return ExtendsTestGDScriptOp(
            sv2stack(bytecode[index + 1]),
            sv2stack(bytecode[index + 2]),
            sv2stack(bytecode[index + 3]))

class IsBuiltInGDScriptOp(GDScriptOp):
    def __init__(self, a: int, type_code: int, dest: int):
        super().__init__(OPCODE_ISBUILTIN)
        self.a = a
        self.type_code = type_code
        self.dest = dest
        self.ssa_a = None
        self.ssa_dest = None
        self._reads = set([a])
        self._writes = set([dest])

    def __str__(self):
        return "IS_BUILTIN"

    @property
    def stride(self) -> int:
        return 4

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.a == old_address:
            self.a = new_address
            dirty = True
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if update_def_use and dirty:
            self._reads = set([self.a])
            self._writes = set([self.dest])

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'IsBuiltInGDScriptOp':
        return IsBuiltInGDScriptOp(
            sv2stack(bytecode[index + 1]),
            bytecode[index + 2],
            sv2stack(bytecode[index + 3]))

class OperatorGDScriptOp(GDScriptOp):
    op: int
    dest: int
    operand1: int
    operand2: int
    ssa_dest: Optional[Value]
    ssa_operand1: Optional[Value]
    ssa_operand2: Optional[Value]

    def __init__(self, op: int, operand1: int, operand2: int, dest: int):
        super().__init__(OPCODE_OPERATOR)
        self.op = op
        self.dest = dest
        self.operand1 = operand1
        self.operand2 = operand2
        self._writes = set([self.dest])
        self._reads = set([self.operand1, self.operand2])
        self.ssa_dest = None
        self.ssa_operand1 = None
        self.ssa_operand2 = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.operand1 == old_address:
            self.operand1 = new_address
            dirty = True
        if self.operand2 == old_address:
            self.operand2 = new_address
            dirty = True
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if update_def_use and dirty:
            self._reads = set([self.operand1, self.operand2])
            self._writes = set([self.dest])

    def set_lhs_ssa(self, addr: int, value: Value):
        self.ssa_dest = value

    def set_rhs_ssa(self, addr: int, value: Value):
        if self.operand1 == addr:
            self.ssa_operand1 = value
        if self.operand2 == addr:
            self.ssa_operand2 = value

    def __str__(self):
        if self.ssa_dest:
            return f"OPER {self.ssa_dest} = {self.ssa_operand1} {OperatorToken[self.op]} {self.ssa_operand2}"
        else:
            return f"OPER {self.dest} = {self.operand1} {OperatorToken[self.op]} {self.operand2}"

    @property
    def stride(self) -> int:
        return 5

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'OperatorGDScriptOp':
        return OperatorGDScriptOp(
            bytecode[index + 1],
            sv2stack(bytecode[index + 2]),
            sv2stack(bytecode[index + 3]),
            sv2stack(bytecode[index + 4]))

class SetGDScriptOp(GDScriptOp):
    array_address: int
    index_address: int
    source_address: int
    ssa_array: Optional[Value]
    ssa_index: Optional[Value]
    ssa_source: Optional[Value]

    def __init__(self, array_address: int, index_address: int, source_address: int):
        super().__init__(OPCODE_SET)
        self.array_address = array_address
        self.index_address = index_address
        self.source_address = source_address
        self._reads = set([self.source_address, self.array_address, self.index_address])
        self._writes = set([self.array_address])
        self.ssa_array = None
        self.ssa_index = None
        self.ssa_source = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.array_address == old_address:
            self.array_address = new_address
            dirty = True
        if self.index_address == old_address:
            self.index_address = new_address
            dirty = True
        if self.source_address == old_address:
            self.source_address = new_address
            dirty = True
        if update_def_use and dirty:
            self._reads = set([self.source_address, self.array_address, self.index_address])
            self._writes = set([self.array_address])

    def __str__(self):
        if self.ssa_array:
            return f"SET {self.ssa_array}[{self.ssa_index}] = {self.ssa_source}"
        else:
            return f"SET {self.array_address}[{self.index_address}] = {self.source_address}"

    @property
    def stride(self) -> int:
        return 4

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'SetGDScriptOp':
        return SetGDScriptOp(
            sv2stack(bytecode[index + 1]),
            sv2stack(bytecode[index + 2]),
            sv2stack(bytecode[index + 3]))

class GetGDScriptOp(GDScriptOp):
    dest: int
    array_address: int
    index_address: int
    ssa_dest: Optional[Value]
    ssa_array: Optional[Value]
    ssa_index: Optional[Value]

    def __init__(self, dest: int, array_address: int, index_address: int):
        super().__init__(OPCODE_GET)
        self.dest = dest
        self.array_address = array_address
        self.index_address = index_address
        self._writes = set([self.dest])
        self._reads = set([self.array_address, self.index_address])
        self.ssa_dest = None
        self.ssa_array = None
        self.ssa_index = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if self.array_address == old_address:
            self.array_address = new_address
            dirty = True
        if self.index_address == old_address:
            self.index_address = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set([self.array_address, self.index_address])

    def __str__(self):
        return f"GET {self.dest} = {self.array_address}[{self.index_address}]"

    @property
    def stride(self) -> int:
        return 4

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'GetGDScriptOp':
        return GetGDScriptOp(
            sv2stack(bytecode[index + 3]),
            sv2stack(bytecode[index + 1]),
            sv2stack(bytecode[index + 2]))

class SetNamedGDScriptOp(GDScriptOp):
    dest: int
    name_index: int
    source: int
    ssa_dest: Optional[Value]
    ssa_source: Optional[Value]

    def __init__(self, dest: int, name_index: int, source: int):
        super().__init__(OPCODE_SETNAMED)
        self.dest = dest
        self.name_index = name_index
        self.source = source
        self._writes = set([self.dest])
        self._reads = set([self.source])
        self.ssa_dest = None
        self.ssa_source = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if self.source == old_address:
            self.source = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set([self.source])

    def __str__(self):
        return f"SETNAMED {self.dest}.global_names[{self.name_index}] = {self.source}"

    @property
    def stride(self) -> int:
        return 4

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'SetNamedGDScriptOp':
        return SetNamedGDScriptOp(
            sv2stack(bytecode[index + 1]),
            bytecode[index + 2],
            sv2stack(bytecode[index + 3]))

class GetNamedGDScriptOp(GDScriptOp):
    dest: int
    name_index: int
    source: int
    ssa_dest: Optional[Value]
    ssa_source: Optional[Value]

    def __init__(self, dest: int, source: int, name_index: int):
        super().__init__(OPCODE_GETNAMED)
        self.dest = dest
        self.name_index = name_index
        self.source = source
        self._writes = set([self.dest])
        self._reads = set([self.source])
        self.ssa_dest = None
        self.ssa_source = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if self.source == old_address:
            self.source = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set([self.source])

    def __str__(self):
        return f"GETNAMED {self.dest} = {self.source}.global_names[{self.name_index}]"

    @property
    def stride(self) -> int:
        return 4

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'GetNamedGDScriptOp':
        return GetNamedGDScriptOp(
            sv2stack(bytecode[index + 3]),
            sv2stack(bytecode[index + 1]),
            bytecode[index + 2])

class SetMemberGDScriptOp(GDScriptOp):
    name_index: int
    source: int
    ssa_source: Optional[Value]

    def __init__(self, name_index: int, source: int):
        super().__init__(OPCODE_SETMEMBER)
        self.name_index = name_index
        self.source = source
        self._reads = set([source])
        self._writes = set([])
        self.ssa_source = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.source == old_address:
            self.source = new_address
            dirty = True
        if update_def_use and dirty:
            self._reads = set([self.source])
            self._writes = set([])

    def __str__(self):
        return f"SETMEMBER {self.receiver}.{self.name_index} = {self.source}"

    @property
    def stride(self) -> int:
        return 3

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'SetMemberGDScriptOp':
        return SetMemberGDScriptOp(
            bytecode[index + 1],
            sv2stack(bytecode[index + 2]))

class GetMemberGDScriptOp(GDScriptOp):
    dest: int
    name_index: int
    ssa_dest: Optional[Value]

    def __init__(self, dest: int, name_index: int):
        super().__init__(OPCODE_GETMEMBER)
        self.dest = dest
        self.name_index = name_index
        self._writes = set([dest])
        self._reads = set([])
        self.ssa_dest = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if update_def_use and dirty:
            self._reads = set([])
            self._writes = set([self.dest])

    def __str__(self):
        return f"GETMEMBER {self.dest} = self.{self.name_index}"

    @property
    def stride(self) -> int:
        return 3

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'GetMemberGDScriptOp':
        return GetMemberGDScriptOp(
            sv2stack(bytecode[index + 2]),
            bytecode[index + 1])

class AssignGDScriptOp(GDScriptOp):
    dest: int
    source: int
    ssa_dest: Optional[Value]
    ssa_source: Optional[Value]

    def __init__(self, dest: int, source: int):
        super().__init__(OPCODE_ASSIGN)
        self.dest = dest
        self.source = source
        self._writes = set([dest])
        self._reads = set([source])
        self.ssa_dest = None
        self.ssa_source = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if self.source == old_address:
            self.source = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set([self.source])

    def set_lhs_ssa(self, addr: int, value: Value):
        self.ssa_dest = value

    def set_rhs_ssa(self, addr: int, value: Value):
        self.ssa_source = value

    def __str__(self):
        if self.ssa_dest:
            return f"ASSIGN {self.ssa_dest} = {self.ssa_source}"
        else:
            return f"ASSIGN {self.dest} = {self.source}"

    @property
    def stride(self) -> int:
        return 3

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'AssignGDScriptOp':
        return AssignGDScriptOp(
            sv2stack(bytecode[index + 1]),
            sv2stack(bytecode[index + 2]))

class AssignTrueGDScriptOp(GDScriptOp):
    dest: int
    ssa_dest: Optional[Value]

    def __init__(self, dest: int):
        super().__init__(OPCODE_ASSIGNTRUE)
        self.dest = dest
        self._writes = set([dest])
        self.ssa_dest = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set([])

    def __str__(self):
        return f"ASGNTRUE {self.dest}"

    @property
    def stride(self) -> int:
        return 2

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'AssignTrueGDScriptOp':
        return AssignTrueGDScriptOp(sv2stack(bytecode[index + 1]))

class AssignFalseGDScriptOp(GDScriptOp):
    dest: int
    ssa_dest: Optional[Value]

    def __init__(self, dest: int):
        super().__init__(OPCODE_ASSIGNFALSE)
        self.dest = dest
        self._writes = set([dest])

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set([])

    def __str__(self):
        return f"ASGNFALSE {self.dest}"

    @property
    def stride(self) -> int:
        return 2

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'AssignFalseGDScriptOp':
        return AssignFalseGDScriptOp(sv2stack(bytecode[index + 1]))

class AssignTypedBuiltinGDScriptOp(GDScriptOp):
    dest: int
    source: int
    ssa_dest: Optional[Value]
    ssa_source: Optional[Value]

    def __init__(self, vtype: int, dest: int, source: int):
        super().__init__(OPCODE_ASSIGNTYPEDBUILTIN)
        self.vtype = VariantType.get(vtype)
        self.dest = dest
        self.source = source
        self._writes = set([dest])
        self._reads = set([source])
        self.ssa_dest = None
        self.ssa_source = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if self.source == old_address:
            self.source = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set([self.source])

    def __str__(self):
        return f"ASGNBI {self.dest}"

    @property
    def stride(self) -> int:
        return 4

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'AssignTypedBuiltinGDScriptOp':
        return AssignTypedBuiltinGDScriptOp(
            bytecode[index + 1],
            sv2stack(bytecode[index + 2]),
            sv2stack(bytecode[index + 3]))

class ReturnGDScriptOp(GDScriptOp):
    source: int
    ssa_source: Optional[Value]

    def __init__(self, source: int):
        super().__init__(OPCODE_RETURN)
        self.source = source
        self._reads = set([source])
        self.ssa_source = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.source == old_address:
            self.source = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([])
            self._reads = set([self.source])

    def __str__(self):
        return f"RETURN {self.source}"

    @property
    def stride(self) -> int:
        return 2

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'ReturnGDScriptOp':
        return ReturnGDScriptOp(sv2stack(bytecode[index + 1]))

    @property
    def is_branch(self) -> bool:
        return True

class ConstructGDScriptOp(GDScriptOp):
    vtype: VariantType
    arg_count: int
    args: List[int]
    dest: int
    ssa_args: List[Value]
    ssa_dest: Optional[Value]

    def __init__(self, vtype: int, arg_count: int, args: Iterable[int], dest: int):
        super().__init__(OPCODE_CONSTRUCT)
        self.vtype = VariantType.get(vtype)
        self.arg_count = arg_count
        self.args = list(args)[:]
        self.dest = dest
        self._writes = set([dest])
        self._reads = set(self.args)
        self.ssa_dest = None
        self.ssa_args = []

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        for i in range(len(self.args)):
            if self.args[i] == old_address:
                self.args[i] = new_address
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set(self.args)

    def __str__(self):
        return f"CONSTRU {self.dest} = ..."

    @property
    def stride(self) -> int:
        return 4 + self.arg_count

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'ConstructGDScriptOp':
        count = bytecode[index + 2]
        return ConstructGDScriptOp(
            bytecode[index + 1],
            count,
            [sv2stack(a) for a in bytecode[index + 3 : index + 3 + count]],
            sv2stack(bytecode[index + 3 + count]))

class ConstructArrayGDScriptOp(GDScriptOp):
    item_count: int
    item_addresses: List[int]
    dest: int
    ssa_dest: Optional[Value]
    ssa_items: List[Value]

    def __init__(self, dest: int, item_count: int, item_addresses: Iterable[int]):
        super().__init__(OPCODE_CONSTRUCTARRAY)
        self.item_count = item_count
        self.item_addresses = list(item_addresses)[:]
        self.dest = dest
        self._writes = set([dest])
        self._reads = set(self.item_addresses)
        self.ssa_dest = None
        self.ssa_items = []

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        for i in range(len(self.item_addresses)):
            if self.item_addresses[i] == old_address:
                self.item_addresses[i] = new_address
                dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set(self.item_addresses)

    def __str__(self):
        return f"NEWARRAY {self.dest} = ..."

    @property
    def stride(self) -> int:
        return 3 + self.item_count

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'ConstructArrayGDScriptOp':
        count = bytecode[index + 1]
        return ConstructArrayGDScriptOp(
            sv2stack(bytecode[index + 2 + count]),
            count,
            [sv2stack(a) for a in bytecode[index + 2 : index + 2 + count]])

class ConstructDictionaryGDScriptOp(GDScriptOp):
    item_count: int
    key_addresses: List[int]
    value_addresses: List[int]
    dest: int
    ssa_keys: List[Value]
    ssa_values: List[Value]
    ssa_dest: Optional[Value]

    def __init__(self, dest: int, item_count: int, key_addresses: Iterable[int], value_addresses: Iterable[int]):
        super().__init__(OPCODE_CONSTRUCTDICTIONARY)
        self.item_count = item_count
        self.key_addresses = list(key_addresses)[:]
        self.value_addresses = list(value_addresses)[:]
        self.dest = dest
        self._writes = set([dest])
        self._reads = set(self.key_addresses) | set(self.value_addresses)
        self.ssa_keys = []
        self.ssa_values = []
        self.ssa_dest = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        for i in range(len(self.key_addresses)):
            if self.key_addresses[i] == old_address:
                self.key_addresses[i] = new_address
                dirty = True
        for i in range(len(self.value_addresses)):
            if self.value_addresses[i] == old_address:
                self.value_addresses[i] = new_address
                dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set(self.key_addresses) | set(self.value_addresses)

    def __str__(self):
        return f"NEWARRAY {self.dest} = ..."

    @property
    def stride(self) -> int:
        return 3 + self.item_count * 2

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'ConstructDictionaryGDScriptOp':
        count = bytecode[index + 1]
        keys = []
        values = []

        for i in range(count):
            keys.append(sv2stack(bytecode[index + 2 + i * 2 + 0]))
            values.append(sv2stack(bytecode[index + 2 + i * 2 + 1]))

        return ConstructDictionaryGDScriptOp(
            sv2stack(bytecode[index + 2 + count * 2]),
            count,
            keys,
            values)

class CallReturnGDScriptOp(GDScriptOp):
    dest: int
    arg_count: int
    name_index: int
    args: List[int]
    receiver: int
    ssa_dest: Optional[Value]
    ssa_args: List[Value]
    ssa_receiver: Optional[Value]

    def __init__(self, dest: int, receiver: int, arg_count: int, name_index: int, args: Iterable[int]):
        super().__init__(OPCODE_CALLRETURN)
        self.dest = dest
        self.arg_count = arg_count
        self.name_index = name_index
        self.args = list(args)[:]
        self.receiver = receiver
        self._writes = set([dest])
        self._reads = set([receiver]) | set(self.args)
        self.ssa_dest = None
        self.ssa_args = []
        self.ssa_receiver = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if self.receiver == old_address:
            self.receiver = new_address
            dirty = True
        for i in range(len(self.args)):
            if self.args[i] == old_address:
                self.args[i] = new_address
                dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set([self.receiver]) | set(self.args)

    def __str__(self):
        return f"CALLRET {self.receiver}.{self.name_index}(...)"

    @property
    def stride(self) -> int:
        return 5 + self.arg_count

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'CallReturnGDScriptOp':
        count = bytecode[index + 1]
        return CallReturnGDScriptOp(
            sv2stack(bytecode[index + 4 + count]),
            sv2stack(bytecode[index + 2]),
            count,
            bytecode[index + 3],
            [sv2stack(a) for a in bytecode[index + 4 : index + 4 + count]])

class CallGDScriptOp(GDScriptOp):
    arg_count: int
    name_index: int
    args: List[int]
    receiver: int
    ssa_args: List[Value]
    ssa_receiver: Optional[Value]

    def __init__(self, receiver: int, arg_count: int, name_index: int, args: Iterable[int]):
        super().__init__(OPCODE_CALL)
        self.arg_count = arg_count
        self.name_index = name_index
        self.args = list(args)[:]
        self.receiver = receiver
        self._reads = set([receiver]) | set(self.args)
        self.ssa_args = []
        self.ssa_receiver = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.receiver == old_address:
            self.receiver = new_address
            dirty = True
        for i in range(len(self.args)):
            if self.args[i] == old_address:
                self.args[i] = new_address
                dirty = True
        if update_def_use and dirty:
            self._writes = set([])
            self._reads = set([self.receiver]) | set(self.args)

    def __str__(self):
        return f"CALL {self.receiver}.{self.name_index}(...)"

    @property
    def stride(self) -> int:
        return 5 + self.arg_count

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'CallGDScriptOp':
        count = bytecode[index + 1]
        return CallGDScriptOp(
            sv2stack(bytecode[index + 2]),
            count,
            bytecode[index + 3],
            [sv2stack(a) for a in bytecode[index + 4 : index + 4 + count]])

class CallSelfBaseGDScriptOp(GDScriptOp):
    dest: int
    arg_count: int
    name_index: int
    args: List[int]
    ssa_dest: Optional[Value]
    ssa_args: List[Value]

    def __init__(self, dest: int, arg_count: int, name_index: int, args: Iterable[int]):
        super().__init__(OPCODE_CALLSELFBASE)
        self.dest = dest
        self.arg_count = arg_count
        self.name_index = name_index
        self.args = list(args)[:]
        self._reads = set(self.args)
        self._writes = set([self.dest])

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        for i in range(len(self.args)):
            if self.args[i] == old_address:
                self.args[i] = new_address
                dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set(self.args)

    def __str__(self):
        return f"CALLBASE {self.dest} = self.{self.name_index}(...)"

    @property
    def stride(self) -> int:
        return 4 + self.arg_count

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'CallSelfBaseGDScriptOp':
        count = bytecode[index + 1]
        return CallSelfBaseGDScriptOp(
            sv2stack(bytecode[index + 3 + count]),
            count,
            bytecode[index + 1],
            [sv2stack(a) for a in bytecode[index + 3 : index + 3 + count]])

class CallBuiltinGDScriptOp(GDScriptOp):
    dest: int
    arg_count: int
    function_index: int
    args: List[int]
    ssa_dest: Optional[Value]
    ssa_args: List[Value]

    def __init__(self, dest: int, function_index: int, arg_count: int, args: Iterable[int]):
        super().__init__(OPCODE_CALLBUILTIN)
        self.dest = dest
        self.arg_count = arg_count
        self.function_index = function_index
        self.args = list(args)[:]
        self._writes = set([dest])
        self._reads = set(self.args)
        self.ssa_dest = None
        self.ssa_args = []

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        for i in range(len(self.args)):
            if self.args[i] == old_address:
                self.args[i] = new_address
                dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set(self.args)

    def __str__(self):
        return f"CALLBI {self.dest} = builtin[{self.function_index}](...)"

    @property
    def stride(self) -> int:
        return 4 + self.arg_count

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'CallBuiltinGDScriptOp':
        fid = bytecode[index + 1]
        count = bytecode[index + 2]
        args = [sv2stack(a) for a in bytecode[index + 3 : index + 3 + count]]
        dest = sv2stack(bytecode[index + 3 + count])
        return CallBuiltinGDScriptOp(dest, fid, count, args)

class JumpGDScriptOp(GDScriptOp):
    branch: int
    fallthrough: int

    def __init__(self, branch: int):
        super().__init__(OPCODE_JUMP)
        self.branch = branch
        self.fallthrough = branch

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        pass

    def __str__(self):
        return f"JUMP {self.branch}"

    @property
    def stride(self) -> int:
        return 2

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'JumpGDScriptOp':
        return JumpGDScriptOp(bytecode[index + 1])

    @property
    def is_branch(self) -> bool:
        return True

class JumpIfGDScriptOp(GDScriptOp):
    branch: int
    condition: int
    fallthrough: int
    ssa_condition: Optional[Value]

    def __init__(self, branch: int, condition: int, fallthrough: int):
        super().__init__(OPCODE_JUMPIF)
        self.branch = branch
        self.condition = condition
        self.fallthrough = fallthrough
        self._reads = set([condition])
        self.ssa_condition = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.condition == old_address:
            self.condition = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([])
            self._reads = set([self.condition])

    def set_rhs_ssa(self, addr: int, value: Value):
        if self.condition == addr:
            self.ssa_condition = value

    def __str__(self):
        if self.ssa_condition:
            return f"JUMPIF {self.ssa_condition} ? {self.branch} : {self.fallthrough}"
        else:
            return f"JUMPIF {self.condition} ? {self.branch} : {self.fallthrough}"

    @property
    def stride(self) -> int:
        return 3

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'JumpIfGDScriptOp':
        return JumpIfGDScriptOp(bytecode[index + 2], sv2stack(bytecode[index + 1]), index + 3)

    @property
    def is_branch(self) -> bool:
        return True

class JumpIfNotGDScriptOp(GDScriptOp):
    branch: int
    condition: int
    fallthrough: int
    ssa_condition: Optional[Value]

    def __init__(self, branch: int, condition: int, fallthrough: int):
        super().__init__(OPCODE_JUMPIFNOT)
        self.branch = branch
        self.fallthrough = fallthrough
        self.condition = condition
        self._reads = set([condition])
        self.ssa_condition: Optional[Value] = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.condition == old_address:
            self.condition = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([])
            self._reads = set([self.condition])

    def __str__(self):
        return f"JUMPIFNT {self.condition} ? {self.branch}"

    @property
    def stride(self) -> int:
        return 3

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'JumpIfNotGDScriptOp':
        return JumpIfNotGDScriptOp(bytecode[index + 2], sv2stack(bytecode[index + 1]), index + 3)

    @property
    def is_branch(self) -> bool:
        return True

class JumpToDefaultArgumentGDScriptOp(GDScriptOp):
    jump_table: List[int]
    fallthrough: int

    def __init__(self, jump_table: Iterable[int], fallthrough: int):
        super().__init__(OPCODE_JUMPTODEFAULTARGUMENT)
        self.jump_table = list(jump_table)[:]
        self.fallthrough = fallthrough

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        pass

    def __str__(self):
        jt = ", ".join([f"{i}: {str(v)}" for i, v in enumerate(self.jump_table)])
        return f"DEFAULT {jt}, {len(self.jump_table)}: {self.fallthrough}"

    @property
    def stride(self) -> int:
        return 1

    @property
    def is_branch(self) -> bool:
        return True

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'JumpToDefaultArgumentGDScriptOp':
        return JumpToDefaultArgumentGDScriptOp(func.default_arguments_jump_table, index + 1)

class LineGDScriptOp(GDScriptOp):
    line_number: int

    def __init__(self, line_number: int):
        super().__init__(OPCODE_LINE)
        self.line_number = line_number

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        pass

    def __str__(self):
        return f"LINE {self.line_number}"

    @property
    def stride(self) -> int:
        return 2

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'LineGDScriptOp':
        return LineGDScriptOp(bytecode[index + 1])

class EndGDScriptOp(GDScriptOp):
    def __init__(self):
        super().__init__(OPCODE_END)

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        pass

    def __str__(self):
        return f"END (jump to exit block)"

    @property
    def stride(self) -> int:
        return 1

    @property
    def is_branch(self) -> bool:
        return True

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'EndGDScriptOp':
        return EndGDScriptOp()

class IterateBeginGDScriptOp(GDScriptOp):
    counter: int
    container: int
    branch: int
    fallthrough: int
    iterator: int
    ssa_counter: Optional[Value]
    ssa_container: Optional[Value]
    ssa_iterator: Optional[Value]

    def __init__(self, counter: int, container: int, branch: int, fallthrough: int, iterator: int):
        super().__init__(OPCODE_ITERATEBEGIN)
        self.counter = counter
        self.container = container
        self.branch = branch
        self.fallthrough = fallthrough
        self.iterator = iterator
        self.ssa_counter: None
        self.ssa_container: None
        self.ssa_iterator: None

        self._writes = set([iterator, counter])
        self._reads = set([container])

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.counter == old_address:
            self.counter = new_address
            dirty = True
        if self.container == old_address:
            self.container = new_address
            dirty = True
        if self.iterator == old_address:
            self.iterator = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.iterator, self.counter])
            self._reads = set([self.container])

    def __str__(self):
        return f"ITERATE BEGIN: BRANCH {self.branch} : FALLTHROUGH {self.fallthrough}"

    @property
    def stride(self) -> int:
        return 5

    @property
    def is_branch(self) -> bool:
        return True

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'IterateBeginGDScriptOp':
        counter = sv2stack(bytecode[index + 1])
        container = sv2stack(bytecode[index + 2])
        branch = bytecode[index + 3]
        iterator = sv2stack(bytecode[index + 4])
        fallthrough = index + 5
        return IterateBeginGDScriptOp(counter, container, branch, fallthrough, iterator)

class IterateGDScriptOp(GDScriptOp):
    counter: int
    container: int
    branch: int
    fallthrough: int
    iterator: int
    ssa_counter: Optional[Value]
    ssa_container: Optional[Value]
    ssa_iterator: Optional[Value]

    def __init__(self, counter: int, container: int, branch: int, fallthrough: int, iterator: int):
        super().__init__(OPCODE_ITERATE)
        self.counter = counter
        self.container = container
        self.branch = branch
        self.fallthrough = fallthrough
        self.iterator = iterator
        self.ssa_counter = None
        self.ssa_container = None
        self.ssa_iterator = None

        self._writes = set([iterator, counter])
        self._reads = set([container])

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.counter == old_address:
            self.counter = new_address
            dirty = True
        if self.container == old_address:
            self.container = new_address
            dirty = True
        if self.iterator == old_address:
            self.iterator = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.iterator, self.counter])
            self._reads = set([self.container])

    def __str__(self):
        return f"ITERATE: FALLTHROUGH {self.fallthrough} : EXIT {self.branch}"

    @property
    def stride(self) -> int:
        return 5

    @property
    def is_branch(self) -> bool:
        return True

    @staticmethod
    def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> 'IterateGDScriptOp':
        counter = sv2stack(bytecode[index + 1])
        container = sv2stack(bytecode[index + 2])
        branch = bytecode[index + 3]
        iterator = sv2stack(bytecode[index + 4])
        fallthrough = index + 5
        return IterateGDScriptOp(counter, container, branch, fallthrough, iterator)

class PseudoGDScriptOp(GDScriptOp):
    def __init__(self, opcode: int):
        super().__init__(opcode)

    @property
    def stride(self) -> int:
        raise Exception("Pseudo-ops have no stride as they do not have a bytecode representation")

class ParameterGDScriptOp(PseudoGDScriptOp):
    """Creates an SSA value from a parameter"""

    parameter: GDScriptFunctionParameter

    def __init__(self, parameter: GDScriptFunctionParameter):
        super().__init__(OPCODE_PARAMETER)
        self.parameter = parameter
        self._writes = set([GDScriptAddress.calc_address(ADDRESS_MODE_STACK, parameter.index)])

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        # TODO: Maybe this needs to be implemeted for this instruction??
        pass

class DefineGDScriptOp(PseudoGDScriptOp):
    address: int
    ssa_address: Optional[Value]

    def __init__(self, address: int):
        super().__init__(OPCODE_DEFINE)
        self.address = address
        self.ssa_address = None
        self.writes = set([address])

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.address == old_address:
            self.address = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.address])
            self._reads = set([])

    def set_lhs_ssa(self, addr: int, value: Value):
        self.ssa_address = value

    def __str__(self):
        if self.ssa_address:
            return f"DEFINE {self.ssa_address} <- {self.address}"
        else:
            return f"DEFINE {self.address}"

class InitializeGDScriptOp(PseudoGDScriptOp):
    address: int
    ssa_address: Optional[Value]

    def __init__(self, address: int):
        super().__init__(OPCODE_INITIALIZE)
        self.address = address
        self._writes = set([address])
        self.ssa_address = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.address == old_address:
            self.address = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.address])
            self._reads = set([])

    def __str__(self):
        return f"INIT {self.address}"

class DestroyGDScriptOp(PseudoGDScriptOp):
    address: int
    ssa_address: Optional[Value]

    def __init__(self, address: int):
        super().__init__(OPCODE_DESTROY)
        self.address = address
        self._writes = set([address])
        self.ssa_address = None

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.address == old_address:
            self.address = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.address])
            self._reads = set([])

    def __str__(self):
        return f"DESTROY {self.address}"

    @property
    def has_side_effects(self) -> bool:
        return True

class PhiGDScriptOp(PseudoGDScriptOp):
    address: int
    ssa_dest: Optional[Value]
    ssa_values: Dict[str, Value]

    def __init__(self, address: int):
        super().__init__(OPCODE_PHI)
        self.address = address
        self.ssa_dest = None
        self.ssa_values = {}

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.address == old_address:
            self.address = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.address])
            self._reads = set([])

    def __str__(self):
        if self.ssa_dest:
            d = f"{self.ssa_dest}" if self.ssa_dest is None else self.ssa_dest
            v = ", ".join([f"({k}: {v})" for k, v in self.ssa_values.items()])
            return f"PHI {d} = {v}"
        else:
            return f"PHI {self.address} = ?"

class CopyParameterGDScriptOp(PseudoGDScriptOp):
    dest: int
    parameter: GDScriptFunctionParameter
    ssa_dest: Optional[Value]
    ssa_parameter: Optional[Value]

    def __init__(self, parameter: GDScriptFunctionParameter, dest: int):
        super().__init__(OPCODE_PARAMETER_COPY)
        self.parameter = parameter
        self.dest = dest
        self.ssa_dest = None
        self.ssa_parameter = None

        self._writes = set([self.dest])
        self._reads = set([self.parameter.address.address])

    def replace_address(self, old_address: int, new_address: int, update_def_use: bool = True) -> None:
        dirty = False
        if self.dest == old_address:
            self.dest = new_address
            dirty = True
        if update_def_use and dirty:
            self._writes = set([self.dest])
            self._reads = set([self.parameter.address.address])

    def __str__(self):
        if self.ssa_dest:
            d = f"{self.ssa_dest}" if self.ssa_dest is None else self.ssa_dest
            v = ", ".join([f"({k}: {v})" for k, v in self.ssa_values.items()])
            return f"PHI {d} = {v}"
        else:
            return f"PHI {self.address} = ?"

_extractors: Dict[int, Optional[Callable[[GDScriptFunction, List[int], int], GDScriptOp]]] = {
    OPCODE_OPERATOR: OperatorGDScriptOp.extract,
    OPCODE_EXTENDSTEST: ExtendsTestGDScriptOp.extract,
    OPCODE_ISBUILTIN: IsBuiltInGDScriptOp.extract,
    OPCODE_SET: SetGDScriptOp.extract,
    OPCODE_GET: GetGDScriptOp.extract,
    OPCODE_SETNAMED: SetNamedGDScriptOp.extract,
    OPCODE_GETNAMED: GetNamedGDScriptOp.extract,
    OPCODE_SETMEMBER: SetMemberGDScriptOp.extract,
    OPCODE_GETMEMBER: GetMemberGDScriptOp.extract,
    OPCODE_ASSIGN: AssignGDScriptOp.extract,
    OPCODE_ASSIGNTRUE: AssignTrueGDScriptOp.extract,
    OPCODE_ASSIGNFALSE: AssignFalseGDScriptOp.extract,
    OPCODE_ASSIGNTYPEDBUILTIN: AssignTypedBuiltinGDScriptOp.extract,
    OPCODE_ASSIGNTYPEDNATIVE: None,
    OPCODE_ASSIGNTYPEDSCRIPT: None,
    OPCODE_CASTTOBUILTIN: None,
    OPCODE_CASTTONATIVE: None,
    OPCODE_CASTTOSCRIPT: None,
    OPCODE_CONSTRUCT: ConstructGDScriptOp.extract,
    OPCODE_CONSTRUCTARRAY: ConstructArrayGDScriptOp.extract,
    OPCODE_CONSTRUCTDICTIONARY: ConstructDictionaryGDScriptOp.extract,
    OPCODE_CALL: CallGDScriptOp.extract,
    OPCODE_CALLRETURN: CallReturnGDScriptOp.extract,
    OPCODE_CALLBUILTIN: CallBuiltinGDScriptOp.extract,
    OPCODE_CALLSELFBASE: CallSelfBaseGDScriptOp.extract,
	OPCODE_STACK: None,
	OPCODE_STACK_RETURN: None,
    OPCODE_YIELD: None,
    OPCODE_YIELDSIGNAL: None,
    OPCODE_YIELDRESUME: None,
    OPCODE_JUMP: JumpGDScriptOp.extract,
    OPCODE_JUMPIF: JumpIfGDScriptOp.extract,
    OPCODE_JUMPIFNOT: JumpIfNotGDScriptOp.extract,
    OPCODE_JUMPTODEFAULTARGUMENT: JumpToDefaultArgumentGDScriptOp.extract,
    OPCODE_RETURN: ReturnGDScriptOp.extract,
    OPCODE_ITERATEBEGIN: IterateBeginGDScriptOp.extract,
    OPCODE_ITERATE: IterateGDScriptOp.extract,
    OPCODE_ASSERT: None,
    OPCODE_BREAKPOINT: None,
    OPCODE_LINE: LineGDScriptOp.extract,
    OPCODE_END: EndGDScriptOp.extract,

    # The following are not defined by Godot but are special instructions
    # for the transpiler
    OPCODE_NOOP: None,
    OPCODE_BOX: None,
    OPCODE_DESTROY: None,
    OPCODE_UNBOX: None,
    OPCODE_INITIALIZE: None,
    OPCODE_PARAMETER: None,
    OPCODE_PHI: None,
    OPCODE_PARAMETER_COPY: None
}

def extract(func: GDScriptFunction, bytecode: List[int], index: int) -> GDScriptOp:
    fn = _extractors[bytecode[index]]
    if fn:
        return fn(func, bytecode, index)

    raise Exception(f"Bytecode {bytecode[index]} is not supported at bytecode[{index}]")


