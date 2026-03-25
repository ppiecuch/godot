"""Tests for target system — verifies both gdnative and cppnative targets load."""
import unittest
import sys
import os

# Ensure the gd2c package is importable
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))


class TestTargetImports(unittest.TestCase):
    """Verify both targets can be imported without errors."""

    def test_import_gdnative_target(self):
        from gd2c.targets.gdnative import GDNativeTarget
        self.assertIsNotNone(GDNativeTarget)

    def test_import_cppnative_target(self):
        from gd2c.targets.cppnative import CPPNativeTarget
        self.assertIsNotNone(CPPNativeTarget)

    def test_import_gdnative_submodules(self):
        from gd2c.targets._gdnative.context import GlobalContext, ClassContext, FunctionContext
        from gd2c.targets._gdnative import builtin
        from gd2c.targets._gdnative import transform
        from gd2c.targets._gdnative import class_codegen
        from gd2c.targets._gdnative import function_codegen
        self.assertIsNotNone(GlobalContext)

    def test_import_cppnative_submodules(self):
        from gd2c.targets._cppnative.context import GlobalContext, ClassContext, FunctionContext
        from gd2c.targets._cppnative import builtin
        from gd2c.targets._cppnative import transform
        from gd2c.targets._cppnative import class_codegen
        from gd2c.targets._cppnative import function_codegen
        self.assertIsNotNone(GlobalContext)


class TestGetTarget(unittest.TestCase):
    """Test the get_target dispatcher in gd2c.py."""

    def test_get_cppnative_target(self):
        # Import the get_target function
        sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
        # We can't call get_target directly without a Project, but we can
        # verify the import path works
        from gd2c.targets.cppnative import CPPNativeTarget
        self.assertTrue(hasattr(CPPNativeTarget, 'transform'))
        self.assertTrue(hasattr(CPPNativeTarget, 'emit'))

    def test_get_gdnative_target(self):
        from gd2c.targets.gdnative import GDNativeTarget
        self.assertTrue(hasattr(GDNativeTarget, 'transform'))
        self.assertTrue(hasattr(GDNativeTarget, 'emit'))

    def test_targets_have_same_interface(self):
        from gd2c.targets.cppnative import CPPNativeTarget
        from gd2c.targets.gdnative import GDNativeTarget
        # Both should have the same public methods
        cpp_methods = {m for m in dir(CPPNativeTarget) if not m.startswith('_')}
        gdn_methods = {m for m in dir(GDNativeTarget) if not m.startswith('_')}
        self.assertEqual(cpp_methods, gdn_methods,
                         f"Target interfaces differ: cpp={cpp_methods - gdn_methods}, gdn={gdn_methods - cpp_methods}")


if __name__ == "__main__":
    unittest.main()
