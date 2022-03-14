from __future__ import annotations
from typing import TYPE_CHECKING, Union, Optional, List, Dict, Set, Iterable, Tuple
from gd2c.variant import VariantType
from gd2c.bytecode import GDScriptOp
from gd2c.address import GDScriptAddress, ADDRESS_MODE_MEMBER, ADDRESS_MODE_LOCALCONSTANT, ADDRESS_MODE_CLASS, ADDRESS_MODE_STACKVARIABLE, ADDRESS_MODE_GLOBAL, ADDRESS_MODE_CLASSCONSTANT
import re

if TYPE_CHECKING:
    from controlflow import ControlFlowGraph

class GDScriptGlobal:
    SOURCE_CONSTANT = 1
    SOURCE_HARDCODED = 2
    SOURCE_CLASSDB = 3
    SOURCE_SINGLETON = 4

    def __init__(self, index: int, name: str, original_name: str, vtype: int, kind_code: int, value: str, source: Union[str, int]):
        self.index = index
        self.original_name = original_name
        self.vtype = VariantType.get(vtype)
        self.kind_code = kind_code
        self.value = value
        self.address = GDScriptAddress.create(ADDRESS_MODE_GLOBAL, index)
        if isinstance(source, str):
            if source == "GlobalConstants":
                self.source = GDScriptGlobal.SOURCE_CONSTANT
            elif source == "hard-coded":
                self.source = GDScriptGlobal.SOURCE_HARDCODED
            elif source == "ClassDB":
                self.source = GDScriptGlobal.SOURCE_CLASSDB
            elif source == "Singleton":
                self.source = GDScriptGlobal.SOURCE_SINGLETON
        elif isinstance(source, int):
            assert source in ( \
                GDScriptGlobal.SOURCE_CONSTANT, \
                GDScriptGlobal.SOURCE_HARDCODED, \
                GDScriptGlobal.SOURCE_CLASSDB, \
                GDScriptGlobal.SOURCE_SINGLETON)
            self.source = source

class GDScriptFunctionConstant:
    def __init__(self, index: int, vtype: Union[VariantType, str, int], data: bytes, declaration: str):
        self.index = index
        self.vtype = VariantType.get(vtype)
        self.data = data
        self.declaration = declaration
        self.address = GDScriptAddress.create(ADDRESS_MODE_LOCALCONSTANT, index)

class GDScriptClassConstant:
    def __init__(self, name: str, vtype: Union[VariantType, str, int], data: bytes, declaration: str):
        self.name = name
        self.vtype = VariantType.get(vtype)
        self.data = data
        self.declaration = declaration
        #self.address = GDScriptAddress.create(ADDRESS_MODE_CLASSCONSTANT, )

class GDScriptMember:
    def __init__(self, name: str, index: int, vtype: Union[VariantType, str, int]):
        self.is_inherited = False
        self.name = name
        self.index = index
        self.vtype = VariantType.get(vtype)
        self.address = GDScriptAddress.create(ADDRESS_MODE_MEMBER, index)

class GDScriptFunctionParameter:
    name: str
    vtype: VariantType
    index: int
    is_assigned: Union[bool, None]
    address: GDScriptAddress

    def __init__(self, name: str, vtype: Union[VariantType, int], index: int):
        self.name = name
        self.vtype = VariantType.get(vtype)
        self.index = index
        self.is_assigned = False
        self.address = GDScriptAddress.create(ADDRESS_MODE_STACKVARIABLE, index)
        self.is_optional = False

class GDScriptFunction:
    name: str
    function_type: int
    default_arguments_jump_table: List[int]
    stack_size: int
    return_vtype: VariantType
    global_names: List[str]
    cfg: Optional[ControlFlowGraph]
    _parameters: Dict[int, GDScriptFunctionParameter]
    _constants: Dict[int, GDScriptFunctionConstant]
    _ops: List[Tuple[int, GDScriptOp]]
    _mutates: Set

    # Annotations
    yields: bool

    TYPE_METHOD = 0

    def __init__(self, name: str, function_type: int):
        self.name = name
        self.function_type = function_type
        self.default_arguments_jump_table = []
        self.stack_size = 0
        self.return_vtype = VariantType.NIL
        self.global_names = []
        self.cfg = None
        self._parameters = {}
        self._constants = {}
        self._ops = []
        self._mutates= set([])
        self.yields = False

    def clear_op_reached_flags(self):
        for _, op in self._ops:
            op.reached = False

    def num_optional_parameters(self):
        return len(default_arguments_jump_table)

    @property
    def has_constants(self) -> bool:
        return len(self.global_names) > 0 or len(self._constants) > 0

    @property
    def len_stack_array(self) -> int:
        return self.stack_size - len(self._parameters)

    def parameters(self) -> Iterable[GDScriptFunctionParameter]:
        for param in self._parameters.values():
            yield param

    def parameter(self, what: Union[int, str]):
        if isinstance(what, int):
            return self._parameters[what]

        if isinstance(what, str):
            for parameter in self._parameters.values():
                if parameter.name == what:
                    return parameter

        raise Exception("parameter not found")

    @property
    def len_parameters(self) -> int:
        return len(self._parameters)

    def add_parameter(self, param: GDScriptFunctionParameter):
        self._parameters[param.index] = param

    def constants(self) -> Iterable[GDScriptFunctionConstant]:
        for const in self._constants.values():
            yield const

    @property
    def len_constants(self):
        return len(self._constants)

    def add_constant(self, const: GDScriptFunctionConstant):
        self._constants[const.index] = const

    def new_stack_variable(self, vtype: VariantType) -> GDScriptAddress:
        addr = GDScriptAddress.create(ADDRESS_MODE_STACKVARIABLE, self.stack_size)
        self.stack_size += 1
        return addr

    def ops(self) -> Iterable[Tuple[int, GDScriptOp]]:
        for tup in self._ops:
            yield tup

    def add_op(self, addr: int, op: GDScriptOp):
        self._ops.append((addr, op))

    def clear_ops(self):
        self._ops = []

    def pretty_print(self, print_bytecode=True):
        print(f"Function: {self.name}")
        print(f"  Return type: {self.return_vtype}")
        print(f"  Stack size: {self.stack_size}")
        print(f"  Parameters:")
        if any(self.parameters()):
            for p in sorted(self.parameters(), key=lambda v: v.index):
                is_assigned = 'Not determined' if p.is_assigned is None else p.is_assigned
                print(f"    {str(p.index).ljust(3)}: {p.name}: {p.vtype}")
                print(f"        is_assigned: {is_assigned}")
        else:
            print(f"    NONE")

        print(f"  Default arguments jump table:")
        if any(self.default_arguments_jump_table):
            for d in self.default_arguments_jump_table:
                print(f"    {d}")
        else:
            print(f"    NONE")

        print(f"  Constants:")
        if any(self.constants()):
            for const in sorted(self.constants(), key=lambda v: v.index):
                print(f"    {str(GDScriptAddress.calc_address(ADDRESS_MODE_LOCALCONSTANT, const.index)).ljust(3)}: {const.declaration}")
        else:
            print(f"    NONE")

        print(f"  Global names:")
        if any(self.global_names):
            for name in self.global_names:
                print(f"    {str(name).ljust(3)}: {name}")
        else:
            print(f"    NONE")

        if print_bytecode:
            print(f"  Ops:")
            for ip, op in self.ops():
                print(f"{str(ip).rjust(8)}: {str(op)}")

        print(f"")

