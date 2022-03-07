/*************************************************************************/
/*  TTFExceptions.cpp                                                    */
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

#include <cstdint>
#include <exception>
#include <sstream>
#include <string>

#include "TTFExceptions.h"

using namespace TTFCore;

FontException::FontException() :
		msg("Unknown error message.") {}
FontException::FontException(const char *msg_) :
		msg(msg_) {}
FontException::FontException(const std::string &msg_) :
		msg(msg_) {}
const char *FontException::what() const throw() {
	return msg.c_str();
}

FileFailure::FileFailure(const std::string &flnm) {
	std::stringstream ss;
	ss << "Unknown error reading file: '" << flnm << "'.";
	msg = ss.str();
}

FileLengthError::FileLengthError() :
		FontException("Length error reading memory mapped file, file has size of 0.") {}

FileLengthError::FileLengthError(const std::string &flnm) {
	std::stringstream ss;
	ss << "Length error reading file: '" << flnm << "', file has size of 0.";
	msg = ss.str();
}

TableDoesNotExist::TableDoesNotExist(const std::string &table) {
	std::stringstream ss;
	ss << "Cannot load font, '" << table << "' table does not exist.";
	msg = ss.str();
}

ChecksumException::ChecksumException(const std::string &table) {
	std::stringstream ss;
	ss << "Check sum error in '" << table << "'.";
	msg = ss.str();
}

VersionException::VersionException(const std::string &msg) :
		FontException(msg) {}

InvalidFontException::InvalidFontException(const std::string &msg) :
		FontException(msg) {}

UnsupportedCap::UnsupportedCap(const std::string &msg) :
		FontException(msg) {}

InvalidContour::InvalidContour(const std::string &msg) :
		FontException(msg) {}

InternalError::InternalError(const std::string &msg) :
		FontException(msg) {}
