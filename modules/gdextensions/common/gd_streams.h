/*************************************************************************/
/*  gd_streams.h                                                         */
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

// This file implements an output stream linked to Godot's console.
// Author: Joshua "Jdbener" Dahl
//
// Usage:
// gd::print << "A simple message" << gd::endl; // All of the manipulators in iomanip should work propperly.
// gd::print.warn("Warning message with all of the file/line number information automatically included");
// gd::print.error("Error message with all of the file/line number information automatcially included");
//
// NOTE: Messages are synced to Godot when a '\n' or an std::endl is encountered.
// The last character sent must be one of these for the things before it
// to print.
// NOTE: By default, string literals are interpreted as char*s... to use Unicode
// symbols make sure to start the literal with an L, ex: L"152 μs."
// NOTE: std::flush is not supported since Godot's print function automatically
// ends the line, thus std::flush would produce odd results equivalent to std::endl.
// Use std::endl instead. (std::flush won't cause errors, it just does nothing impactful)
//
// License: You are welcome to use this however you would like! No warranty and no credit needed.

#ifndef GD_STREAM_H
#define GD_STREAM_H

#include "core/error_macros.h"
#include "core/ustring.h"

#include "current_function.h"

#include <sstream>

#ifndef GODOT_NO_STRING_STREAM_INSERTION_OPERATOR
// Define how Godot Strings get processed by wostreams
inline std::wostream &operator<<(std::wostream &stream, const String &string) {
	stream << string.unicode_str();
	return stream;
}
// By defining an rvalue function as well, any datatype with a String() opperator
// is now supported by any wostream
inline std::wostream &operator<<(std::wostream &stream, const String &&string) {
	return stream << string;
}
#endif // GODOT_NO_STRING_STREAM_INSERTION_OPERATOR

namespace {
void godot_print_error(const char *p_description, const char *p_function, const char *p_file, int p_line) {
	_err_print_error(p_function, p_file, p_line, p_description, ERR_HANDLER_ERROR);
}

void godot_print_warning(const char *p_description, const char *p_function, const char *p_file, int p_line) {
	_err_print_error(p_function, p_file, p_line, p_description, ERR_HANDLER_WARNING);
}

void godot_print(const godot_string *p_message) {
	print_line(*(String *)p_message);
}
} // namespace

namespace gd {
using std::endl;

// Function which converts anything with << overloaded for wostreams, into a Godot String
template <class T>
String to_string(const T &t) {
	std::wstringstream ss;
	ss << t;
	return String(ss.str().c_str());
}

// Original class code from: http://videocortex.io/2017/custom-stream-buffers/
class godot_streambuf : public std::wstreambuf {
private:
	// Buffer which is used to store characters before sending them to Godot
	std::wstring buffer;

public:
	// Function which constucts an instance of the stream buffer
	static godot_streambuf *get_singleton() {
		static godot_streambuf gdbuf = godot_streambuf();
		return &gdbuf;
	}

protected:
	// Function which processes the characters in the stream as they arrive.
	void storeCharacter(const int_type character) {
		// If the character is a newline: send the buffer to Godot, then clear the buffer
		if (character == '\n') {
			String toPrint = buffer.c_str();
			if (toPrint.length() == 0)
				toPrint = wchar_t(' '); // Print a space to ensure extra newlines happen
			Godot::print(toPrint);
			buffer.clear();
			// Otherwise add the character to the buffer
		} else
			buffer += wchar_t(character);
	}

	// Functions which direct characters to storeCharacter()
	std::streamsize xsputn(const char_type *string, std::streamsize count) override {
		for (int i = 0; i < count; i++)
			storeCharacter(string[i]);
		return count; // returns the number of characters successfully written.
	}
	int_type overflow(int_type character) override {
		storeCharacter(character);
		return 1;
	}
};

// Wrapper class including warn and error functions
class gostream : public std::wostream {
public:
	gostream() :
			std::wostream(godot_streambuf::get_singleton()) {}

	void warn(const String description, const String function, const String file, int line) {
		Godot::print_warning(description, function, file, line);
	}
	void error(const String description, const String function, const String file, int line) {
		Godot::print_error(description, function, file, line);
	}
};

// Implementaion of the streambuffer
static gostream print;
}; // namespace gd

// Macros add in warning and error infromation automatically
#ifdef BOOST_CURRENT_FUNCTION
#define warn(description) warn(description, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__)
#define error(description) error(description, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__)
#ifdef __FUNCTION__
#define warn(description) warn(description, __FUNCTION__, __FILE__, __LINE__)
#define error(description) error(description, __FUNCTION__, __FILE__, __LINE__)
#else
#define warn(description) warn(description, __func__, __FILE__, __LINE__)
#define error(description) error(description, __func__, __FILE__, __LINE__)
#endif

#endif // GD_STREAM_H
