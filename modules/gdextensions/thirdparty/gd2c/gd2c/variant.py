from __future__ import annotations
from typing import Union, Dict, cast

class VariantType:
    NIL: 'VariantType' = cast('VariantType', None)
    BOOL: 'VariantType' = cast('VariantType', None)
    INT: 'VariantType' = cast('VariantType', None)
    REAL: 'VariantType' = cast('VariantType', None)
    STRING: 'VariantType' = cast('VariantType', None)
    VECTOR2: 'VariantType' = cast('VariantType', None)
    RECT2: 'VariantType' = cast('VariantType', None)
    VECTOR3: 'VariantType' = cast('VariantType', None)
    TRANSFORM2D: 'VariantType' = cast('VariantType', None)
    PLANE: 'VariantType' = cast('VariantType', None)
    QUAT: 'VariantType' = cast('VariantType', None)
    AABB: 'VariantType' = cast('VariantType', None)
    BASIS: 'VariantType' = cast('VariantType', None)
    TRANSFORM: 'VariantType' = cast('VariantType', None)
    COLOR: 'VariantType' = cast('VariantType', None)
    NODE_PATH: 'VariantType' = cast('VariantType', None)
    RID: 'VariantType' = cast('VariantType', None)
    OBJECT: 'VariantType' = cast('VariantType', None)
    DICTIONARY: 'VariantType' = cast('VariantType', None)
    ARRAY: 'VariantType' = cast('VariantType', None)
    POOL_BYTE_ARRAY: 'VariantType' = cast('VariantType', None)
    POOL_INT_ARRAY: 'VariantType' = cast('VariantType', None)
    POOL_REAL_ARRAY: 'VariantType' = cast('VariantType', None)
    POOL_STRING_ARRAY: 'VariantType' = cast('VariantType', None)
    POOL_VECTOR2_ARRAY: 'VariantType' = cast('VariantType', None)
    POOL_VECTOR3_ARRAY: 'VariantType' = cast('VariantType', None)
    POOL_COLOR_ARRAY: 'VariantType' = cast('VariantType', None)
    VARIANT_MAX = 27

    @staticmethod
    def get(value: Union[VariantType, str, int]) -> VariantType:
        if isinstance(value, VariantType):
            return value
        elif isinstance(value, int):
            return _vtypes[value]
        elif isinstance(value, str):
            return _vtypes[int(value)]
        elif value is None:
            return VariantType.NIL

        raise "Value must be int, str, or VariantType"

    def __init__(self, value: int, name: str):
        self._value = value
        self._name = name

        _vtypes[self.value] = self

    @property
    def value(self):
        return self._value

    @property
    def name(self):
        return self._name

    def __str__(self):
        return self._name

_vtypes: Dict[int, VariantType] = {}

class Variant:
    def __init__(self, vtype: Union[VariantType, int]):
        if isinstance(vtype, VariantType):
            self.vtype = vtype
        else:
            self.vtype = VariantType.get(vtype)


VariantType.NIL = VariantType(0, "NIL")
VariantType.BOOL = VariantType(1, "BOOL")
VariantType.INT = VariantType(2, "INT")
VariantType.REAL = VariantType(3, "REAL")
VariantType.STRING = VariantType(4, "STRING")
VariantType.VECTOR2 = VariantType(5, "VECTOR2")
VariantType.RECT2 = VariantType(6, "RECT2")
VariantType.VECTOR3 = VariantType(7, "VECTOR3")
VariantType.TRANSFORM2D = VariantType(8, "TRANSFORM2D")
VariantType.PLANE = VariantType(9, "PLANE")
VariantType.QUAT = VariantType(10, "QUAT")
VariantType.AABB = VariantType(11, "AABB")
VariantType.BASIS = VariantType(12, "BASIS")
VariantType.TRANSFORM = VariantType(13, "TRANSFORM")
VariantType.COLOR = VariantType(14, "COLOR")
VariantType.NODE_PATH = VariantType(15, "NODE_PATH")
VariantType.RID = VariantType(16, "_RID")
VariantType.OBJECT = VariantType(17, "OBJECT")
VariantType.DICTIONARY = VariantType(18, "DICTIONARY")
VariantType.ARRAY = VariantType(19, "ARRAY")
VariantType.POOL_BYTE_ARRAY = VariantType(20, "POOL_BYTE_ARRAY")
VariantType.POOL_INT_ARRAY = VariantType(21, "POOL_INT_ARRAY")
VariantType.POOL_REAL_ARRAY = VariantType(22, "POOL_REAL_ARRAY")
VariantType.POOL_STRING_ARRAY = VariantType(23, "POOL_STRING_ARRAY")
VariantType.POOL_VECTOR2_ARRAY = VariantType(24, "POOL_VECTOR2_ARRAY")
VariantType.POOL_VECTOR3_ARRAY = VariantType(25, "POOL_VECTOR3_ARRAY")
VariantType.POOL_COLOR_ARRAY = VariantType(26, "POOL_COLOR_ARRAY")
