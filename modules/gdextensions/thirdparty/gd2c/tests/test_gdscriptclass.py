"""Tests for GDScript class/function/member metadata structures."""
import unittest
from gd2c.gdscriptclass import (
    GDScriptClass, GDScriptFunction, GDScriptMember,
    GDScriptFunctionParameter, GDScriptFunctionConstant,
    GDScriptClassConstant, GDScriptGlobal,
)
from gd2c.variant import VariantType
from gd2c.address import ADDRESS_MODE_MEMBER, ADDRESS_MODE_STACKVARIABLE, ADDRESS_MODE_GLOBAL


class TestGDScriptMember(unittest.TestCase):
    def test_create(self):
        m = GDScriptMember("health", 0, VariantType.INT)
        self.assertEqual(m.name, "health")
        self.assertEqual(m.index, 0)
        self.assertEqual(m.vtype, VariantType.INT)
        self.assertFalse(m.is_inherited)

    def test_address_mode(self):
        m = GDScriptMember("pos", 3, VariantType.VECTOR2)
        self.assertEqual(m.address.mode, ADDRESS_MODE_MEMBER)
        self.assertEqual(m.address.offset, 3)

    def test_variant_type_from_int(self):
        m = GDScriptMember("x", 0, 2)  # 2 = INT
        self.assertEqual(m.vtype, VariantType.INT)


class TestGDScriptFunctionParameter(unittest.TestCase):
    def test_create(self):
        p = GDScriptFunctionParameter("speed", VariantType.REAL, 0)
        self.assertEqual(p.name, "speed")
        self.assertEqual(p.vtype, VariantType.REAL)
        self.assertEqual(p.index, 0)
        self.assertFalse(p.is_assigned)
        self.assertFalse(p.is_optional)

    def test_address_mode(self):
        p = GDScriptFunctionParameter("a", VariantType.INT, 2)
        self.assertEqual(p.address.mode, ADDRESS_MODE_STACKVARIABLE)
        self.assertEqual(p.address.offset, 2)


class TestGDScriptFunctionConstant(unittest.TestCase):
    def test_create(self):
        c = GDScriptFunctionConstant(0, VariantType.STRING, b"hello", '"hello"')
        self.assertEqual(c.index, 0)
        self.assertEqual(c.vtype, VariantType.STRING)


class TestGDScriptClassConstant(unittest.TestCase):
    def test_create(self):
        c = GDScriptClassConstant("MAX_HP", VariantType.INT, b"\x64", "100")
        self.assertEqual(c.name, "MAX_HP")
        self.assertEqual(c.vtype, VariantType.INT)


class TestGDScriptGlobal(unittest.TestCase):
    def test_create_constant_source(self):
        g = GDScriptGlobal(0, "PI", "PI", 3, 0, "3.14159", "GlobalConstants")
        self.assertEqual(g.source, GDScriptGlobal.SOURCE_CONSTANT)

    def test_create_classdb_source(self):
        g = GDScriptGlobal(1, "Node", "Node", 17, 0, "", "ClassDB")
        self.assertEqual(g.source, GDScriptGlobal.SOURCE_CLASSDB)

    def test_create_singleton_source(self):
        g = GDScriptGlobal(2, "OS", "OS", 17, 0, "", "Singleton")
        self.assertEqual(g.source, GDScriptGlobal.SOURCE_SINGLETON)


class TestGDScriptFunction(unittest.TestCase):
    def test_create(self):
        f = GDScriptFunction("_ready", False)
        self.assertEqual(f.name, "_ready")


class TestGDScriptClass(unittest.TestCase):
    def test_create(self):
        c = GDScriptClass("res://player.gd", "Player", 100)
        self.assertEqual(c.name, "Player")
        self.assertEqual(c.resource_path, "res://player.gd")

    def test_add_member(self):
        c = GDScriptClass("res://e.gd", "Enemy", 101)
        m = GDScriptMember("health", 0, VariantType.INT)
        c.add_member(m)
        members = list(c.members())
        self.assertEqual(len(members), 1)
        self.assertEqual(members[0].name, "health")

    def test_add_function(self):
        c = GDScriptClass("res://n.gd", "NPC", 102)
        f = GDScriptFunction("talk", False)
        c.add_function(f)
        funcs = list(c.functions())
        self.assertEqual(len(funcs), 1)
        self.assertEqual(funcs[0].name, "talk")


if __name__ == "__main__":
    unittest.main()
