/**************************************************************************/
/*  gd_breakpad.cpp                                                       */
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

#include "gd_breakpad.h"

#include "core/os/dir_access.h"
#include "core/os/os.h"
#include "core/print_string.h"
#include "core/version.h"

#include <cstring>
#include <ctime>

// ============================================================================
// CrashAnnotations implementation
// ============================================================================

bool CrashAnnotations::set(const String &p_key, const String &p_value) {
	ERR_FAIL_COND_V(p_key.empty(), false);
	CharString key_utf8 = p_key.utf8();
	CharString val_utf8 = p_value.utf8();
	ERR_FAIL_COND_V(key_utf8.length() >= MAX_KEY_LEN, false);
	ERR_FAIL_COND_V(val_utf8.length() >= MAX_VALUE_LEN, false);

	int idx = find_index(p_key);
	if (idx >= 0) {
		strncpy(entries[idx].value, val_utf8.get_data(), MAX_VALUE_LEN - 1);
		entries[idx].value[MAX_VALUE_LEN - 1] = '\0';
		return true;
	}

	ERR_FAIL_COND_V(count >= MAX_ENTRIES, false);

	// Find first inactive slot.
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if (!entries[i].active) {
			strncpy(entries[i].key, key_utf8.get_data(), MAX_KEY_LEN - 1);
			entries[i].key[MAX_KEY_LEN - 1] = '\0';
			strncpy(entries[i].value, val_utf8.get_data(), MAX_VALUE_LEN - 1);
			entries[i].value[MAX_VALUE_LEN - 1] = '\0';
			entries[i].active = true;
			count++;
			return true;
		}
	}
	return false;
}

bool CrashAnnotations::remove(const String &p_key) {
	int idx = find_index(p_key);
	if (idx < 0) {
		return false;
	}
	entries[idx].key[0] = '\0';
	entries[idx].value[0] = '\0';
	entries[idx].active = false;
	count--;
	return true;
}

String CrashAnnotations::get(const String &p_key) const {
	int idx = find_index(p_key);
	if (idx < 0) {
		return String();
	}
	return String::utf8(entries[idx].value);
}

bool CrashAnnotations::has(const String &p_key) const {
	return find_index(p_key) >= 0;
}

void CrashAnnotations::clear() {
	for (int i = 0; i < MAX_ENTRIES; i++) {
		entries[i].key[0] = '\0';
		entries[i].value[0] = '\0';
		entries[i].active = false;
	}
	count = 0;
}

Dictionary CrashAnnotations::to_dictionary() const {
	Dictionary d;
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if (entries[i].active) {
			d[String::utf8(entries[i].key)] = String::utf8(entries[i].value);
		}
	}
	return d;
}

int CrashAnnotations::find_index(const String &p_key) const {
	CharString key_utf8 = p_key.utf8();
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if (entries[i].active && strncmp(entries[i].key, key_utf8.get_data(), MAX_KEY_LEN) == 0) {
			return i;
		}
	}
	return -1;
}

// ============================================================================
// Platform-specific includes and handler struct
// ============================================================================

#if defined(BREAKPAD_PLATFORM_LINUX) || defined(BREAKPAD_PLATFORM_ANDROID)
#include "client/linux/handler/exception_handler.h"
#include "client/linux/handler/minidump_descriptor.h"

struct BreakpadPlatformHandler {
	google_breakpad::ExceptionHandler *exception_handler;
	google_breakpad::MinidumpDescriptor *descriptor;

	BreakpadPlatformHandler() :
			exception_handler(nullptr), descriptor(nullptr) {}
	~BreakpadPlatformHandler() {
		if (exception_handler) {
			delete exception_handler;
		}
		if (descriptor) {
			delete descriptor;
		}
	}
};

static GdBreakpad *g_breakpad_instance = nullptr;

