"""Tests for address mode encoding/decoding."""
import unittest
from gd2c.address import (
    GDScriptAddress,
    ADDRESS_MODE_SELF, ADDRESS_MODE_CLASS, ADDRESS_MODE_MEMBER,
    ADDRESS_MODE_CLASSCONSTANT, ADDRESS_MODE_LOCALCONSTANT,
    ADDRESS_MODE_STACK, ADDRESS_MODE_STACKVARIABLE, ADDRESS_MODE_GLOBAL,
    ADDRESS_MODE_NAMEDGLOBAL, ADDRESS_MODE_NIL, ADDRESS_MODE_PARAMETER,
    ADDRESS_MODE_TEMPORARY, ADDRESS_MODE_SETTERVALUEPARAMETER,
    ADDRESS_BITS, ADDRESS_MASK, ADDRESS_MODE_MASK,
    AddressModePrefix,
)


class TestAddressEncoding(unittest.TestCase):
    def test_create_and_decompose(self):
        addr = GDScriptAddress.create(ADDRESS_MODE_STACKVARIABLE, 42)
        self.assertEqual(addr.mode, ADDRESS_MODE_STACKVARIABLE)
        self.assertEqual(addr.offset, 42)

    def test_zero_offset(self):
        addr = GDScriptAddress.create(ADDRESS_MODE_MEMBER, 0)
        self.assertEqual(addr.mode, ADDRESS_MODE_MEMBER)
        self.assertEqual(addr.offset, 0)

    def test_large_offset(self):
        max_offset = ADDRESS_MASK
        addr = GDScriptAddress.create(ADDRESS_MODE_GLOBAL, max_offset)
        self.assertEqual(addr.offset, max_offset)
        self.assertEqual(addr.mode, ADDRESS_MODE_GLOBAL)

    def test_all_modes_roundtrip(self):
        modes = [
            ADDRESS_MODE_SELF, ADDRESS_MODE_CLASS, ADDRESS_MODE_MEMBER,
            ADDRESS_MODE_CLASSCONSTANT, ADDRESS_MODE_LOCALCONSTANT,
            ADDRESS_MODE_STACK, ADDRESS_MODE_STACKVARIABLE, ADDRESS_MODE_GLOBAL,
            ADDRESS_MODE_NAMEDGLOBAL, ADDRESS_MODE_NIL, ADDRESS_MODE_PARAMETER,
            ADDRESS_MODE_TEMPORARY, ADDRESS_MODE_SETTERVALUEPARAMETER,
        ]
        for mode in modes:
            addr = GDScriptAddress.create(mode, 7)
            self.assertEqual(addr.mode, mode, f"mode {mode} roundtrip failed")
            self.assertEqual(addr.offset, 7)

    def test_equality(self):
        a = GDScriptAddress.create(ADDRESS_MODE_STACK, 10)
        b = GDScriptAddress.create(ADDRESS_MODE_STACK, 10)
        c = GDScriptAddress.create(ADDRESS_MODE_STACK, 11)
        self.assertEqual(a, b)
        self.assertNotEqual(a, c)

    def test_hash_consistency(self):
        a = GDScriptAddress.create(ADDRESS_MODE_MEMBER, 3)
        b = GDScriptAddress.create(ADDRESS_MODE_MEMBER, 3)
        self.assertEqual(hash(a), hash(b))

    def test_zero_address(self):
        zero = GDScriptAddress.Zero
        self.assertEqual(zero.mode, ADDRESS_MODE_SELF)
        self.assertEqual(zero.offset, 0)

    def test_calc_address_matches_create(self):
        mode, offset = ADDRESS_MODE_PARAMETER, 5
        raw = GDScriptAddress.calc_address(mode, offset)
        addr = GDScriptAddress.create(mode, offset)
        self.assertEqual(raw, addr.address)

    def test_str_representation(self):
        addr = GDScriptAddress.create(ADDRESS_MODE_STACKVARIABLE, 3)
        s = str(addr)
        self.assertIn("stva", s)
        self.assertIn("3", s)

    def test_all_modes_have_prefix(self):
        for mode in AddressModePrefix:
            self.assertIsInstance(AddressModePrefix[mode], str)
            self.assertGreater(len(AddressModePrefix[mode]), 0)


class TestAddressBitLayout(unittest.TestCase):
    def test_address_bits_is_24(self):
        self.assertEqual(ADDRESS_BITS, 24)

    def test_mask_covers_lower_24_bits(self):
        self.assertEqual(ADDRESS_MASK, 0xFFFFFF)

    def test_mode_mask_covers_upper_bits(self):
        self.assertEqual(ADDRESS_MODE_MASK & ADDRESS_MASK, 0)


if __name__ == "__main__":
    unittest.main()
