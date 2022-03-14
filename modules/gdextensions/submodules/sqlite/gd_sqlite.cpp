/* gd_sqlite.cpp */

#include "gd_sqlite.h"
#include "core/project_settings.h"
#include "core/os/os.h"

#include <string>

Dictionary SQLite::deep_copy(Dictionary p_dict)
{
    Dictionary copy_dict;
    Array keys = p_dict.keys();
    int number_of_keys = keys.size();
    for (int i = 0; i <= number_of_keys - 1; i++) {
        copy_dict[keys[i]] = p_dict[keys[i]];
    }
    return copy_dict;
}

Variant SQLite::get_with_default(Dictionary p_dict, String p_key, Variant p_default)
{
    if (p_dict.has(p_key)) {
        return p_dict[p_key];
    }
    else {
        return p_default;
    }
}

int SQLite::open(String p_path) {

	String path = p_path;

	String ending = String(".db");
	if (!path.ends_with(ending)) {
		path += ending;
	}

	if (path.begins_with("res://")) {
		if (ProjectSettings::get_singleton()) {
			String resource_path = ProjectSettings::get_singleton()->get_resource_path();
			if (resource_path != "") path = path.replace("res:/",resource_path);
			else path = path.replace("res://", "");
		}
	}
	else if (path.begins_with("user://")) {
		String data_dir = OS::get_singleton()->get_data_path();
		if (data_dir != "") path = path.replace("user:/",data_dir);
		else path = path.replace("user://", "");
	}
	
	return sqlite3_open(path.utf8().get_data(), &db);
}

void SQLite::prepare(String p_query) {

	sqlite3_prepare_v2(db, p_query.utf8().get_data(), -1, &stmt, NULL);
}


bool SQLite::query_with_bindings(String p_query, Array param_bindings, Array query_result, bool verbose_mode)
{
    String err_msg;
    int rc;

    const char *sql = p_query.utf8().get_data();

    /* Clear the previous query results */
    query_result.clear();

    sqlite3_stmt *stmt;
    /* Prepare an SQL statement */
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    err_msg = String(sqlite3_errmsg(db));
    if (rc != SQLITE_OK) {
        ERR_PRINT("SQL error: " + err_msg);
        return false;
    }

    /* Bind any given parameters to the prepared statement */
    for (int i = 0; i < param_bindings.size(); i++) {
        switch (param_bindings[i].get_type()) {
            case Variant::NIL:
				sqlite3_bind_null(stmt, i + 1);
				break;

			case Variant::BOOL:
            case Variant::INT:
                sqlite3_bind_int64(stmt, i + 1, int64_t(param_bindings[i]));
                break;

            case Variant::REAL:
                sqlite3_bind_double(stmt, i + 1, param_bindings[i]);
                break;

            case Variant::STRING:
                sqlite3_bind_text(stmt, i + 1, (param_bindings[i].operator String()).utf8().get_data(), -1, SQLITE_TRANSIENT);
                break;

            case Variant::POOL_BYTE_ARRAY: {
                PoolByteArray binding = ((const PoolByteArray &)param_bindings[i]);
                PoolByteArray::Read r = binding.read();
                sqlite3_bind_blob64(stmt, i + 1, r.ptr(), binding.size(), SQLITE_TRANSIENT);
                break;
            }

            default:
                ERR_PRINT("GDSQLite Error: Binding a parameter of type " + String::num(param_bindings[i].get_type()) + " (TYPE_*) is not supported!");
                sqlite3_finalize(stmt);
                return false;
        }
    }

    // Execute the statement and iterate over all the resulting rows.
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Dictionary column_dict;
        int argc = sqlite3_column_count(stmt);

        /* Loop over all columns and add them to the Dictionary */
        for (int i = 0; i < argc; i++)
        {
            Variant column_value;
            /* Check the column type and do correct casting */
            switch (sqlite3_column_type(stmt, i))
            {
            case SQLITE_INTEGER:
                column_value = Variant((int64_t)sqlite3_column_int64(stmt, i));
                break;

            case SQLITE_FLOAT:
                column_value = Variant(sqlite3_column_double(stmt, i));
                break;

            case SQLITE_TEXT:
                column_value = Variant((char *)sqlite3_column_text(stmt, i));
                break;

            case SQLITE_BLOB: {
                int bytes = sqlite3_column_bytes(stmt, i);
                PoolByteArray arr = PoolByteArray();
                arr.resize(bytes);
                PoolByteArray::Write write = arr.write();
                memcpy(write.ptr(), (char *)sqlite3_column_blob(stmt, i), bytes);
                column_value = arr;
                break;
            }

            case SQLITE_NULL:
                break;

            default:
                break;
            }

            const char * col_name = sqlite3_column_name(stmt, i);
            column_dict[String(col_name)] = column_value;
        }
        /* Add result to query_result Array */
        query_result.append(column_dict);
    }

    // Clean up and delete the resources used by the prepared statement.
    sqlite3_finalize(stmt);

    rc = sqlite3_errcode(db);
    err_msg = String(sqlite3_errmsg(db));
    if (rc != SQLITE_OK) {
        ERR_PRINT("SQL error: " + err_msg);
        return false;
    }

    return true;
}

int SQLite::step() {

	return sqlite3_step(stmt);
}

int SQLite::step_assoc() {

	int ret = sqlite3_step(stmt);
	if (_row_names.size() == 0) {
		for(int i = 0; i < sqlite3_column_count(stmt); ++i)
			_row_names[sqlite3_column_name(stmt,i)] = i;
	}
	return ret;
}