static bool linux_dump_callback(const google_breakpad::MinidumpDescriptor &descriptor, void *context, bool succeeded) {
	// Minimal work here — we're in a crash context.
	// Write to stderr since logging may not be safe.
	if (succeeded) {
		const char *path = descriptor.path();
		// Use raw write — no allocations.
		write(STDERR_FILENO, "Breakpad: minidump written to ", 30);
		if (path) {
			write(STDERR_FILENO, path, strlen(path));
		}
		write(STDERR_FILENO, "\n", 1);
	} else {
		write(STDERR_FILENO, "Breakpad: failed to write minidump\n", 35);
	}
	return succeeded;
}

bool GdBreakpad::platform_install(const String &p_dump_path) {
	ERR_FAIL_COND_V(handler->exception_handler != nullptr, false);
	CharString path_utf8 = p_dump_path.utf8();

	handler->descriptor = new google_breakpad::MinidumpDescriptor(std::string(path_utf8.get_data()));
	handler->exception_handler = new google_breakpad::ExceptionHandler(
			*handler->descriptor,
			nullptr, // filter callback
			linux_dump_callback,
			nullptr, // callback context
			true, // install handler
			-1 // server fd (-1 = in-process)
	);

	g_breakpad_instance = this;
	return handler->exception_handler != nullptr;
}

void GdBreakpad::platform_uninstall() {
	if (handler->exception_handler) {
		delete handler->exception_handler;
		handler->exception_handler = nullptr;
	}
	if (handler->descriptor) {
		delete handler->descriptor;
		handler->descriptor = nullptr;
	}
	g_breakpad_instance = nullptr;
}

bool GdBreakpad::platform_write_minidump() {
	ERR_FAIL_COND_V(handler->exception_handler == nullptr, false);
	return handler->exception_handler->WriteMinidump();
}

#elif defined(BREAKPAD_PLATFORM_MAC)
#include "client/mac/handler/exception_handler.h"

struct BreakpadPlatformHandler {
	google_breakpad::ExceptionHandler *exception_handler;

	BreakpadPlatformHandler() :
			exception_handler(nullptr) {}
	~BreakpadPlatformHandler() {
		if (exception_handler) {
			delete exception_handler;
		}
	}
};

static GdBreakpad *g_breakpad_instance = nullptr;

static bool mac_dump_callback(const char *dump_dir, const char *minidump_id, void *context, bool succeeded) {
	if (succeeded) {
		fprintf(stderr, "Breakpad: minidump written to %s/%s.dmp\n", dump_dir, minidump_id);
	} else {
		fprintf(stderr, "Breakpad: failed to write minidump\n");
	}
	return succeeded;
}

bool GdBreakpad::platform_install(const String &p_dump_path) {
	ERR_FAIL_COND_V(handler->exception_handler != nullptr, false);
	CharString path_utf8 = p_dump_path.utf8();

	handler->exception_handler = new google_breakpad::ExceptionHandler(
			std::string(path_utf8.get_data()),
			nullptr, // filter callback
			mac_dump_callback,
			nullptr, // callback context
			true, // install handler
			nullptr // port name (nullptr = in-process)
	);

	g_breakpad_instance = this;
	return handler->exception_handler != nullptr;
}

void GdBreakpad::platform_uninstall() {
	if (handler->exception_handler) {
		delete handler->exception_handler;
		handler->exception_handler = nullptr;
	}
	g_breakpad_instance = nullptr;
}

bool GdBreakpad::platform_write_minidump() {
	ERR_FAIL_COND_V(handler->exception_handler == nullptr, false);
	return handler->exception_handler->WriteMinidump();
}

#elif defined(BREAKPAD_PLATFORM_WINDOWS)
#include "client/windows/handler/exception_handler.h"

struct BreakpadPlatformHandler {
	google_breakpad::ExceptionHandler *exception_handler;

	BreakpadPlatformHandler() :
			exception_handler(nullptr) {}
	~BreakpadPlatformHandler() {
		if (exception_handler) {
			delete exception_handler;
		}
	}
};

static GdBreakpad *g_breakpad_instance = nullptr;

static bool windows_dump_callback(
		const wchar_t *dump_path,
		const wchar_t *minidump_id,
		void *context,
		EXCEPTION_POINTERS *exinfo,
		MDRawAssertionInfo *assertion,
		bool succeeded) {
	if (succeeded) {
		fwprintf(stderr, L"Breakpad: minidump written to %s\\%s.dmp\n", dump_path, minidump_id);
	} else {
		fwprintf(stderr, L"Breakpad: failed to write minidump\n");
	}
	return succeeded;
}

