/**************************************************************************/
/*  gd_sqlite.h                                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

/* gd_sqlite.h */
#ifndef GD_SQLITE_H
#define GD_SQLITE_H

#include "core/map.h"
#include "core/reference.h"
#include "core/ustring.h"

#include "sqlite/sqlite3.h"

class SQLite : public Reference {
	GDCLASS(SQLite, Reference);

protected:
	static void _bind_methods();

public:
	SQLite();

	int open(String p_path);

	void prepare(String p_query);
	int step();
	int step_assoc();
	Array fetch_assoc();
	Array fetch_one();
	Array fetch_array(String p_query);
	int query(String query);
	bool query_with_bindings(String p_query, Array param_bindings, Array query_result, bool verbose_mode);
	int get_data_count();
	int get_column_count();
	int get_column_int(int p_col);
	double get_column_double(int p_col);
	String get_column_text(int p_col);
	int get_column_int_assoc(String p_col);
	double get_column_double_assoc(String p_col);
	String get_column_text_assoc(String p_col);

	void finalize();
	String get_errormsg();
	void close();

private:
	sqlite3 *db;
	sqlite3_stmt *stmt;

	Map<String, unsigned int> _row_names;

	Dictionary deep_copy(Dictionary p_dict);
	Variant get_with_default(Dictionary p_dict, String p_key, Variant p_default);
};

#endif // GD_SQLITE_H
