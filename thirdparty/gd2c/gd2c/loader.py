from __future__ import annotations
from pathlib import Path
from gd2c.gdscriptclass import GDScriptClass, GDScriptClassConstant, GDScriptFunctionConstant, GDScriptFunction, GDScriptGlobal, GDScriptMember, GDScriptFunctionParameter
from gd2c.variant import VariantType
from gd2c.bytecode import extract
from typing import List, Iterable, TYPE_CHECKING
import json

def to_camel_case(snake_str: str, capitalize_first: bool = True):
    components = snake_str.split('_')
    # We capitalize the first letter of each component
    if capitalize_first:
        return ''.join(x.title() for x in components[0:])
    else:
        return components[0] + ''.join(x.title() for x in components[1:])

if TYPE_CHECKING:
    from gd2c.project import Project

class JsonGDScriptLoader:
    def __init__(self, project: Project):
        self._project = project

    def load_classes(self, physical_path: Path) -> Iterable[GDScriptClass]:
        with physical_path.open() as f:
            data = json.load(f)
            yield self._build_class(physical_path, data)

    def _build_class(self, physical_path: Path, data) -> GDScriptClass:
        cls = GDScriptClass(
            self._project.to_resource_path(str(physical_path)),
            data.get("name", None) or self._project.generate_unique_class_name(to_camel_case(physical_path.with_suffix('').stem)),
            self._project.generate_unique_class_type_id())
        cls.base_resource_path = data["base_type"]
        cls.built_in_type = data["type"]

        for index, entry in enumerate(data["global_constants"]):
            glob = GDScriptGlobal(index, entry["name"], entry["original_name"], entry["type_code"], entry["kind_code"], entry["value"], entry["source"])
            cls.globals[glob.index] = glob

        for signal in data["signals"]:
            cls.add_signal(signal)

        for entry in data["members"]:
            member = GDScriptMember(entry["name"], int(entry["index"]), entry["type"])
            cls.add_member(member)

        for index, entry in enumerate(data["constants"]):
            cconst = GDScriptClassConstant(entry["name"], int(entry["type"]), bytes(list(entry["data"])), entry["declaration"])
            cls.add_constant(cconst)

        for index, entry in enumerate(data["methods"]):
            func = GDScriptFunction(entry["name"], GDScriptFunction.TYPE_METHOD)
            func.stack_size = int(entry["stack_size"])
            func.default_arguments_jump_table = list(map(lambda x: int(x), entry["default_arguments"]))
            func.return_vtype = VariantType.get(int(entry["return_type"]["type"]))
            func.global_names = entry["global_names"]

            num_parameters = len(entry["parameters"])
            len_jump_table = len(func.default_arguments_jump_table)
            for pindex, pentry in enumerate(entry["parameters"]):
                param = GDScriptFunctionParameter(
                    pentry["name"],
                    VariantType.get(pentry["type"]),
                    pindex)
                param.is_optional = pindex >= num_parameters - len_jump_table
                func.add_parameter(param)

            for centry in entry["constants"]:
                mconst = GDScriptFunctionConstant(
                    int(centry["index"]),
                    centry["type"],
                    bytes(list(map(lambda x: int(x), centry["data"]))),
                    centry["declaration"])
                func.add_constant(mconst)

            ip = 0
            while ip < len(entry["bytecode"]):
                op = extract(func, entry["bytecode"], ip)
                func.add_op(ip, op)
                ip += op.stride

            cls.add_function(func)

        return cls
