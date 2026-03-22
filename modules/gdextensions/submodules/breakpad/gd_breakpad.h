/**************************************************************************/
/*  gd_breakpad.h                                                         */
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

#ifndef GD_BREAKPAD_H
#define GD_BREAKPAD_H

#include "core/dictionary.h"
#include "core/os/dir_access.h"
#include "core/os/os.h"
#include "core/reference.h"
#include "core/ustring.h"
#include "core/variant.h"
#include "core/vector.h"

// Platform detection for breakpad backend selection.
// BREAKPAD_HAS_BACKEND is defined only when a prebuilt breakpad_client library
// is linked. Otherwise the stub backend is used (crash handler metadata only).
#if defined(BREAKPAD_HAS_LIBRARY)
#if defined(X11_ENABLED) || defined(LINUX_ENABLED)
#define BREAKPAD_PLATFORM_LINUX 1
#elif defined(OSX_ENABLED)
#define BREAKPAD_PLATFORM_MAC 1
#elif defined(WINDOWS_ENABLED)
#define BREAKPAD_PLATFORM_WINDOWS 1
#elif defined(IPHONE_ENABLED)
#define BREAKPAD_PLATFORM_IOS 1
#elif defined(ANDROID_ENABLED)
#define BREAKPAD_PLATFORM_ANDROID 1
#endif
#endif

// Forward declarations for platform-specific breakpad types
struct BreakpadPlatformHandler;

// Crash report metadata entry
struct CrashReportEntry {
	String dump_path;
	String minidump_id;
	String timestamp;
	bool succeeded;

	CrashReportEntry() :
			succeeded(false) {}
	CrashReportEntry(const String &p_path, const String &p_id, const String &p_time, bool p_ok) :
			dump_path(p_path), minidump_id(p_id), timestamp(p_time), succeeded(p_ok) {}

	Dictionary to_dictionary() const {
		Dictionary d;
		d["dump_path"] = dump_path;
		d["minidump_id"] = minidump_id;
		d["timestamp"] = timestamp;
		d["succeeded"] = succeeded;
		return d;
	}
};

// Annotations store: key-value metadata attached to crash reports.
// Uses fixed-size storage to be safe in crash contexts (no allocations).
class CrashAnnotations {
public:
	enum {
		MAX_ENTRIES = 64,
		MAX_KEY_LEN = 64,
		MAX_VALUE_LEN = 256,
	};

	struct Entry {
		char key[MAX_KEY_LEN];
		char value[MAX_VALUE_LEN];
		bool active;

		Entry() :
				active(false) {
			key[0] = '\0';
			value[0] = '\0';
		}
	};

	CrashAnnotations() :
			count(0) {}

	bool set(const String &p_key, const String &p_value);
	bool remove(const String &p_key);
	String get(const String &p_key) const;
	bool has(const String &p_key) const;
	void clear();
	int size() const { return count; }
	Dictionary to_dictionary() const;

	const Entry *get_entries() const { return entries; }

private:
	Entry entries[MAX_ENTRIES];
	int count;

	int find_index(const String &p_key) const;
};

// Main Godot crash handler wrapper
class GdBreakpad : public Reference {
	GDCLASS(GdBreakpad, Reference)

public:
	enum Platform {
		PLATFORM_UNKNOWN = 0,
		PLATFORM_LINUX,
		PLATFORM_MAC,
		PLATFORM_WINDOWS,
		PLATFORM_IOS,
		PLATFORM_ANDROID,
	};

	enum HandlerState {
		STATE_UNINITIALIZED = 0,
		STATE_INSTALLED,
		STATE_ERROR,
	};

	// Initialize crash handler with the given dump directory.
	// If empty, uses OS::get_user_data_dir() + "/crash_reports".
	bool install(const String &p_dump_path = "");

	// Uninstall the crash handler.
	void uninstall();

	// Manually write a minidump (for debugging / non-crash situations).
	bool write_minidump();

	// Annotation management: key-value metadata included in crash reports.
	void set_annotation(const String &p_key, const String &p_value);
	void remove_annotation(const String &p_key);
	String get_annotation(const String &p_key) const;
	Dictionary get_annotations() const;
	void clear_annotations();

	// Auto-populate annotations with Godot engine info.
	void set_engine_annotations();

	// Product and version metadata (some platforms use these directly).
	void set_product_name(const String &p_name);
	String get_product_name() const;
	void set_product_version(const String &p_version);
	String get_product_version() const;

	// Query state.
	bool is_installed() const;
	Platform get_platform() const;
	String get_platform_name() const;
	String get_dump_path() const;
	HandlerState get_state() const;

	// Crash report history (persisted dumps found on disk).
	int get_crash_report_count() const;
	Array get_crash_reports() const;
	bool delete_crash_report(const String &p_dump_path);
	bool delete_all_crash_reports();

	// Signal: emitted after a crash dump is written (if possible).
	// In practice this fires only for write_minidump(), not real crashes.

	GdBreakpad();
	~GdBreakpad();

protected:
	static void _bind_methods();

private:
	BreakpadPlatformHandler *handler;
	CrashAnnotations annotations;
	String dump_path;
	String product_name;
	String product_version;
	HandlerState state;

	// Platform-specific init/teardown (implemented per-platform).
	bool platform_install(const String &p_dump_path);
	void platform_uninstall();
	bool platform_write_minidump();

	String ensure_dump_directory(const String &p_path) const;
	Vector<String> scan_dump_files() const;
};

VARIANT_ENUM_CAST(GdBreakpad::Platform);
VARIANT_ENUM_CAST(GdBreakpad::HandlerState);

#endif // GD_BREAKPAD_H
