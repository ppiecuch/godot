/*************************************************************************/
/*  sw_local_file_storage.cpp                                            */
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

#include "silent_wolf.h"

#include "core/variant.h"
#include "core/io/json.h"
#include "core/os/file_access.h"

// Retrieves data stored as JSON in local storage
// example path: "user://swsession.save"

// store lookup (not logged in player name) and validator in local file

void sw_save_data(const String &path, const Dictionary &data, const String &debug_message) {
	FileAccessRef local_file = FileAccess::open(path, FileAccess::WRITE);
	ERR_FAIL_COND_MSG(!local_file, "Failed to open file: " + path);
	if (!debug_message.empty()) {
		sw_debug(debug_message, data);
	}
	local_file->store_line(JSON::print(data));
}

void sw_remove_data(const String &path, const String &debug_message) {
	FileAccessRef local_file = FileAccess::open(path, FileAccess::WRITE);
	ERR_FAIL_COND_MSG(!local_file, "Failed to open file: " + path);
	Dictionary data;
	if (!debug_message.empty()) {
		sw_debug(debug_message, data);
	}
	local_file->store_line(JSON::print(data));
}

bool sw_does_file_exist(const String &path) {
	FileAccessRef file_check = FileAccess::create_for_path(path);
	return file_check ? file_check->file_exists(path) : false;
}

Dictionary sw_get_data(const String &path) {
	Dictionary return_data;
	if (sw_does_file_exist(path)) {
		FileAccessRef local_file = FileAccess::open(path, FileAccess::READ);
		ERR_FAIL_COND_V_MSG(!local_file, Dictionary(), "Failed to open file: " + path);
		Error file_err;
		String json_string = local_file->get_file_as_string(path, &file_err);
		ERR_FAIL_COND_V_MSG(file_err != OK, Dictionary(), "Can not read file.");
		Variant data;
		String error_string;
		int error_line = 0;
		int err = JSON::parse(json_string, data, error_string, error_line);
		ERR_FAIL_COND_V_MSG(err != OK, Dictionary(), "Can not parse JSON: " + error_string + " on line " + rtos(error_line));
		if (data.get_type() == Variant::DICTIONARY) {
			return_data = data;
		} else {
			sw_warn("Invalid data in local storage");
		}
	} else {
		sw_warn("Could not find any data at: ", path);
	}
	return return_data;
}