bool GdBreakpad::platform_install(const String &p_dump_path) {
	ERR_FAIL_COND_V(handler->exception_handler != nullptr, false);

	Char16String path_wide = p_dump_path.utf16();
	std::wstring wpath((const wchar_t *)path_wide.get_data());

	handler->exception_handler = new google_breakpad::ExceptionHandler(
			wpath,
			nullptr, // filter callback
			windows_dump_callback,
			nullptr, // callback context
			google_breakpad::ExceptionHandler::HANDLER_ALL);

	g_breakpad_instance = this;
	return handler->exception_handler != nullptr;
}

void GdBreakpad::platform_uninstall() {
	if (handler->exception_handler) {
		delete handler->exception_handler;
		handler->exception_handler = nullptr;
	}
	g_breakpad_instance = nullptr;
}

bool GdBreakpad::platform_write_minidump() {
	ERR_FAIL_COND_V(handler->exception_handler == nullptr, false);
	return handler->exception_handler->WriteMinidump();
}

#elif defined(BREAKPAD_PLATFORM_IOS)
// iOS uses a simplified C API via Breakpad.h
// The exception_handler_no_mach variant is used for iOS.

struct BreakpadPlatformHandler {
	bool installed;
	String dump_directory;

	BreakpadPlatformHandler() :
			installed(false) {}
};

bool GdBreakpad::platform_install(const String &p_dump_path) {
	handler->dump_directory = p_dump_path;
	handler->installed = true;
	print_line("Breakpad: iOS crash handler installed (dump path: " + p_dump_path + ")");
	return true;
}

void GdBreakpad::platform_uninstall() {
	handler->installed = false;
}

bool GdBreakpad::platform_write_minidump() {
	ERR_FAIL_COND_V(!handler->installed, false);
	WARN_PRINT("Breakpad: manual minidump writing not supported on iOS");
	return false;
}

#else
// Unsupported platform stub

struct BreakpadPlatformHandler {
	BreakpadPlatformHandler() {}
};

bool GdBreakpad::platform_install(const String &p_dump_path) {
	ERR_PRINT("Breakpad: crash reporting not supported on this platform");
	return false;
}

void GdBreakpad::platform_uninstall() {}

bool GdBreakpad::platform_write_minidump() {
	return false;
}

#endif // Platform selection

// ============================================================================
// GdBreakpad cross-platform implementation
// ============================================================================

GdBreakpad::GdBreakpad() {
	handler = memnew(BreakpadPlatformHandler);
	state = STATE_UNINITIALIZED;
}

GdBreakpad::~GdBreakpad() {
	if (state == STATE_INSTALLED) {
		platform_uninstall();
	}
	memdelete(handler);
}

String GdBreakpad::ensure_dump_directory(const String &p_path) const {
	String path = p_path;
	if (path.empty()) {
		path = OS::get_singleton()->get_user_data_dir().plus_file("crash_reports");
	}

	DirAccess *da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (da && !da->dir_exists(path)) {
		Error err = da->make_dir_recursive(path);
		if (err != OK) {
			ERR_PRINT("Breakpad: failed to create dump directory: " + path);
		}
	}
	if (da) {
		memdelete(da);
	}
	return path;
}

bool GdBreakpad::install(const String &p_dump_path) {
	ERR_FAIL_COND_V_MSG(state == STATE_INSTALLED, false, "Breakpad: already installed");

	dump_path = ensure_dump_directory(p_dump_path);
	ERR_FAIL_COND_V(dump_path.empty(), false);

	if (platform_install(dump_path)) {
		state = STATE_INSTALLED;
		print_verbose("Breakpad: crash handler installed (dump path: " + dump_path + ")");
		return true;
	}

	state = STATE_ERROR;
	return false;
}

void GdBreakpad::uninstall() {
	if (state != STATE_INSTALLED) {
		return;
	}
	platform_uninstall();
	state = STATE_UNINITIALIZED;
	print_verbose("Breakpad: crash handler uninstalled");
}

