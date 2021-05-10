from __future__ import annotations
from typing import Union, List, Set, FrozenSet, Optional, Dict, IO, TYPE_CHECKING
from gd2c.gdscriptclass import GDScriptClassConstant, GDScriptClass, GDScriptFunction, GDScriptMember, GDScriptGlobal
from gd2c.address import *
from gd2c.variant import VariantType
import itertools

class Variable:
    func_context: 'FunctionContext'
    address: GDScriptAddress
    var_type: int
    cls: GDScriptClass
    _identifier: str

    def __init__(self, func_context: 'FunctionContext', address: int):
        self.func_context = func_context
        self.address = GDScriptAddress(address)

        self._identifier = ""

        if self.address.mode in (ADDRESS_MODE_STACKVARIABLE, ADDRESS_MODE_STACK):
            if self.address.offset >= func_context.func.len_parameters:
                self.identifier = f"st_{self.address.offset}"

    def define(self) -> str:
        assert self.identifier
        return f"godot_variant {self.identifier};\n"

    def needs_definition(self) -> bool:
        return self.address.offset >= self.func_context.func.len_parameters \
            and self.address.mode in (ADDRESS_MODE_STACKVARIABLE, ADDRESS_MODE_STACK)

    def value(self) -> str:
        if self.address.mode == ADDRESS_MODE_SELF:
            return f"p_user_data->__self"
        elif self.address.mode == ADDRESS_MODE_CLASS:
            pass
        elif self.address.mode == ADDRESS_MODE_MEMBER:
            return f"p_user_data->{self.func_context.class_context.get_member_context(self.address.offset).member_identifier}"
        elif self.address.mode == ADDRESS_MODE_CLASSCONSTANT:
            name = self.func_context.func.global_names[self.address.offset]
            return self.func_context.class_context.value_of_constant(name)
        elif self.address.mode == ADDRESS_MODE_LOCALCONSTANT or self.address.mode == ADDRESS_MODE_STACK:
            if self.address.offset < self.func_context.func.len_parameters:
                return f"*p_args[{self.address.offset}]"
            else:
                return self.identifier
        elif self.address.mode == ADDRESS_MODE_STACK:
            return self.identifier
        elif self.address.mode == ADDRESS_MODE_GLOBAL:
            self.func_context.class_context.global_context.value_expression(self.address.offset)
        elif self.address.mode == ADDRESS_MODE_NAMEDGLOBAL:
            pass
        elif self.address.mode == ADDRESS_MODE_NIL:
            return "__nil"
        elif self.address.mode == ADDRESS_MODE_PARAMETER:
            pass
        elif self.address.mode == ADDRESS_MODE_TEMPORARY:
            pass
        elif self.address.mode == ADDRESS_MODE_SETTERVALUEPARAMETER:
            pass

        return f"{AddressModePrefix[self.address.mode]}_{self.address.address}"

    def address_of(self) -> str:
        if self.address.mode == ADDRESS_MODE_SELF:
            return f"&p_user_data->__self"
        elif self.address.mode == ADDRESS_MODE_CLASS:
            pass
        elif self.address.mode == ADDRESS_MODE_MEMBER:
            return f"&p_user_data->{self.func_context.class_context.get_member_context(self.address.offset).member_identifier}"
        elif self.address.mode == ADDRESS_MODE_CLASSCONSTANT:
            name = self.func_context.func.global_names[self.address.offset]
            return self.func_context.class_context.address_of_constant(name)
        elif self.address.mode == ADDRESS_MODE_LOCALCONSTANT:
            return f"&{self.func_context.local_constants_array_identifier}[{self.address.offset}]"
        elif self.address.mode == ADDRESS_MODE_STACKVARIABLE or self.address.mode == ADDRESS_MODE_STACK:
            if self.address.offset < self.func_context.func.len_parameters:
                return f"p_args[{self.address.offset}]"
            else:
                return f"&{self.identifier}"
        elif self.address.mode == ADDRESS_MODE_GLOBAL:
            return self.func_context.class_context.global_context.address_of_expression(self.address.offset)
        elif self.address.mode == ADDRESS_MODE_NAMEDGLOBAL:
            pass
        elif self.address.mode == ADDRESS_MODE_NIL:
            return "&__nil"
        elif self.address.mode == ADDRESS_MODE_PARAMETER:
            pass
        elif self.address.mode == ADDRESS_MODE_TEMPORARY:
            pass
        elif self.address.mode == ADDRESS_MODE_SETTERVALUEPARAMETER:
            pass

        return f"&{AddressModePrefix[self.address.mode]}_{self.address.address}"

