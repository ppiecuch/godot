/**************************************************************************/
/*  gd_flashdb.h                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifndef GD_FLASHDB_H
#define GD_FLASHDB_H

#include "core/reference.h"
#include "core/variant.h"

struct fdb_kvdb;
struct fdb_tsdb;

// FlashDB Key-Value Database — persistent key-value store backed by file
class FlashKVDB : public Reference {
	GDCLASS(FlashKVDB, Reference);

	struct fdb_kvdb *_db;
	bool _initialized;
	String _path;
	String _name;

protected:
	static void _bind_methods();

public:
	// Initialize the database at the given directory path
	// sector_size: flash sector size (4096 typical), max_size: max DB size (0=unlimited)
	bool open(const String &p_name, const String &p_path, int p_sector_size = 4096, int p_max_size = 0);
	void close();
	bool is_open() const;

	// String key-value operations
	bool set_string(const String &p_key, const String &p_value);
	String get_string(const String &p_key, const String &p_default = "") const;

	// Binary data operations
	bool set_data(const String &p_key, const PoolByteArray &p_data);
	PoolByteArray get_data(const String &p_key) const;

	// Variant operations (auto-serializes via JSON)
	bool set_value(const String &p_key, const Variant &p_value);
	Variant get_value(const String &p_key, const Variant &p_default = Variant()) const;

	// Delete and query
	bool delete_key(const String &p_key);
	bool has_key(const String &p_key) const;
	Array get_all_keys() const;
	int get_key_count() const;

	// Reset to defaults
	void reset_defaults();

	FlashKVDB();
	~FlashKVDB();
};

// FlashDB Time Series Database — append-only timestamped log
class FlashTSDB : public Reference {
	GDCLASS(FlashTSDB, Reference);

	struct fdb_tsdb *_db;
	bool _initialized;
	String _path;
	String _name;

protected:
	static void _bind_methods();

public:
	// Initialize the TSDB at the given directory path
	// max_len: max record length, sector_size/max_size as above
	bool open(const String &p_name, const String &p_path, int p_max_len = 256, int p_sector_size = 4096, int p_max_size = 0);
	void close();
	bool is_open() const;

	// Append a record (string or binary)
	bool append_string(const String &p_data);
	bool append_data(const PoolByteArray &p_data);
	bool append_value(const Variant &p_value);

	// Query records
	Array get_all_records() const;
	Array get_records_by_time(int64_t p_from, int64_t p_to) const;
	int get_record_count() const;

	// Maintenance
	void clean();
	void set_rollover(bool p_enable);

	FlashTSDB();
	~FlashTSDB();
};

#endif // GD_FLASHDB_H