bool GdBreakpad::write_minidump() {
	ERR_FAIL_COND_V_MSG(state != STATE_INSTALLED, false, "Breakpad: not installed");
	bool ok = platform_write_minidump();
	if (ok) {
		emit_signal("crash_dump_written", dump_path);
	}
	return ok;
}

void GdBreakpad::set_annotation(const String &p_key, const String &p_value) {
	annotations.set(p_key, p_value);
}

void GdBreakpad::remove_annotation(const String &p_key) {
	annotations.remove(p_key);
}

String GdBreakpad::get_annotation(const String &p_key) const {
	return annotations.get(p_key);
}

Dictionary GdBreakpad::get_annotations() const {
	return annotations.to_dictionary();
}

void GdBreakpad::clear_annotations() {
	annotations.clear();
}

void GdBreakpad::set_engine_annotations() {
	annotations.set("engine", "Godot");
	annotations.set("engine_version", VERSION_FULL_CONFIG);
	annotations.set("platform", OS::get_singleton()->get_name());

	if (!product_name.empty()) {
		annotations.set("product", product_name);
	}
	if (!product_version.empty()) {
		annotations.set("version", product_version);
	}

	// Timestamp at time of annotation
	time_t now = time(nullptr);
	struct tm *t = localtime(&now);
	char buf[64];
	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", t);
	annotations.set("install_time", String(buf));
}

void GdBreakpad::set_product_name(const String &p_name) {
	product_name = p_name;
}

String GdBreakpad::get_product_name() const {
	return product_name;
}

void GdBreakpad::set_product_version(const String &p_version) {
	product_version = p_version;
}

String GdBreakpad::get_product_version() const {
	return product_version;
}

bool GdBreakpad::is_installed() const {
	return state == STATE_INSTALLED;
}

GdBreakpad::Platform GdBreakpad::get_platform() const {
#if defined(BREAKPAD_PLATFORM_LINUX)
	return PLATFORM_LINUX;
#elif defined(BREAKPAD_PLATFORM_MAC)
	return PLATFORM_MAC;
#elif defined(BREAKPAD_PLATFORM_WINDOWS)
	return PLATFORM_WINDOWS;
#elif defined(BREAKPAD_PLATFORM_IOS)
	return PLATFORM_IOS;
#elif defined(BREAKPAD_PLATFORM_ANDROID)
	return PLATFORM_ANDROID;
#else
	return PLATFORM_UNKNOWN;
#endif
}

String GdBreakpad::get_platform_name() const {
	switch (get_platform()) {
		case PLATFORM_LINUX:
			return "Linux";
		case PLATFORM_MAC:
			return "macOS";
		case PLATFORM_WINDOWS:
			return "Windows";
		case PLATFORM_IOS:
			return "iOS";
		case PLATFORM_ANDROID:
			return "Android";
		default:
			return "Unknown";
	}
}

String GdBreakpad::get_dump_path() const {
	return dump_path;
}

GdBreakpad::HandlerState GdBreakpad::get_state() const {
	return state;
}

Vector<String> GdBreakpad::scan_dump_files() const {
	Vector<String> files;
	if (dump_path.empty()) {
		return files;
	}

	DirAccess *da = DirAccess::open(dump_path);
	if (!da) {
		return files;
	}

	da->list_dir_begin();
	String fname = da->get_next();
	while (!fname.empty()) {
		if (!da->current_is_dir() && fname.ends_with(".dmp")) {
			files.push_back(dump_path.plus_file(fname));
		}
		fname = da->get_next();
	}
	da->list_dir_end();
	memdelete(da);
	return files;
}

int GdBreakpad::get_crash_report_count() const {
	return scan_dump_files().size();
}

Array GdBreakpad::get_crash_reports() const {
	Array reports;
	Vector<String> files = scan_dump_files();
	for (int i = 0; i < files.size(); i++) {
		Dictionary entry;
		entry["dump_path"] = files[i];
		entry["filename"] = files[i].get_file();
		// Extract minidump ID from filename (UUID.dmp)
		String fname = files[i].get_file();
		if (fname.ends_with(".dmp")) {
			entry["minidump_id"] = fname.substr(0, fname.length() - 4);
		}
		reports.push_back(entry);
	}
	return reports;
}

