from __future__ import annotations
from typing import List
from gd2c.project import load_project, Project
from gd2c.gdscriptclass import GDScriptClass
from gd2c.target import Target
from gd2c import transform
from gd2c import controlflow
from gd2c import domtree
from gd2c import ssa
from gd2c import analysis

def print_info(project, print_cfg, print_domtree):
	for cls in project.classes():
		print(f"---------------------------------------------")
		print(f"Class: {cls.name}")
		print(f"---------------------------------------------")
		for func in cls.functions():
			if func.cfg is None:
				func.cfg = controlflow.build_control_flow_graph(func)
			#to_ssa_form(func)
			func.pretty_print(True)
			#func.cfg.update_function(func)

			if print_cfg:
				func.cfg.pretty_print(True, True)

			if print_domtree:
				print("-- DOMTREE ---------------------------------")
				tree = domtree.build_domtree_naive(cfg)
				tree.pretty_print()

		print("\n")

def get_target(target_name: str, project: Project) -> Target:
	if target_name == "cppnative":
		from gd2c.targets.cppnative import CPPNativeTarget
		return CPPNativeTarget(project)

	raise Exception("target not known: " + target_name)

def assert_nothing_in_ssa_form(project: Project):
	for cls in project.iter_classes_in_dependency_order():
		for func in cls.functions():
			if func.cfg:
				assert not func.cfg.is_in_ssa_form

if __name__ == "__main__":
	import argparse
	import sys

	if sys.version_info < (3, 0, 0):
		sys.stderr.write("You need python 3.0 or later to run this script\n")
		exit(1)

	parser = argparse.ArgumentParser(description='gd2c: GodotEngine bytecode to native converter')
	parser.add_argument('project_path',
						metavar='<Project path>',
						type=str,
						help='the path to project')
	parser.add_argument('output_path',
						metavar='<Output path>',
						type=str,
						help='the path to output')
	parser.add_argument('target',
						default='cppnative',
						metavar='<Target>',
						type=str,
						help='target translator')
	args = parser.parse_args()

	project_path = args.project_path
	project_output_path = args.output_path
	project_target = args.target
	project = load_project(project_path)

	# Phase 0: Analyze
	for cls in project.iter_classes_in_dependency_order():
		for func in cls.functions():
			func.cfg = controlflow.build_control_flow_graph(func)
			analysis.annotate_coroutines(func)
			analysis.annotate_assigned_parameters(func)
			analysis.annotate_loops(func)

	# Phase 1: Compile to intermediate
	for cls in project.iter_classes_in_dependency_order():
		for func in cls.functions():
			if func.yields:
				transform.make_coroutine(func)

			func.cfg = controlflow.build_control_flow_graph(func)

			# Transforms not requiring SSA form
			transform.strip_debug(func)
			# func.cfg.pretty_print()

			# Transforms done in SSA form
			if False:
				ssa.to_ssa_form(func)
				transform.promote_typed_arithmetic(func)
				transform.substitute_intrinsics(func)

				iteration_count = 0
				iterate = True
				while iterate and iteration_count < 10:
					iterate = any([
						transform.common_subexpression_elimination(func),
						transform.copy_elimination(func),
						transform.dead_code_elimination(func),
						transform.redundant_phi_arg_elimination(func)])

				# Done doing SSA transformations
				ssa.from_ssa_form(func)

			func.cfg.update_function(func)
			func.cfg = None

	# Phase 2: Apply target-specific transformations
	target = get_target(project_target, project)
	target.transform()

	# Phase 3: Emit code
	target.emit(project_output_path)

	# print_info(project, False, False)
	for cls in project.iter_classes_in_dependency_order():
		print(cls.resource_path, cls.name, sep=" -> ")