Array SQLite::fetch_assoc() {

	Array result;
	while (1) {
		int ret = sqlite3_step(stmt);
		if (ret == SQLITE_ROW) {
			Dictionary row;
			for(int i = 0; i < sqlite3_column_count(stmt); ++i) {
				int type = sqlite3_column_type(stmt, i);
				switch (type) {
					case SQLITE_INTEGER: row[sqlite3_column_name(stmt,i)] = sqlite3_column_int(stmt, i); break;
					case SQLITE_FLOAT: row[sqlite3_column_name(stmt,i)] = sqlite3_column_double(stmt, i); break;
					case SQLITE_TEXT: row[sqlite3_column_name(stmt,i)] = String::utf8((char *)sqlite3_column_text(stmt, i)); break;
					default: break;
				}
			}
			result.push_back(row);
		}
		else if (ret == SQLITE_DONE) {
			break;
		}
		else {
			break;
		}
	}
	return result;
}

Array SQLite::fetch_one() {

	Array result = fetch_assoc();
	if (result.size() <= 0) { return Array(); }
	return result[0];
}

Array SQLite::fetch_array(String p_query) {

	prepare(p_query);
	Array result = fetch_assoc();
	finalize();
	return result;
}

int SQLite::query(String p_query) {

	prepare(p_query);
	int ret = step();
	finalize();
	return ret;
}

int SQLite::get_data_count() {

	return sqlite3_data_count(stmt);
}

int SQLite::get_column_count() {

	return sqlite3_column_count(stmt);
}

int SQLite::get_column_int(int p_col) {

	return sqlite3_column_int(stmt, p_col);
}

double SQLite::get_column_double(int p_col) {

	return sqlite3_column_double(stmt, p_col);
}

String SQLite::get_column_text(int p_col) {

	return String::utf8((char *)sqlite3_column_text(stmt, p_col));
}

int SQLite::get_column_int_assoc(String p_col) {

	return sqlite3_column_int(stmt, _row_names[p_col]);
}

double SQLite::get_column_double_assoc(String p_col) {

	return sqlite3_column_double(stmt, _row_names[p_col]);
}

String SQLite::get_column_text_assoc(String p_col) {

	return String::utf8((char *)sqlite3_column_text(stmt, _row_names[p_col]));
}

void SQLite::finalize() {

	sqlite3_finalize(stmt);
	_row_names.clear();
}

String SQLite::get_errormsg() {

	return sqlite3_errmsg(db);
}

void SQLite::close() {

	sqlite3_close(db);
}

void SQLite::_bind_methods() {

	ClassDB::bind_method("open", &SQLite::open);
	ClassDB::bind_method("prepare", &SQLite::prepare);
	ClassDB::bind_method("step", &SQLite::step);
	ClassDB::bind_method("step_assoc", &SQLite::step_assoc);
	ClassDB::bind_method("fetch_assoc", &SQLite::fetch_assoc);
	ClassDB::bind_method("fetch_one", &SQLite::fetch_one);
	ClassDB::bind_method("fetch_array", &SQLite::fetch_array);
	ClassDB::bind_method("query", &SQLite::query);

	ClassDB::bind_method("get_data_count", &SQLite::get_data_count);
	ClassDB::bind_method("get_column_count", &SQLite::get_column_count);
	ClassDB::bind_method("get_column_int", &SQLite::get_column_int);
	ClassDB::bind_method("get_column_double", &SQLite::get_column_double);
	ClassDB::bind_method("get_column_text", &SQLite::get_column_text);
	ClassDB::bind_method("finalize", &SQLite::finalize);
	ClassDB::bind_method("get_errormsg", &SQLite::get_errormsg);
	ClassDB::bind_method("close", &SQLite::close);

	ClassDB::bind_method("get_column_int_assoc", &SQLite::get_column_int_assoc);
	ClassDB::bind_method("get_column_double_assoc", &SQLite::get_column_double_assoc);
	ClassDB::bind_method("get_column_text_assoc", &SQLite::get_column_text_assoc);
	
	BIND_CONSTANT(SQLITE_OK);
	BIND_CONSTANT(SQLITE_ERROR);
	BIND_CONSTANT(SQLITE_INTERNAL);
	BIND_CONSTANT(SQLITE_PERM);
	BIND_CONSTANT(SQLITE_ABORT);
	BIND_CONSTANT(SQLITE_BUSY);
	BIND_CONSTANT(SQLITE_LOCKED);
	BIND_CONSTANT(SQLITE_NOMEM);
	BIND_CONSTANT(SQLITE_READONLY);
	BIND_CONSTANT(SQLITE_INTERRUPT);
	BIND_CONSTANT(SQLITE_IOERR);
	BIND_CONSTANT(SQLITE_CORRUPT);
	BIND_CONSTANT(SQLITE_NOTFOUND);
	BIND_CONSTANT(SQLITE_FULL);
	BIND_CONSTANT(SQLITE_CANTOPEN);
	BIND_CONSTANT(SQLITE_PROTOCOL);
	BIND_CONSTANT(SQLITE_EMPTY);
	BIND_CONSTANT(SQLITE_SCHEMA);
	BIND_CONSTANT(SQLITE_TOOBIG);
	BIND_CONSTANT(SQLITE_CONSTRAINT);
	BIND_CONSTANT(SQLITE_MISMATCH);
	BIND_CONSTANT(SQLITE_MISUSE);
	BIND_CONSTANT(SQLITE_NOLFS);
	BIND_CONSTANT(SQLITE_AUTH);
	BIND_CONSTANT(SQLITE_FORMAT);
	BIND_CONSTANT(SQLITE_RANGE);
	BIND_CONSTANT(SQLITE_NOTADB);
	BIND_CONSTANT(SQLITE_NOTICE);
	BIND_CONSTANT(SQLITE_WARNING);
	BIND_CONSTANT(SQLITE_ROW);
	BIND_CONSTANT(SQLITE_DONE);
}

SQLite::SQLite() {
}