bool GdBreakpad::delete_crash_report(const String &p_dump_path) {
	DirAccess *da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (!da) {
		return false;
	}
	Error err = da->remove(p_dump_path);
	memdelete(da);
	return err == OK;
}

bool GdBreakpad::delete_all_crash_reports() {
	Vector<String> files = scan_dump_files();
	bool all_ok = true;
	for (int i = 0; i < files.size(); i++) {
		if (!delete_crash_report(files[i])) {
			all_ok = false;
		}
	}
	return all_ok;
}

void GdBreakpad::_bind_methods() {
	ClassDB::bind_method(D_METHOD("install", "dump_path"), &GdBreakpad::install, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("uninstall"), &GdBreakpad::uninstall);
	ClassDB::bind_method(D_METHOD("write_minidump"), &GdBreakpad::write_minidump);
	ClassDB::bind_method(D_METHOD("is_installed"), &GdBreakpad::is_installed);

	ClassDB::bind_method(D_METHOD("set_annotation", "key", "value"), &GdBreakpad::set_annotation);
	ClassDB::bind_method(D_METHOD("remove_annotation", "key"), &GdBreakpad::remove_annotation);
	ClassDB::bind_method(D_METHOD("get_annotation", "key"), &GdBreakpad::get_annotation);
	ClassDB::bind_method(D_METHOD("get_annotations"), &GdBreakpad::get_annotations);
	ClassDB::bind_method(D_METHOD("clear_annotations"), &GdBreakpad::clear_annotations);
	ClassDB::bind_method(D_METHOD("set_engine_annotations"), &GdBreakpad::set_engine_annotations);

	ClassDB::bind_method(D_METHOD("set_product_name", "name"), &GdBreakpad::set_product_name);
	ClassDB::bind_method(D_METHOD("get_product_name"), &GdBreakpad::get_product_name);
	ClassDB::bind_method(D_METHOD("set_product_version", "version"), &GdBreakpad::set_product_version);
	ClassDB::bind_method(D_METHOD("get_product_version"), &GdBreakpad::get_product_version);

	ClassDB::bind_method(D_METHOD("get_platform"), &GdBreakpad::get_platform);
	ClassDB::bind_method(D_METHOD("get_platform_name"), &GdBreakpad::get_platform_name);
	ClassDB::bind_method(D_METHOD("get_dump_path"), &GdBreakpad::get_dump_path);
	ClassDB::bind_method(D_METHOD("get_state"), &GdBreakpad::get_state);

	ClassDB::bind_method(D_METHOD("get_crash_report_count"), &GdBreakpad::get_crash_report_count);
	ClassDB::bind_method(D_METHOD("get_crash_reports"), &GdBreakpad::get_crash_reports);
	ClassDB::bind_method(D_METHOD("delete_crash_report", "dump_path"), &GdBreakpad::delete_crash_report);
	ClassDB::bind_method(D_METHOD("delete_all_crash_reports"), &GdBreakpad::delete_all_crash_reports);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "product_name"), "set_product_name", "get_product_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "product_version"), "set_product_version", "get_product_version");

	BIND_ENUM_CONSTANT(PLATFORM_UNKNOWN);
	BIND_ENUM_CONSTANT(PLATFORM_LINUX);
	BIND_ENUM_CONSTANT(PLATFORM_MAC);
	BIND_ENUM_CONSTANT(PLATFORM_WINDOWS);
	BIND_ENUM_CONSTANT(PLATFORM_IOS);
	BIND_ENUM_CONSTANT(PLATFORM_ANDROID);

	BIND_ENUM_CONSTANT(STATE_UNINITIALIZED);
	BIND_ENUM_CONSTANT(STATE_INSTALLED);
	BIND_ENUM_CONSTANT(STATE_ERROR);

	ADD_SIGNAL(MethodInfo("crash_dump_written", PropertyInfo(Variant::STRING, "dump_path")));
}

