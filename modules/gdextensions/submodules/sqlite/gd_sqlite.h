/* gd_sqlite.h */
#ifndef GD_SQLITE_H
#define GD_SQLITE_H

#include "core/reference.h"
#include "core/ustring.h"
#include "core/map.h"

#include "sqlite/sqlite3.h"

class SQLite : public Reference {
	GDCLASS(SQLite,Reference);

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
