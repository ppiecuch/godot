/*************************************************************************/
/*  main.cpp                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#define CATCH_CONFIG_RUNNER
#include "main/main.h"
#include "catch.hpp"
#include "core/math/face3.h"
#include "platform/server/os_server.h"
#include <string.h>

// Find the index of the --godot argument. If no argument
// found then returns -1
int get_start_of_godot_args(int argc, char *argv[]) {
	// The first index is reserved for the program name so we can skip it
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "--godot", 2) == 0) {
			return i;
		}
	}

	// Because 0 is invalid this could also have just been 0, but
	// let's try to keep things a little less confusing
	return -1;
}

// Allocates and fills a new array of char pointers that points to godot arguments
char **make_godot_args(int godot_args_length, int start_of_godot_args, char *argv[]) {
	// We shouldn't need to worry too much about memory management here
	char **godot_args = new char *[godot_args_length];

	// We don't want the --godot arg but we do want the program name
	godot_args[0] = argv[0];

	for (int i = 1; i < godot_args_length; i++) {
		godot_args[i] = argv[start_of_godot_args + i];
	}

	return godot_args;
}

int main(int argc, char *argv[]) {
	// Allow passing in commands into both Catch and the Godot server
	// by splitting on a "--godot" arg
	int start_of_godot_args = get_start_of_godot_args(argc, argv);
	int godot_args_length = start_of_godot_args != -1 ? argc - start_of_godot_args : 0;
	int catch_args_length = argc - godot_args_length;
	char **godot_args = make_godot_args(godot_args_length, start_of_godot_args, argv);

	OS_Server os;
	Error err = Main::setup(argv[0], godot_args_length, godot_args);
	if (err != OK)
		return 255;

	int result = Catch::Session().run(catch_args_length, argv);

	Main::cleanup();
	delete godot_args;

	return result;
}