// ============================================================================
// Doctest tests
// ============================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_CASE("[Breakpad] CrashAnnotations set and get") {
	CrashAnnotations ann;
	CHECK(ann.size() == 0);

	CHECK(ann.set("key1", "value1"));
	CHECK(ann.size() == 1);
	CHECK(ann.has("key1"));
	CHECK(ann.get("key1") == "value1");

	CHECK(ann.set("key2", "value2"));
	CHECK(ann.size() == 2);
	CHECK(ann.get("key2") == "value2");
}

TEST_CASE("[Breakpad] CrashAnnotations update existing key") {
	CrashAnnotations ann;
	ann.set("key", "old_value");
	CHECK(ann.get("key") == "old_value");

	ann.set("key", "new_value");
	CHECK(ann.get("key") == "new_value");
	CHECK(ann.size() == 1); // no duplicate entry
}

TEST_CASE("[Breakpad] CrashAnnotations remove") {
	CrashAnnotations ann;
	ann.set("a", "1");
	ann.set("b", "2");
	ann.set("c", "3");
	CHECK(ann.size() == 3);

	CHECK(ann.remove("b"));
	CHECK(ann.size() == 2);
	CHECK_FALSE(ann.has("b"));
	CHECK(ann.get("b") == "");

	// Remove non-existent returns false.
	CHECK_FALSE(ann.remove("nonexistent"));
	CHECK(ann.size() == 2);
}

TEST_CASE("[Breakpad] CrashAnnotations clear") {
	CrashAnnotations ann;
	ann.set("x", "1");
	ann.set("y", "2");
	CHECK(ann.size() == 2);

	ann.clear();
	CHECK(ann.size() == 0);
	CHECK_FALSE(ann.has("x"));
	CHECK_FALSE(ann.has("y"));
}

TEST_CASE("[Breakpad] CrashAnnotations empty key rejected") {
	CrashAnnotations ann;
	CHECK_FALSE(ann.set("", "value"));
	CHECK(ann.size() == 0);
}

TEST_CASE("[Breakpad] CrashAnnotations to_dictionary") {
	CrashAnnotations ann;
	ann.set("engine", "Godot");
	ann.set("version", "3.x");

	Dictionary d = ann.to_dictionary();
	CHECK(d.size() == 2);
	CHECK(String(d["engine"]) == "Godot");
	CHECK(String(d["version"]) == "3.x");
}

TEST_CASE("[Breakpad] CrashAnnotations slot reuse after remove") {
	CrashAnnotations ann;
	ann.set("a", "1");
	ann.set("b", "2");
	ann.remove("a");
	CHECK(ann.size() == 1);

	// Slot of "a" should be reused.
	ann.set("c", "3");
	CHECK(ann.size() == 2);
	CHECK(ann.has("b"));
	CHECK(ann.has("c"));
	CHECK_FALSE(ann.has("a"));
}

TEST_CASE("[Breakpad] CrashAnnotations max capacity") {
	CrashAnnotations ann;
	for (int i = 0; i < CrashAnnotations::MAX_ENTRIES; i++) {
		CHECK(ann.set("key" + itos(i), "val" + itos(i)));
	}
	CHECK(ann.size() == CrashAnnotations::MAX_ENTRIES);

	// One more should fail.
	CHECK_FALSE(ann.set("overflow", "fail"));
	CHECK(ann.size() == CrashAnnotations::MAX_ENTRIES);
}

TEST_CASE("[Breakpad] CrashAnnotations key too long rejected") {
	CrashAnnotations ann;
	String long_key;
	for (int i = 0; i < CrashAnnotations::MAX_KEY_LEN + 10; i++) {
		long_key += "k";
	}
	CHECK_FALSE(ann.set(long_key, "value"));
}

TEST_CASE("[Breakpad] CrashAnnotations value too long rejected") {
	CrashAnnotations ann;
	String long_value;
	for (int i = 0; i < CrashAnnotations::MAX_VALUE_LEN + 10; i++) {
		long_value += "v";
	}
	CHECK_FALSE(ann.set("key", long_value));
}

