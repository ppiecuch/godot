from __future__ import annotations
from typing import Iterable, Dict, Union, Optional, Callable, List
from gd2c.gdscriptclass import GDScriptClass
from gd2c.loader import JsonGDScriptLoader
from pathlib import Path, PurePath, PurePosixPath
from os import PathLike

class Project:
    __next_type_id = 0

    def __init__(self, root_path: Union[PurePath, PathLike, str]):
        self._root = PurePath(root_path)
        self._classes_by_resource_path: Dict[str, GDScriptClass] = {}
        self._classes_by_name: Dict[str, GDScriptClass] = {}
        self._classes_by_type_id: Dict[int, GDScriptClass] = {}
        self._export_prefix = "godot"

    @property
    def export_prefix(self) -> str:
        return self._export_prefix

    @property
    def root(self) -> PurePath:
        return self._root

    def classes(self) -> Iterable[GDScriptClass]:
        """All GDScript classes in no particular order.
        """
        for item in self._classes_by_type_id.values():
            yield item

    def derived_classes(self, what: Union[GDScriptClass, str, int]):
        """Returns all GDScript classes that directly derive from the specified class.

        Args:
          what: Either a GDScriptClass, resource path, class name, or class type_id indicating the base class. Can be None.
        """
        base_cls: Optional[GDScriptClass] = None
        if isinstance(what, GDScriptClass):
            base_cls = what
        else:
            base_cls = self.get_class(what)

        for cls in self._classes_by_type_id.values():
            if cls.base == base_cls:
                yield cls

    def to_rooted_path(self, filepath: Union[Path, PathLike, str]) -> PurePath:
        """Returns absolute file system path of a file within the rootPath of the project.
        """
        fp = Path(filepath)
        if not fp.is_absolute():
            fp = Path(self._root, filepath)

        return PurePath(fp.absolute())

    def to_resource_path(self, filepath: Union[Path, PathLike, str]) -> str:
        """Returns resource path (res://path/to/file) of a file underneath a project root path
        """
        absolute = Path(filepath).absolute()
        root = Path(self._root).absolute()
        rel = absolute.relative_to(root)
        as_str = str(PurePosixPath(rel)).replace('\\', '/').replace(".gd.json", ".gd")
        return f'res://{as_str}'

    def to_file_path(self, resourcePath: str) -> PurePath:
        """Translates the resource path to the physical file path in the file system.
        """
        rel = resourcePath.replace('res://', '')
        return self._root.joinpath(rel)

    def get_class(self, key: Union[int, str]) -> Union[GDScriptClass, None]:
        """Returns a class that was loaded by the project.

        Args:
        key (int, str): Either the generated class type-id, class name, or \
            resource_path of the class.
        """
        if (isinstance(key, str)):
            if (key.startswith("res://")):
                return self._classes_by_resource_path[key]
            else:
                return self._classes_by_name[key]
        elif isinstance(key, int):
            return self._classes_by_type_id[key]

        raise Exception("Key must be str or int")

    def add_class(self, cls: GDScriptClass) -> None:
        if cls.name in self._classes_by_name:
            raise Exception(f"Class with name '{cls.name}' already registered.")
        if cls.resource_path in self._classes_by_resource_path:
            raise Exception(f"Class with resource path `{cls.name}' already registered.")
        if cls.type_id in self._classes_by_type_id:
            raise Exception(f"Class with type_id {cls.type_id} already registered.")

        self._classes_by_name[cls.name] = cls
        self._classes_by_resource_path[cls.resource_path] = cls
        self._classes_by_type_id[cls.type_id] = cls

    def remove_class(self, key: Union[int, str]):
        cls: Optional[GDScriptClass] = None
        if (isinstance(key, str)):
            if (key.startswith("res://")):
                cls = self._classes_by_resource_path.get(key, None)
            else:
                cls = self._classes_by_name.get(key, None)
        elif isinstance(key, int):
            cls = self._classes_by_type_id.get(key, None)
        else:
            raise Exception("Key must be str or int")

        if cls:
            del self._classes_by_resource_path[cls.resource_path]
            del self._classes_by_name[cls.name]
            del self._classes_by_type_id[cls.type_id]

    def generate_unique_class_name(self, base_name: str):
        """Generates a type id that is guaranteed to not have been generated for this project.
        """
        if Project.__next_type_id > 2_000:
            raise Exception("Too many classes")
        Project.__next_type_id += 1
        """ TODO: Donot append type_id if name is already unique
        """
        if base_name:
            return f'Class{base_name}{Project.__next_type_id}'
        else:
            return f'Class_{Project.__next_type_id}'

    def generate_unique_class_type_id(self):
        """Generates a class name that is guaranteed to not have been generated for this project.
        Class names are not related to generated type_ids. "Class1" does not necessarily correspond
        to GDScriptClass with type_id == 1
        """
        if Project.__next_type_id > 2_000_000_000:
            raise Exception("Too many classes")

        Project.__next_type_id += 1
        return Project.__next_type_id

    def visit_classes_in_dependency_order(self, visitor: Callable[[GDScriptClass, int], None]):
        def iterate(cls, depth):
            visitor(cls, depth)
            for dep in [c for c in self._classes_by_type_id.values() if c.base is cls]:
                iterate(dep, depth + 1)

        [iterate(c, 0) for c in self._classes_by_type_id.values() if not c.base_resource_path]

    def iter_classes_in_dependency_order(self) -> Iterable[GDScriptClass]:
        classes: List[GDScriptClass] = []

        def iterate(cls, depth):
            nonlocal classes
            classes.append(cls)
            for dep in [c for c in self._classes_by_type_id.values() if c.base is cls]:
                iterate(dep, depth + 1)

        [iterate(c, 0) for c in self._classes_by_type_id.values() if not c.base_resource_path]
        return classes

def load_project(root_path: str) -> Project:
    project = Project(root_path)

    loader = JsonGDScriptLoader(project)
    stack = [Path(project.root)]
    count = 0

    # load classes
    while any(stack):
        path = stack.pop()
        for it in path.iterdir():
            if it.is_dir():
                stack.append(it)
            elif it.is_file():
                if str(it).endswith(".gd.json"):
                    classes = loader.load_classes(it)
                    for cls in classes:
                        project.add_class(cls)
                        count += 1

    # link classes
    for cls in project._classes_by_type_id.values():
        if cls.base_resource_path:
            cls.base = project.get_class(cls.base_resource_path)

    def set_inherited_flag(cls: GDScriptClass, depth: int):
        if cls.base:
            for member in cls.members():
                member.is_inherited = cls.base.has_member(member.name)

    project.visit_classes_in_dependency_order(set_inherited_flag)

    return project
