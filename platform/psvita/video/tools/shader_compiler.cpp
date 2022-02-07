/*************************************************************************/
/*  shader_compiler.cpp                                                  */
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

// See the original implementation: https://code.google.com/p/nya-engine/

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "shader_code_parser.h"

const char *help = "Usage: shader_compiler %%mode%%\n"
				   "accepts shader's code from stdin\n"
				   "stderr output begins with Error: if something goes wrong\n"
				   "outputs shader text to stdout\n"
				   "modes:\n"
				   "glsl2hlsl - converts glsl shader to hlsl\n"
				   "\n";

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "[Error] No arguments\n");
		printf("%s", help);
		return 0;
	}

#if O_BINARY
	setmode(fileno(stdout), O_BINARY);
	setmode(fileno(stdin), O_BINARY);
#endif

	std::string shader_code;
	char buf[512];
	for (size_t size = fread(buf, 1, sizeof(buf), stdin); size > 0; size = fread(buf, 1, sizeof(buf), stdin))
		shader_code.append(buf, size);

	if (shader_code.empty()) {
		fprintf(stderr, "[Error] Empty stdin\n");
		printf("%s", help);
		return 0;
	}

	if (strcmp(argv[1], "glsl2hlsl") == 0) {
		shader_code_parser parser(shader_code.c_str(), "_gl_", "_flip_y_");
		if (!parser.convert_to_hlsl()) {
			fprintf(stderr, "[Error] Cannot convert to hlsl: %s", parser.get_error());
			return -1;
		}

		printf("%s", parser.get_code());
		return 0;
	}

	fprintf(stderr, "[Error] Invalid compile mode: %s\n", argv[1]);
	return -1;
}