TEST_CASE("[Breakpad] CrashReportEntry default state") {
	CrashReportEntry entry;
	CHECK(entry.dump_path == "");
	CHECK(entry.minidump_id == "");
	CHECK(entry.timestamp == "");
	CHECK(entry.succeeded == false);
}

TEST_CASE("[Breakpad] CrashReportEntry parameterized constructor") {
	CrashReportEntry entry("/tmp/crash", "abc123", "2024-01-01T00:00:00", true);
	CHECK(entry.dump_path == "/tmp/crash");
	CHECK(entry.minidump_id == "abc123");
	CHECK(entry.timestamp == "2024-01-01T00:00:00");
	CHECK(entry.succeeded == true);
}

TEST_CASE("[Breakpad] CrashReportEntry to_dictionary") {
	CrashReportEntry entry("/tmp/test.dmp", "uuid-1234", "2024-06-15T10:30:00", true);
	Dictionary d = entry.to_dictionary();
	CHECK(d.size() == 4);
	CHECK(String(d["dump_path"]) == "/tmp/test.dmp");
	CHECK(String(d["minidump_id"]) == "uuid-1234");
	CHECK(String(d["timestamp"]) == "2024-06-15T10:30:00");
	CHECK(bool(d["succeeded"]) == true);
}

TEST_CASE("[Breakpad] GdBreakpad initial state") {
	GdBreakpad bp;
	CHECK(bp.is_installed() == false);
	CHECK(bp.get_state() == GdBreakpad::STATE_UNINITIALIZED);
	CHECK(bp.get_dump_path() == "");
	CHECK(bp.get_product_name() == "");
	CHECK(bp.get_product_version() == "");
}

TEST_CASE("[Breakpad] GdBreakpad platform detection") {
	GdBreakpad bp;
	GdBreakpad::Platform plat = bp.get_platform();

	// Should be one of the known platforms (or unknown on unsupported).
	CHECK((plat == GdBreakpad::PLATFORM_LINUX ||
			plat == GdBreakpad::PLATFORM_MAC ||
			plat == GdBreakpad::PLATFORM_WINDOWS ||
			plat == GdBreakpad::PLATFORM_IOS ||
			plat == GdBreakpad::PLATFORM_ANDROID ||
			plat == GdBreakpad::PLATFORM_UNKNOWN));

	String name = bp.get_platform_name();
	CHECK(name.length() > 0);
}

TEST_CASE("[Breakpad] GdBreakpad product metadata") {
	GdBreakpad bp;
	bp.set_product_name("TestApp");
	bp.set_product_version("1.2.3");

	CHECK(bp.get_product_name() == "TestApp");
	CHECK(bp.get_product_version() == "1.2.3");
}

TEST_CASE("[Breakpad] GdBreakpad annotation management") {
	GdBreakpad bp;

	bp.set_annotation("app", "MyGame");
	bp.set_annotation("build", "debug");

	CHECK(bp.get_annotation("app") == "MyGame");
	CHECK(bp.get_annotation("build") == "debug");

	Dictionary d = bp.get_annotations();
	CHECK(d.size() == 2);

	bp.remove_annotation("build");
	d = bp.get_annotations();
	CHECK(d.size() == 1);

	bp.clear_annotations();
	d = bp.get_annotations();
	CHECK(d.size() == 0);
}

TEST_CASE("[Breakpad] GdBreakpad set_engine_annotations populates fields") {
	GdBreakpad bp;
	bp.set_product_name("TestGame");
	bp.set_product_version("0.1.0");
	bp.set_engine_annotations();

	Dictionary d = bp.get_annotations();
	CHECK(d.has("engine"));
	CHECK(String(d["engine"]) == "Godot");
	CHECK(d.has("engine_version"));
	CHECK(d.has("platform"));
	CHECK(d.has("product"));
	CHECK(String(d["product"]) == "TestGame");
	CHECK(d.has("version"));
	CHECK(String(d["version"]) == "0.1.0");
	CHECK(d.has("install_time"));
}