class FunctionContext:
    func: GDScriptFunction
    class_context: ClassContext
    variables: Dict[int, Variable]
    local_constants_array_identifier: str
    global_names_identifier: str
    global_strings_identifier: str

    def __init__(self, func: GDScriptFunction, class_context: ClassContext):
        assert func
        assert func.cfg
        assert class_context

        self.func = func
        self.class_context = class_context
        self.function_identifier =  f"{self.class_context.cls.name}_{self.func.name}_func"
        self.local_constants_array_identifier = f"{self.class_context.cls.name}_{self.func.name}_local_const"
        self.initialized_local_constants_array_identifier = f"{self.class_context.cls.name}_{self.func.name}_local_const_initialized"
        self.global_names_identifier = f"{self.class_context.cls.name}_{self.func.name}_string_names"
        self.global_strings_identifier = f"{self.class_context.cls.name}_{self.func.name}_strings"
        self.init_variables()

    def init_variables(self) -> None:
        assert self.func
        assert self.func.cfg

        self.func.cfg.live_variable_analysis()

        all_addr: Set[int] = set()

        for block in self.func.cfg.nodes():
            all_addr.update(block.defs)
            all_addr.update(block.uses)

        variables: Dict[int, Variable] = {}
        for addr in all_addr:
            variables[addr] = Variable(self, addr)

        self.variables = variables

class VtableEntry:
    func_context: FunctionContext
    index: int

    def __init__(self, index: int, func: FunctionContext):
        self.index = index
        self.func_context = func

class MemberContext:
    member: GDScriptMember
    member_identifier: str
    path: str
    setter_identifier: str
    getter_identifier: str

    def __init__(self, cls: GDScriptClass, member: GDScriptMember):
        self.member_identifier = f"{member.name}"
        self.member = member
        self.path = f"{member.name}"
        self.setter_identifier = f"{cls.name}_set_{member.name}"
        self.getter_identifier = f"{cls.name}_get_{member.name}"

class ClassConstantContext:
    defining_class_context: ClassContext
    constant: GDScriptClassConstant
    index: int

    def __init__(self, defining_class_context: ClassContext, constant: GDScriptClassConstant, index: int):
        self.defining_class_context = defining_class_context
        self.constant = constant
        self.index = index

class ClassContext:
    cls: GDScriptClass
    global_context: GlobalContext
    base_context: Optional[ClassContext]
    function_contexts: Dict[str, FunctionContext]
    member_contexts: Dict[str, MemberContext]
    constant_contexts: Dict[str, ClassConstantContext]
    vtable_entries: List[VtableEntry]
    struct_tag: str
    vtable_wrappers_identifier: str
    vtable_methods_identifier: str
    vtable_method_names_identifier: str
    vable_identifier: str
    base_vtable_identifier: str
    vtable_init_function_identifier: str
    constants_initialized_identifier: str
    constants_array_identifier: str
    ctor_identifier: str
    dtor_identifier: str

    def __init__(self, cls: GDScriptClass, global_context: GlobalContext, base_context: Optional[ClassContext]):
        self.cls = cls
        self.global_context = global_context
        self.base_context = base_context
        self.function_contexts = {}
        self.member_contexts = {}
        self.constant_contexts = {}
        self.vtable_entries = []
        self.struct_tag = f"{cls.name}_struct_t"
        self.vtable_wrappers_identifier = f"{cls.name}_vtable_wrappers"
        self.vtable_methods_identifier = f"{cls.name}_vtable_methods"
        self.vtable_method_names_identifier = f"{cls.name}_vtable_method_names"
        self.vtable_identifier = f"{cls.name}_vtable"
        self.base_vtable_identifier = "vtable"
        self.vtable_init_function_identifier = f"{cls.name}_vtable_init"
        self.constants_initialized_identifier = f"{cls.name}_constants_initialized"
        self.constants_array_identifier = f"{cls.name}_constants"
        self.ctor_identifier = f"{cls.name}_ctor"
        self.dtor_identifier = f"{cls.name}_dtor"

        self.initialize_class_constants()
        self.initialize_members()
        self.initialize_vtable()

    def initialize_members(self):
        own_members = self.cls.own_members()
        own_member_names = [m.name for m in own_members]
        for member in self.cls.members():
            if member.name in own_member_names:
                self.member_contexts[member.name] = MemberContext(self.cls, member)
            elif self.base_context:
                self.member_contexts[member.name] = self.base_context.member_contexts[member.name]
            else:
                raise Exception("internal error: base_context is None")

    def initialize_class_constants(self):
        for index, constant in enumerate(self.cls.constants()):
            self.constant_contexts[constant.name] = ClassConstantContext(self, constant, index)

    def initialize_vtable(self):
        self.function_contexts = dict([(f.name, FunctionContext(f, self)) for f in self.cls.functions()])

        self.vtable_entries = []
        new_funcs: List[FunctionContext] = []
        if self.cls.base:
            assert self.base_context
            self.base_vtable_identifier = self.base_context.vtable_identifier
            self.vtable_entries.extend(self.base_context.vtable_entries)
            for func_context in self.function_contexts.values():
                found = False
                for i, entry in enumerate(self.vtable_entries):
                    if entry.func_context.func.name == func_context.func.name:
                        self.vtable_entries[i] = VtableEntry(entry.index, func_context)
                        found = True
                        break

                if not found:
                    new_funcs.append(func_context)
        else:
            new_funcs.extend(self.function_contexts.values())

        for func_context in new_funcs:
            self.vtable_entries.append(VtableEntry(len(self.vtable_entries), func_context))

    def get_member_vtable_entry(self, method_name: str) -> Union[VtableEntry, None]:
        for entry in self.vtable_entries:
            if entry.func_context.func.name == method_name:
                return entry

        return None

    def get_function_context(self, what: Union[str, GDScriptFunction]) -> FunctionContext:
        if isinstance(what, str):
            return self.function_contexts[what]

        if isinstance(what, GDScriptFunction):
            for func_context in self.function_contexts.values():
                if func_context.func == what:
                    return func_context

        raise Exception("function not found")

    def get_member_context(self, what: Union[str, int]) -> MemberContext:
        if isinstance(what, str):
            return self.member_contexts[what]

        if isinstance(what, int):
            for member_context in self.member_contexts.values():
                if member_context.member.index == what:
                    return member_context

        raise Exception("member not found")

    def get_constant_context(self, name: str) -> ClassConstantContext:
        cc: Union[ClassContext, None] = self
        while not cc is None:
            if name in cc.constant_contexts:
                return cc.constant_contexts[name]
            else:
                cc = cc.base_context

        raise Exception("constant not found")

    def address_of_constant(self, constant_name: str) -> str:
        cc = self.get_constant_context(constant_name)
        return f"&{cc.defining_class_context.constants_array_identifier}[{cc.index}]"

    def value_of_constant(self, constant_name: str) -> str:
        cc = self.get_constant_context(constant_name)
        return f"{cc.defining_class_context.constants_array_identifier}[{cc.index}]"