class GDScriptClass:
    _built_in_type: Optional[str]
    _resource_path: str
    _base_resource_path: Optional[str]
    _base: Optional[GDScriptClass]
    _type_id: int
    _name: str
    _constants: Dict[str, GDScriptClassConstant]
    _members: Dict[str, GDScriptMember]
    _signals: Set[str]
    _functions: Dict[str, GDScriptFunction]
    globals: Dict[int, GDScriptGlobal]
    _ctor: Optional[GDScriptFunction]
    _dtor: Optional[GDScriptFunction]

    def __init__(self, resource_path: str, name: str, type_id: int):
        self._built_in_type = None
        self._resource_path = re.sub(r'\.gd\.json', '', resource_path, flags=re.IGNORECASE)
        self._base_resource_path = None
        self._base = None
        self._type_id = type_id
        self._name = name
        self._constants = {}
        self._members = {}
        self._signals = set()
        self._functions = {}
        self.globals = {}
        self._ctor = None
        self._dtor = None

    @property
    def resource_path(self) -> str:
        return self._resource_path

    @property
    def type_id(self) -> int:
        return self._type_id

    @property
    def name(self) -> str:
        return self._name

    @property
    def built_in_type(self):
        return self._built_in_type
    @built_in_type.setter
    def built_in_type(self, value: str):
        self._built_in_type = value

    @property
    def base_resource_path(self) -> Optional[str]:
        return self._base_resource_path
    @base_resource_path.setter
    def base_resource_path(self, value: str):
        self._base_resource_path = value

    @property
    def base(self) -> Optional[GDScriptClass]:
        return self._base
    @base.setter
    def base(self, value: Union[GDScriptClass, None]):
        self._base = value
        if value:
            self._base_resource_path = value.resource_path
        else:
            self._base_resource_path = None

    def add_function(self, func: GDScriptFunction):
        self._functions[func.name] = func

    def function(self, name: str) -> Optional[GDScriptFunction]:
        return self._functions.get(name, None)

    def functions(self) -> Iterable[GDScriptFunction]:
        for func in self._functions.values():
            yield func

    @property
    def len_functions(self) -> int:
        return len(self._functions)

    def add_constant(self, const: GDScriptClassConstant):
        self._constants[const.name] = const

    def constant(self, name: str) -> GDScriptClassConstant:
        return self._constants[name]

    def constants(self) -> Iterable[GDScriptClassConstant]:
        for const in self._constants.values():
            yield const

    @property
    def has_constants(self) -> bool:
        return len(self._constants) > 0

    @property
    def len_constants(self) -> int:
        return len(self._constants)

    def add_member(self, member: GDScriptMember):
        self._members[member.name] = member

    def member(self, name: str) -> GDScriptMember:
        return self._members[name]

    def members(self) -> Iterable[GDScriptMember]:
        for member in self._members.values():
            yield member

    def has_member(self, what: str) -> bool:
        if isinstance(what, str):
            return what in self._members

        raise Exception("what must be str")

    @property
    def len_members(self) -> int:
        return len(self._members)

    def add_signal(self, signal: str):
        self._signals.add(signal)

    @property
    def len_signal(self) -> int:
        return len(self._signals)

    def signals(self) -> Iterable[str]:
        for signal in self._signals:
            yield signal

    def own_members(self) -> Iterable[GDScriptMember]:
        result: List[GDScriptMember] = []
        for member in self.members():
            base = self.base
            inherited = False
            while base and not inherited:
                for base_member in base.members():
                    if base_member.name == member.name:
                        inherited = True
                        break
                base = base.base
            if not inherited:
                result.append(member)

        return sorted(result, key=lambda it: it.index)


