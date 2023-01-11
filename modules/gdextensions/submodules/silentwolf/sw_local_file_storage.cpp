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

// Retrieves data stored as JSON in local storage
// example path: "user://swsession.save"

// store lookup (not logged in player name) and validator in local file

void sw_save_data(const String &path, const Dictionary &data, const String &debug_message = "Saving data to file in local storage: ") {
	var local_file = File.new();
	local_file.open(path, File.WRITE);
	sw_debug(debug_message + str(data));
	local_file.store_line(to_json(data));
	local_file.close();
}

void sw_remove_data(const String &path, const String &debug_message = "Removing data from file in local storage: ") {
	var local_file = File.new();
	local_file.open(path, File.WRITE);
	var data = {};
	sw_debug(debug_message + str(data));
	local_file.store_line(to_json(data));
	local_file.close();
}

bool sw_does_file_exist(const String &path, Ref<File> file) {
	Ref<File> local_file = file;
	if (local_file.is_null()) {
		local_file = newref(File);
	}
	return local_file->file_exists(path);
}

Dictionary sw_get_data(const String &path) {
	var local_file = File.new();
	var return_data = null;
	if (does_file_exist(path, local_file)) {
		local_file.open(path, File.READ);
		var data = parse_json(local_file.get_as_text());
		if (typeof(data) == TYPE_DICTIONARY) {
			return_data = data;
		} else {
			sw_debug("Invalid data in local storage");
		}
	} else {
		sw_debug("Could not find any data at: ", path);
	}
	return return_data;
}