class GlobalContext:
    constants_array_identifier: str
    hard_coded_constants_array_identifier: str
    classdb_array_identifier: str
    singletons_array_identifier: str
    num_constants: int
    num_hard_coded_constants: int
    num_classdb: int
    num_singleton: int
    num_globals: int

    globals: Dict[int, GDScriptGlobal]
    index_map: Dict[int, int]

    def __init__(self):
        self.globals = {}
        self.index_map = {}
        self.constants_array_identifier = "global_constants_array"
        self.hard_coded_constants_array_identifier = "global_hard_coded_constants_array"
        self.classdb_array_identifier = "global_classdb_array"
        self.singletons_array_identifier = "global_singletons_array"

    def initialize_globals(self, globals: Dict[int, GDScriptGlobal]) -> None:
        self.globals = globals

        count = len(globals)
        self.num_constants = 0
        self.num_hard_coded_constants = 0
        self.num_classdb = 0
        self.num_singletons = 0

        for g in globals.values():
            if g.source == GDScriptGlobal.SOURCE_CONSTANT:
                self.index_map[g.index] = self.num_constants
                self.num_constants += 1
            elif g.source == GDScriptGlobal.SOURCE_HARDCODED:
                self.index_map[g.index] = self.num_hard_coded_constants
                self.num_hard_coded_constants += 1
            elif g.source == GDScriptGlobal.SOURCE_CLASSDB:
                self.index_map[g.index] = self.num_classdb
                self.num_classdb += 1
            elif g.source == GDScriptGlobal.SOURCE_SINGLETON:
                self.index_map[g.index] = self.num_singletons
                self.num_singletons += 1

    def address_of_expression(self, index: int) -> str:
        glob = self.globals[index]
        if glob.source == GDScriptGlobal.SOURCE_CONSTANT:
            return f"&{self.constants_array_identifier}[{self.index_map[glob.index]}]"
        elif glob.source == GDScriptGlobal.SOURCE_HARDCODED:
            return f"&{self.hard_coded_constants_array_identifier}[{self.index_map[glob.index]}]"
        elif glob.source == GDScriptGlobal.SOURCE_CLASSDB:
            return f"&{self.classdb_array_identifier}[{self.index_map[glob.index]}]"
        elif glob.source == GDScriptGlobal.SOURCE_SINGLETON:
            return f"&{self.singletons_array_identifier}[{self.index_map[glob.index]}]"

        return f"&glob_{index}"

    def value_expression(self, index: int) -> str:
        glob = self.globals[index]
        if glob.source == GDScriptGlobal.SOURCE_CONSTANT:
            return f"{self.constants_array_identifier}[{self.index_map[glob.index]}]"
        elif glob.source == GDScriptGlobal.SOURCE_HARDCODED:
            return f"{self.hard_coded_constants_array_identifier}[{self.index_map[glob.index]}]"
        elif glob.source == GDScriptGlobal.SOURCE_CLASSDB:
            return f"{self.classdb_array_identifier}[{self.index_map[glob.index]}]"
        elif glob.source == GDScriptGlobal.SOURCE_SINGLETON:
            return f"{self.singletons_array_identifier}[{self.index_map[glob.index]}]"

        return f"&glob_{index}"

    def define(self) -> str:
        return f"""\
            godot_variant {self.constants_array_identifier}[{self.num_constants}];
            godot_variant {self.classdb_array_identifier}[{self.num_classdb}];
            godot_variant {self.hard_coded_constants_array_identifier}[{self.num_hard_coded_constants}];
            godot_variant {self.singletons_array_identifier}[{self.num_singletons}];
        """
