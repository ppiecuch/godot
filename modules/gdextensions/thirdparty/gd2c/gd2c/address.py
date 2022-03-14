from __future__ import annotations
from typing import ClassVar, cast

ADDRESS_MODE_SELF = 0
ADDRESS_MODE_CLASS = 1
ADDRESS_MODE_MEMBER = 2
ADDRESS_MODE_CLASSCONSTANT = 3
ADDRESS_MODE_LOCALCONSTANT = 4
ADDRESS_MODE_STACK = 5
ADDRESS_MODE_STACKVARIABLE = 6
ADDRESS_MODE_GLOBAL = 7
ADDRESS_MODE_NAMEDGLOBAL = 8
ADDRESS_MODE_NIL = 9
# Function parameter not defined by GDSCRIPT
ADDRESS_MODE_PARAMETER = 14
ADDRESS_MODE_TEMPORARY = 15
ADDRESS_MODE_SETTERVALUEPARAMETER = 16

AddressModePrefix = {
    ADDRESS_MODE_SELF: "self",
    ADDRESS_MODE_CLASS: "class",
    ADDRESS_MODE_MEMBER: "memb",
    ADDRESS_MODE_CLASSCONSTANT: "clco",
    ADDRESS_MODE_LOCALCONSTANT: "loco",
    ADDRESS_MODE_STACK: "stac",
    ADDRESS_MODE_STACKVARIABLE: "stva",
    ADDRESS_MODE_GLOBAL: "glob",
    ADDRESS_MODE_NAMEDGLOBAL: "namg",
    ADDRESS_MODE_NIL: " nil",
    ADDRESS_MODE_PARAMETER: "para",
    ADDRESS_MODE_TEMPORARY: "temp",
    ADDRESS_MODE_SETTERVALUEPARAMETER: "setv"
}

ADDRESS_BITS = 24
ADDRESS_MASK = ((1 << 24) - 1)
ADDRESS_MODE_MASK = ~((1 << 24) - 1)

class GDScriptAddress:
    Zero: ClassVar['GDScriptAddress'] = cast('GDScriptAddress', None)

    def __init__(self, address: int):
        self._address = address

    @property
    def address(self):
        return self._address

    @property
    def mode(self):
        return (self._address & ADDRESS_MODE_MASK) >> ADDRESS_BITS

    @property
    def offset(self):
        return self._address & ADDRESS_MASK

    def __eq__(self, other):
        return other and self._address == other._address

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        return hash(self._address)

    @staticmethod
    def create(mode: int, offset: int) -> GDScriptAddress:
        return GDScriptAddress((mode << ADDRESS_BITS) | (offset & ADDRESS_MASK))

    @staticmethod
    def calc_address(mode: int, offset: int) -> int:
        return (mode << ADDRESS_BITS) | (offset & ADDRESS_MASK)

    def __str__(self) -> str:
        return f"\"{AddressModePrefix[self.mode]}#{self.offset}\""

GDScriptAddress.Zero = GDScriptAddress(0)

