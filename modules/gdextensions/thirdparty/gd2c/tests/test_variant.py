"""Tests for variant type system."""
import unittest
from gd2c.variant import VariantType, Variant


class TestVariantType(unittest.TestCase):
    def test_all_types_defined(self):
        expected = [
            "NIL", "BOOL", "INT", "REAL", "STRING",
            "VECTOR2", "RECT2", "VECTOR3", "TRANSFORM2D",
            "PLANE", "QUAT", "AABB", "BASIS", "TRANSFORM",
            "COLOR", "NODE_PATH", "RID", "OBJECT",
            "DICTIONARY", "ARRAY",
            "POOL_BYTE_ARRAY", "POOL_INT_ARRAY", "POOL_REAL_ARRAY",
            "POOL_STRING_ARRAY", "POOL_VECTOR2_ARRAY",
            "POOL_VECTOR3_ARRAY", "POOL_COLOR_ARRAY",
        ]
        for name in expected:
            vt = getattr(VariantType, name)
            self.assertIsNotNone(vt, f"VariantType.{name} should exist")
            self.assertIsInstance(vt, VariantType)

    def test_nil_is_zero(self):
        self.assertEqual(VariantType.NIL.value, 0)

    def test_variant_max(self):
        self.assertEqual(VariantType.VARIANT_MAX, 27)

    def test_sequential_values(self):
        for i in range(VariantType.VARIANT_MAX):
            vt = VariantType.get(i)
            self.assertEqual(vt.value, i)

    def test_get_by_int(self):
        vt = VariantType.get(2)
        self.assertEqual(vt, VariantType.INT)

    def test_get_by_string(self):
        vt = VariantType.get("4")
        self.assertEqual(vt, VariantType.STRING)

    def test_get_by_variant_type(self):
        vt = VariantType.get(VariantType.BOOL)
        self.assertEqual(vt, VariantType.BOOL)

    def test_get_none_returns_nil(self):
        vt = VariantType.get(None)
        self.assertEqual(vt, VariantType.NIL)

    def test_str(self):
        self.assertEqual(str(VariantType.INT), "INT")
        self.assertEqual(str(VariantType.NIL), "NIL")
        self.assertEqual(str(VariantType.VECTOR2), "VECTOR2")

    def test_name_property(self):
        self.assertEqual(VariantType.REAL.name, "REAL")

    def test_value_property(self):
        self.assertEqual(VariantType.ARRAY.value, 19)


class TestVariant(unittest.TestCase):
    def test_create_with_type(self):
        v = Variant(VariantType.INT)
        self.assertEqual(v.vtype, VariantType.INT)

    def test_create_with_int(self):
        v = Variant(3)
        self.assertEqual(v.vtype, VariantType.REAL)

    def test_create_nil(self):
        v = Variant(VariantType.NIL)
        self.assertEqual(v.vtype, VariantType.NIL)


if __name__ == "__main__":
    unittest.main()