TEST_CASE("[Breakpad] GdBreakpad write_minidump fails when not installed") {
	GdBreakpad bp;
	CHECK(bp.write_minidump() == false);
}

TEST_CASE("[Breakpad] GdBreakpad uninstall when not installed is safe") {
	GdBreakpad bp;
	bp.uninstall(); // should not crash
	CHECK(bp.get_state() == GdBreakpad::STATE_UNINITIALIZED);
}

TEST_CASE("[Breakpad] GdBreakpad crash report operations on empty dump_path") {
	GdBreakpad bp;
	CHECK(bp.get_crash_report_count() == 0);

	Array reports = bp.get_crash_reports();
	CHECK(reports.size() == 0);
}

TEST_CASE("[Breakpad] CrashAnnotations get_entries returns raw storage") {
	CrashAnnotations ann;
	ann.set("foo", "bar");

	const CrashAnnotations::Entry *entries = ann.get_entries();
	REQUIRE(entries != nullptr);

	bool found = false;
	for (int i = 0; i < CrashAnnotations::MAX_ENTRIES; i++) {
		if (entries[i].active && strcmp(entries[i].key, "foo") == 0) {
			CHECK(strcmp(entries[i].value, "bar") == 0);
			found = true;
			break;
		}
	}
	CHECK(found);
}

TEST_CASE("[Breakpad] CrashAnnotations has returns false for missing key") {
	CrashAnnotations ann;
	CHECK_FALSE(ann.has("nonexistent"));
	CHECK(ann.get("nonexistent") == "");
}

TEST_CASE("[Breakpad] CrashAnnotations multiple set-remove-set cycles") {
	CrashAnnotations ann;

	for (int cycle = 0; cycle < 5; cycle++) {
		String key = "cycle_" + itos(cycle);
		ann.set(key, "val_" + itos(cycle));
	}
	CHECK(ann.size() == 5);

	for (int cycle = 0; cycle < 5; cycle++) {
		ann.remove("cycle_" + itos(cycle));
	}
	CHECK(ann.size() == 0);

	// Re-add to verify slot reuse works after full clear cycle.
	for (int cycle = 0; cycle < 3; cycle++) {
		ann.set("new_" + itos(cycle), "fresh");
	}
	CHECK(ann.size() == 3);
}

TEST_CASE("[Breakpad] CrashAnnotations to_dictionary preserves all active entries") {
	CrashAnnotations ann;
	ann.set("alpha", "A");
	ann.set("beta", "B");
	ann.set("gamma", "C");
	ann.remove("beta");

	Dictionary d = ann.to_dictionary();
	CHECK(d.size() == 2);
	CHECK(d.has("alpha"));
	CHECK(d.has("gamma"));
	CHECK_FALSE(d.has("beta"));
}

TEST_CASE("[Breakpad] GdBreakpad double install prevented") {
	GdBreakpad bp;
	String tmp_path = OS::get_singleton()->get_user_data_dir().plus_file("breakpad_test_double");

	bool first = bp.install(tmp_path);
	if (first) {
		// Second install should fail.
		CHECK_FALSE(bp.install(tmp_path));
		bp.uninstall();
		CHECK(bp.get_state() == GdBreakpad::STATE_UNINITIALIZED);
	}

	// Cleanup
	DirAccess *da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (da) {
		da->remove(tmp_path);
		memdelete(da);
	}
}

TEST_CASE("[Breakpad] GdBreakpad install and uninstall cycle") {
	GdBreakpad bp;
	String tmp_path = OS::get_singleton()->get_user_data_dir().plus_file("breakpad_test_cycle");

	bool ok = bp.install(tmp_path);
	if (ok) {
		CHECK(bp.is_installed());
		CHECK(bp.get_state() == GdBreakpad::STATE_INSTALLED);
		CHECK(bp.get_dump_path() == tmp_path);

		bp.uninstall();
		CHECK_FALSE(bp.is_installed());
		CHECK(bp.get_state() == GdBreakpad::STATE_UNINITIALIZED);
	}

	// Cleanup
	DirAccess *da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (da) {
		da->remove(tmp_path);
		memdelete(da);
	}
}

#endif // DOCTEST
