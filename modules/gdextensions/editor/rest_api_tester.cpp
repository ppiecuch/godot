/**************************************************************************/
/*  rest_api_tester.cpp                                                   */
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

#ifdef TOOLS_ENABLED

#include "rest_api_tester.h"

#include "core/crypto/crypto_core.h"
#include "core/os/file_access.h"
#include "core/os/os.h"

// =========================================================================
// Utility functions
// =========================================================================

String rest_build_auth_header(int p_auth_type, const String &p_token, const String &p_user, const String &p_pass, const String &p_key_name, const String &p_key_value) {
	switch (p_auth_type) {
		case 1: { // Bearer
			if (!p_token.empty()) {
				return "Authorization: Bearer " + p_token;
			}
		} break;
		case 2: { // Basic
			if (!p_user.empty()) {
				CharString creds = (p_user + ":" + p_pass).utf8();
				size_t b64_len = 0;
				CryptoCore::b64_encode(nullptr, 0, &b64_len, (const uint8_t *)creds.get_data(), creds.length());
				Vector<uint8_t> b64;
				b64.resize(b64_len + 1);
				CryptoCore::b64_encode(b64.ptrw(), b64_len, &b64_len, (const uint8_t *)creds.get_data(), creds.length());
				b64.write[b64_len] = 0;
				return "Authorization: Basic " + String((const char *)b64.ptr());
			}
		} break;
		case 3: { // API Key
			if (!p_key_name.empty()) {
				return p_key_name + ": " + p_key_value;
			}
		} break;
	}
	return "";
}

String rest_ensure_url_scheme(const String &p_url) {
	String url = p_url.strip_edges();
	if (url.empty()) {
		return url;
	}
	if (!url.begins_with("http://") && !url.begins_with("https://")) {
		return "https://" + url;
	}
	return url;
}

String rest_pretty_print_json(const String &p_json) {
	Variant parsed;
	String err_str;
	int err_line;
	if (JSON::parse(p_json, parsed, err_str, err_line) == OK) {
		return JSON::print(parsed, "\t");
	}
	return p_json;
}

Dictionary rest_serialize_request(const String &p_method, const String &p_url,
		const Vector<Pair<String, String>> &p_headers, const String &p_body,
		int p_auth_type, const String &p_auth_token, const String &p_auth_user,
		const String &p_auth_pass, const String &p_auth_key_name, const String &p_auth_key_value) {
	Dictionary d;
	d["method"] = p_method;
	d["url"] = p_url;
	d["body"] = p_body;
	d["auth_type"] = p_auth_type;
	d["auth_token"] = p_auth_token;
	d["auth_user"] = p_auth_user;
	d["auth_pass"] = p_auth_pass;
	d["auth_key_name"] = p_auth_key_name;
	d["auth_key_value"] = p_auth_key_value;

	Array headers_arr;
	for (int i = 0; i < p_headers.size(); i++) {
		Array pair;
		pair.push_back(p_headers[i].first);
		pair.push_back(p_headers[i].second);
		headers_arr.push_back(pair);
	}
	d["headers"] = headers_arr;
	return d;
}

// =========================================================================
// RestApiTesterDock — UI Construction
// =========================================================================

void RestApiTesterDock::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			http_request = memnew(HTTPRequest);
			http_request->set_name("HTTPReq");
			http_request->set_use_threads(true);
			http_request->set_timeout(30);
			add_child(http_request);
			http_request->connect("request_completed", this, "_on_request_completed");
		} break;
		case NOTIFICATION_RESIZED: {
			_update_layout();
		} break;
	}
}

void RestApiTesterDock::_update_layout() {
	bool want_horizontal = get_size().x > get_size().y;
	if (want_horizontal == is_horizontal_layout && split_container) {
		return;
	}
	is_horizontal_layout = want_horizontal;

	// Reparent panels into new split container
	if (split_container) {
		split_container->remove_child(request_panel);
		split_container->remove_child(response_panel);
		remove_child(split_container);
		memdelete(split_container);
	}

	if (is_horizontal_layout) {
		HSplitContainer *hsplit = memnew(HSplitContainer);
		split_container = hsplit;
	} else {
		VSplitContainer *vsplit = memnew(VSplitContainer);
		split_container = vsplit;
	}
	split_container->set_v_size_flags(SIZE_EXPAND_FILL);
	split_container->set_h_size_flags(SIZE_EXPAND_FILL);
	split_container->add_child(request_panel);
	split_container->add_child(response_panel);
	add_child(split_container);
}

void RestApiTesterDock::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_send_pressed"), &RestApiTesterDock::_on_send_pressed);
	ClassDB::bind_method(D_METHOD("_on_request_completed", "result", "code", "headers", "body"), &RestApiTesterDock::_on_request_completed);
	ClassDB::bind_method(D_METHOD("_on_add_header_pressed"), &RestApiTesterDock::_on_add_header_pressed);
	ClassDB::bind_method(D_METHOD("_on_auth_type_changed", "idx"), &RestApiTesterDock::_on_auth_type_changed);
	ClassDB::bind_method(D_METHOD("_on_save_pressed"), &RestApiTesterDock::_on_save_pressed);
	ClassDB::bind_method(D_METHOD("_on_load_pressed"), &RestApiTesterDock::_on_load_pressed);
	ClassDB::bind_method(D_METHOD("_on_save_file_selected", "path"), &RestApiTesterDock::_on_save_file_selected);
	ClassDB::bind_method(D_METHOD("_on_load_file_selected", "path"), &RestApiTesterDock::_on_load_file_selected);
	ClassDB::bind_method(D_METHOD("_on_history_selected", "idx"), &RestApiTesterDock::_on_history_selected);
	ClassDB::bind_method(D_METHOD("_on_clear_history_pressed"), &RestApiTesterDock::_on_clear_history_pressed);
}

RestApiTesterDock::RestApiTesterDock() {
	request_start_time = 0;

	// ===================== URL BAR (full width top) =====================
	HBoxContainer *url_bar = memnew(HBoxContainer);

	method_option = memnew(OptionButton);
	method_option->add_item("GET", HTTPClient::METHOD_GET);
	method_option->add_item("POST", HTTPClient::METHOD_POST);
	method_option->add_item("PUT", HTTPClient::METHOD_PUT);
	method_option->add_item("DELETE", HTTPClient::METHOD_DELETE);
	method_option->add_item("PATCH", HTTPClient::METHOD_PATCH);
	method_option->add_item("HEAD", HTTPClient::METHOD_HEAD);
	method_option->add_item("OPTIONS", HTTPClient::METHOD_OPTIONS);
	method_option->set_custom_minimum_size(Size2(80, 0));
	url_bar->add_child(method_option);

	url_edit = memnew(LineEdit);
	url_edit->set_placeholder("https://api.example.com/endpoint");
	url_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	url_bar->add_child(url_edit);

	send_btn = memnew(Button);
	send_btn->set_text("Send");
	send_btn->connect("pressed", this, "_on_send_pressed");
	url_bar->add_child(send_btn);

	save_btn = memnew(Button);
	save_btn->set_text("Save");
	save_btn->connect("pressed", this, "_on_save_pressed");
	url_bar->add_child(save_btn);

	load_btn = memnew(Button);
	load_btn->set_text("Load");
	load_btn->connect("pressed", this, "_on_load_pressed");
	url_bar->add_child(load_btn);

	add_child(url_bar);

	// ===================== REQUEST PANEL =====================
	split_container = nullptr;
	is_horizontal_layout = true;

	request_panel = memnew(VBoxContainer);
	request_panel->set_h_size_flags(SIZE_EXPAND_FILL);
	request_panel->set_v_size_flags(SIZE_EXPAND_FILL);

	request_tabs = memnew(TabContainer);
	request_tabs->set_h_size_flags(SIZE_EXPAND_FILL);
	request_tabs->set_v_size_flags(SIZE_EXPAND_FILL);

	// --- Headers tab ---
	VBoxContainer *headers_tab = memnew(VBoxContainer);
	headers_tab->set_name("Headers");

	headers_tree = memnew(Tree);
	headers_tree->set_columns(2);
	headers_tree->set_column_titles_visible(true);
	headers_tree->set_column_title(0, "Key");
	headers_tree->set_column_title(1, "Value");
	headers_tree->set_column_expand(0, true);
	headers_tree->set_column_expand(1, true);
	headers_tree->set_column_min_width(0, 150);
	headers_tree->set_column_min_width(1, 200);
	headers_tree->set_v_size_flags(SIZE_EXPAND_FILL);
	headers_tree->set_hide_root(true);
	headers_tree->create_item(); // root
	headers_tab->add_child(headers_tree);

	add_header_btn = memnew(Button);
	add_header_btn->set_text("+ Add Header");
	add_header_btn->connect("pressed", this, "_on_add_header_pressed");
	headers_tab->add_child(add_header_btn);

	request_tabs->add_child(headers_tab);

	// --- Body tab ---
	VBoxContainer *body_tab = memnew(VBoxContainer);
	body_tab->set_name("Body");

	body_type_option = memnew(OptionButton);
	body_type_option->add_item("JSON");
	body_type_option->add_item("Form URL-Encoded");
	body_type_option->add_item("Raw Text");
	body_tab->add_child(body_type_option);

	body_edit = memnew(TextEdit);
	body_edit->set_v_size_flags(SIZE_EXPAND_FILL);
	body_edit->set_show_line_numbers(true);
	body_tab->add_child(body_edit);

	request_tabs->add_child(body_tab);

	// --- Auth tab ---
	VBoxContainer *auth_tab = memnew(VBoxContainer);
	auth_tab->set_name("Auth");

	auth_type_option = memnew(OptionButton);
	auth_type_option->add_item("None", 0);
	auth_type_option->add_item("Bearer Token", 1);
	auth_type_option->add_item("Basic Auth", 2);
	auth_type_option->add_item("API Key", 3);
	auth_type_option->connect("item_selected", this, "_on_auth_type_changed");
	auth_tab->add_child(auth_type_option);

	// Bearer container
	auth_bearer_container = memnew(VBoxContainer);
	auth_bearer_container->set_visible(false);
	Label *bearer_label = memnew(Label);
	bearer_label->set_text("Token:");
	auth_bearer_container->add_child(bearer_label);
	auth_token_edit = memnew(LineEdit);
	auth_token_edit->set_placeholder("Bearer token");
	auth_bearer_container->add_child(auth_token_edit);
	auth_tab->add_child(auth_bearer_container);

	// Basic container
	auth_basic_container = memnew(VBoxContainer);
	auth_basic_container->set_visible(false);
	Label *user_label = memnew(Label);
	user_label->set_text("Username:");
	auth_basic_container->add_child(user_label);
	auth_user_edit = memnew(LineEdit);
	auth_basic_container->add_child(auth_user_edit);
	Label *pass_label = memnew(Label);
	pass_label->set_text("Password:");
	auth_basic_container->add_child(pass_label);
	auth_pass_edit = memnew(LineEdit);
	auth_pass_edit->set_secret(true);
	auth_basic_container->add_child(auth_pass_edit);
	auth_tab->add_child(auth_basic_container);

	// API Key container
	auth_apikey_container = memnew(VBoxContainer);
	auth_apikey_container->set_visible(false);
	Label *keyname_label = memnew(Label);
	keyname_label->set_text("Header Name:");
	auth_apikey_container->add_child(keyname_label);
	auth_key_name_edit = memnew(LineEdit);
	auth_key_name_edit->set_placeholder("X-API-Key");
	auth_apikey_container->add_child(auth_key_name_edit);
	Label *keyval_label = memnew(Label);
	keyval_label->set_text("Value:");
	auth_apikey_container->add_child(keyval_label);
	auth_key_value_edit = memnew(LineEdit);
	auth_apikey_container->add_child(auth_key_value_edit);
	auth_tab->add_child(auth_apikey_container);

	request_tabs->add_child(auth_tab);
	request_panel->add_child(request_tabs);

	// ===================== RESPONSE PANEL =====================
	response_panel = memnew(VBoxContainer);
	response_panel->set_h_size_flags(SIZE_EXPAND_FILL);
	response_panel->set_v_size_flags(SIZE_EXPAND_FILL);

	status_label = memnew(Label);
	status_label->set_text("Ready");
	status_label->set_align(Label::ALIGN_CENTER);
	response_panel->add_child(status_label);

	response_tabs = memnew(TabContainer);
	response_tabs->set_v_size_flags(SIZE_EXPAND_FILL);

	// Response body
	response_body_edit = memnew(TextEdit);
	response_body_edit->set_name("Body");
	response_body_edit->set_readonly(true);
	response_body_edit->set_show_line_numbers(true);
	response_body_edit->set_v_size_flags(SIZE_EXPAND_FILL);
	response_tabs->add_child(response_body_edit);

	// Response headers
	response_headers_edit = memnew(TextEdit);
	response_headers_edit->set_name("Headers");
	response_headers_edit->set_readonly(true);
	response_headers_edit->set_v_size_flags(SIZE_EXPAND_FILL);
	response_tabs->add_child(response_headers_edit);

	// History
	VBoxContainer *history_tab = memnew(VBoxContainer);
	history_tab->set_name("History");

	history_list = memnew(ItemList);
	history_list->set_v_size_flags(SIZE_EXPAND_FILL);
	history_list->connect("item_selected", this, "_on_history_selected");
	history_tab->add_child(history_list);

	clear_history_btn = memnew(Button);
	clear_history_btn->set_text("Clear History");
	clear_history_btn->connect("pressed", this, "_on_clear_history_pressed");
	history_tab->add_child(clear_history_btn);

	response_tabs->add_child(history_tab);
	response_panel->add_child(response_tabs);

	// Initial layout — will be updated on resize
	_update_layout();

	// ===================== DIALOGS =====================
	save_dialog = memnew(FileDialog);
	save_dialog->set_mode(FileDialog::MODE_SAVE_FILE);
	save_dialog->set_title("Save Request");
	save_dialog->add_filter("*.json ; JSON Request");
	save_dialog->connect("file_selected", this, "_on_save_file_selected");
	add_child(save_dialog);

	load_dialog = memnew(FileDialog);
	load_dialog->set_mode(FileDialog::MODE_OPEN_FILE);
	load_dialog->set_title("Load Request");
	load_dialog->add_filter("*.json ; JSON Request");
	load_dialog->connect("file_selected", this, "_on_load_file_selected");
	add_child(load_dialog);
}

// =========================================================================
// Request building and sending
// =========================================================================

Vector<String> RestApiTesterDock::_build_request_headers() {
	Vector<String> headers;

	// Custom headers from tree
	TreeItem *root = headers_tree->get_root();
	if (root) {
		TreeItem *item = root->get_children();
		while (item) {
			String key = item->get_text(0).strip_edges();
			String val = item->get_text(1).strip_edges();
			if (!key.empty()) {
				headers.push_back(key + ": " + val);
			}
			item = item->get_next();
		}
	}

	// Auth header
	int auth_type = auth_type_option->get_selected_id();
	String auth = rest_build_auth_header(auth_type,
			auth_token_edit->get_text(),
			auth_user_edit->get_text(),
			auth_pass_edit->get_text(),
			auth_key_name_edit->get_text(),
			auth_key_value_edit->get_text());
	if (!auth.empty()) {
		headers.push_back(auth);
	}

	// Content-Type for body
	String body = body_edit->get_text().strip_edges();
	if (!body.empty()) {
		bool has_content_type = false;
		for (int i = 0; i < headers.size(); i++) {
			if (headers[i].to_lower().begins_with("content-type:")) {
				has_content_type = true;
				break;
			}
		}
		if (!has_content_type) {
			switch (body_type_option->get_selected()) {
				case 0:
					headers.push_back("Content-Type: application/json");
					break;
				case 1:
					headers.push_back("Content-Type: application/x-www-form-urlencoded");
					break;
				case 2:
					headers.push_back("Content-Type: text/plain");
					break;
			}
		}
	}

	return headers;
}

void RestApiTesterDock::_on_send_pressed() {
	String url = rest_ensure_url_scheme(url_edit->get_text());
	if (url.empty()) {
		status_label->set_text("Error: URL is empty");
		return;
	}

	url_edit->set_text(url);

	Vector<String> headers = _build_request_headers();
	String body = body_edit->get_text().strip_edges();
	int method_id = method_option->get_selected_id();

	status_label->set_text("Sending...");
	status_label->add_color_override("font_color", Color(0.8, 0.8, 0.8));
	send_btn->set_disabled(true);

	request_start_time = OS::get_singleton()->get_ticks_msec();
	http_request->request(url, headers, true, (HTTPClient::Method)method_id, body);
}

void RestApiTesterDock::_on_request_completed(int p_result, int p_code, const PoolStringArray &p_headers, const PoolByteArray &p_body) {
	uint64_t elapsed = OS::get_singleton()->get_ticks_msec() - request_start_time;
	send_btn->set_disabled(false);

	// Parse response body
	String body_str;
	if (p_body.size() > 0) {
		body_str.parse_utf8((const char *)p_body.read().ptr(), p_body.size());
	}

	// Status color
	Color status_color;
	if (p_result != HTTPRequest::RESULT_SUCCESS) {
		status_color = Color(1, 0.3, 0.3); // red
		status_label->set_text(vformat("Error: %d | %dms", p_result, elapsed));
	} else if (p_code >= 200 && p_code < 300) {
		status_color = Color(0.3, 0.9, 0.3); // green
		status_label->set_text(vformat("%d OK | %dms | %s", p_code, elapsed, String::humanize_size(p_body.size())));
	} else if (p_code >= 300 && p_code < 400) {
		status_color = Color(0.9, 0.9, 0.3); // yellow
		status_label->set_text(vformat("%d Redirect | %dms", p_code, elapsed));
	} else if (p_code >= 400 && p_code < 500) {
		status_color = Color(0.9, 0.6, 0.2); // orange
		status_label->set_text(vformat("%d Client Error | %dms | %s", p_code, elapsed, String::humanize_size(p_body.size())));
	} else {
		status_color = Color(1, 0.3, 0.3); // red
		status_label->set_text(vformat("%d Server Error | %dms | %s", p_code, elapsed, String::humanize_size(p_body.size())));
	}
	status_label->add_color_override("font_color", status_color);

	// Response body (pretty-print JSON)
	response_body_edit->set_text(rest_pretty_print_json(body_str));

	// Response headers
	String headers_text;
	for (int i = 0; i < p_headers.size(); i++) {
		headers_text += p_headers[i] + "\n";
	}
	response_headers_edit->set_text(headers_text);

	// Switch to body tab
	response_tabs->set_current_tab(0);

	// Add to history
	FullHistoryEntry entry;
	entry.method = method_option->get_item_text(method_option->get_selected());
	entry.url = url_edit->get_text();
	// Collect current headers
	TreeItem *root = headers_tree->get_root();
	if (root) {
		TreeItem *item = root->get_children();
		while (item) {
			String key = item->get_text(0).strip_edges();
			String val = item->get_text(1).strip_edges();
			if (!key.empty()) {
				entry.headers.push_back(Pair<String, String>(key, val));
			}
			item = item->get_next();
		}
	}
	entry.body = body_edit->get_text();
	entry.auth_type = auth_type_option->get_selected_id();
	entry.auth_token = auth_token_edit->get_text();
	entry.auth_user = auth_user_edit->get_text();
	entry.auth_pass = auth_pass_edit->get_text();
	entry.auth_key_name = auth_key_name_edit->get_text();
	entry.auth_key_value = auth_key_value_edit->get_text();
	entry.status_code = p_code;
	entry.response_body = body_str;
	entry.response_headers_text = headers_text;
	entry.elapsed_ms = (int)elapsed;
	_add_to_history(entry);
}

// =========================================================================
// Headers management
// =========================================================================

void RestApiTesterDock::_on_add_header_pressed() {
	TreeItem *root = headers_tree->get_root();
	TreeItem *item = headers_tree->create_item(root);
	item->set_editable(0, true);
	item->set_editable(1, true);
	item->set_text(0, "");
	item->set_text(1, "");
}

// =========================================================================
// Auth UI
// =========================================================================

void RestApiTesterDock::_on_auth_type_changed(int p_idx) {
	_update_auth_visibility();
}

void RestApiTesterDock::_update_auth_visibility() {
	int sel = auth_type_option->get_selected_id();
	auth_bearer_container->set_visible(sel == 1);
	auth_basic_container->set_visible(sel == 2);
	auth_apikey_container->set_visible(sel == 3);
}

// =========================================================================
// History
// =========================================================================

void RestApiTesterDock::_add_to_history(const FullHistoryEntry &p_entry) {
	// Insert at front
	full_history.insert(0, p_entry);
	if (full_history.size() > MAX_HISTORY) {
		full_history.resize(MAX_HISTORY);
	}

	// Update UI
	history_list->clear();
	for (int i = 0; i < full_history.size(); i++) {
		const FullHistoryEntry &e = full_history[i];
		String label = vformat("%s  %s  [%d]  %dms", e.method, e.url, e.status_code, e.elapsed_ms);
		history_list->add_item(label);
	}
}

void RestApiTesterDock::_on_history_selected(int p_idx) {
	if (p_idx >= 0 && p_idx < full_history.size()) {
		_restore_request(full_history[p_idx]);
	}
}

void RestApiTesterDock::_on_clear_history_pressed() {
	full_history.clear();
	history_list->clear();
}

void RestApiTesterDock::_restore_request(const FullHistoryEntry &p_entry) {
	// Restore method
	for (int i = 0; i < method_option->get_item_count(); i++) {
		if (method_option->get_item_text(i) == p_entry.method) {
			method_option->select(i);
			break;
		}
	}
	url_edit->set_text(p_entry.url);
	body_edit->set_text(p_entry.body);

	// Restore headers
	TreeItem *root = headers_tree->get_root();
	root->get_children(); // clear
	headers_tree->clear();
	headers_tree->create_item(); // new root
	for (int i = 0; i < p_entry.headers.size(); i++) {
		TreeItem *item = headers_tree->create_item(headers_tree->get_root());
		item->set_editable(0, true);
		item->set_editable(1, true);
		item->set_text(0, p_entry.headers[i].first);
		item->set_text(1, p_entry.headers[i].second);
	}

	// Restore auth
	auth_type_option->select(p_entry.auth_type);
	auth_token_edit->set_text(p_entry.auth_token);
	auth_user_edit->set_text(p_entry.auth_user);
	auth_pass_edit->set_text(p_entry.auth_pass);
	auth_key_name_edit->set_text(p_entry.auth_key_name);
	auth_key_value_edit->set_text(p_entry.auth_key_value);
	_update_auth_visibility();

	// Restore response
	status_label->set_text(vformat("%d | %dms", p_entry.status_code, p_entry.elapsed_ms));
	response_body_edit->set_text(rest_pretty_print_json(p_entry.response_body));
	response_headers_edit->set_text(p_entry.response_headers_text);
}

// =========================================================================
// Save / Load
// =========================================================================

void RestApiTesterDock::_on_save_pressed() {
	save_dialog->popup_centered_ratio(0.5);
}

void RestApiTesterDock::_on_load_pressed() {
	load_dialog->popup_centered_ratio(0.5);
}

void RestApiTesterDock::_on_save_file_selected(const String &p_path) {
	Vector<Pair<String, String>> headers;
	TreeItem *root = headers_tree->get_root();
	if (root) {
		TreeItem *item = root->get_children();
		while (item) {
			headers.push_back(Pair<String, String>(item->get_text(0), item->get_text(1)));
			item = item->get_next();
		}
	}

	Dictionary d = rest_serialize_request(
			method_option->get_item_text(method_option->get_selected()),
			url_edit->get_text(), headers, body_edit->get_text(),
			auth_type_option->get_selected_id(),
			auth_token_edit->get_text(), auth_user_edit->get_text(),
			auth_pass_edit->get_text(), auth_key_name_edit->get_text(),
			auth_key_value_edit->get_text());

	String json = JSON::print(d, "\t");
	FileAccess *f = FileAccess::open(p_path, FileAccess::WRITE);
	if (f) {
		f->store_string(json);
		f->close();
		memdelete(f);
	}
}

void RestApiTesterDock::_on_load_file_selected(const String &p_path) {
	FileAccess *f = FileAccess::open(p_path, FileAccess::READ);
	if (!f) {
		return;
	}
	String json = f->get_as_utf8_string();
	f->close();
	memdelete(f);

	Variant parsed;
	String err_str;
	int err_line;
	if (JSON::parse(json, parsed, err_str, err_line) != OK) {
		status_label->set_text("Error: Invalid JSON file");
		return;
	}

	Dictionary d = parsed;

	// Restore method
	String method = d.has("method") ? String(d["method"]) : "GET";
	for (int i = 0; i < method_option->get_item_count(); i++) {
		if (method_option->get_item_text(i) == method) {
			method_option->select(i);
			break;
		}
	}

	url_edit->set_text(d.has("url") ? String(d["url"]) : "");
	body_edit->set_text(d.has("body") ? String(d["body"]) : "");

	// Headers
	headers_tree->clear();
	headers_tree->create_item();
	if (d.has("headers")) {
		Array arr = d["headers"];
		for (int i = 0; i < arr.size(); i++) {
			Array pair = arr[i];
			if (pair.size() >= 2) {
				TreeItem *item = headers_tree->create_item(headers_tree->get_root());
				item->set_editable(0, true);
				item->set_editable(1, true);
				item->set_text(0, pair[0]);
				item->set_text(1, pair[1]);
			}
		}
	}

	// Auth
	auth_type_option->select(d.has("auth_type") ? int(d["auth_type"]) : 0);
	auth_token_edit->set_text(d.has("auth_token") ? String(d["auth_token"]) : "");
	auth_user_edit->set_text(d.has("auth_user") ? String(d["auth_user"]) : "");
	auth_pass_edit->set_text(d.has("auth_pass") ? String(d["auth_pass"]) : "");
	auth_key_name_edit->set_text(d.has("auth_key_name") ? String(d["auth_key_name"]) : "");
	auth_key_value_edit->set_text(d.has("auth_key_value") ? String(d["auth_key_value"]) : "");
	_update_auth_visibility();

	status_label->set_text("Loaded: " + p_path.get_file());
}

// =========================================================================
// RestApiTesterPlugin
// =========================================================================

RestApiTesterPlugin::RestApiTesterPlugin(EditorNode *p_node) {
	dock = memnew(RestApiTesterDock);
	dock->set_name("REST API");
	add_control_to_bottom_panel(dock, "REST API");
}

RestApiTesterPlugin::~RestApiTesterPlugin() {
}

#endif // TOOLS_ENABLED

// =========================================================================
// Doctests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

#include "core/io/json.h"
#include "core/io/marshalls.h"

// Forward declarations from rest_api_tester.h (utility functions)
String rest_build_auth_header(int p_auth_type, const String &p_token, const String &p_user, const String &p_pass, const String &p_key_name, const String &p_key_value);
String rest_ensure_url_scheme(const String &p_url);
String rest_pretty_print_json(const String &p_json);
Dictionary rest_serialize_request(const String &p_method, const String &p_url,
		const Vector<Pair<String, String>> &p_headers, const String &p_body,
		int p_auth_type, const String &p_auth_token, const String &p_auth_user,
		const String &p_auth_pass, const String &p_auth_key_name, const String &p_auth_key_value);

TEST_SUITE("[[rest_api_tester]] REST API Tester") {
	TEST_CASE("[rest_api_tester] auth: none produces empty string") {
		String result = rest_build_auth_header(0, "", "", "", "", "");
		CHECK(result == "");
	}

	TEST_CASE("[rest_api_tester] auth: bearer token") {
		String result = rest_build_auth_header(1, "mytoken123", "", "", "", "");
		CHECK(result == "Authorization: Bearer mytoken123");
	}

	TEST_CASE("[rest_api_tester] auth: bearer empty token") {
		String result = rest_build_auth_header(1, "", "", "", "", "");
		CHECK(result == "");
	}

	TEST_CASE("[rest_api_tester] auth: basic auth") {
		String result = rest_build_auth_header(2, "", "user", "pass", "", "");
		CHECK(result.begins_with("Authorization: Basic "));
		CHECK(result.length() > 21); // "Authorization: Basic " = 21 chars
	}

	TEST_CASE("[rest_api_tester] auth: api key") {
		String result = rest_build_auth_header(3, "", "", "", "X-API-Key", "secret123");
		CHECK(result == "X-API-Key: secret123");
	}

	TEST_CASE("[rest_api_tester] url: adds https scheme") {
		CHECK(rest_ensure_url_scheme("example.com/api") == "https://example.com/api");
		CHECK(rest_ensure_url_scheme("http://example.com") == "http://example.com");
		CHECK(rest_ensure_url_scheme("https://example.com") == "https://example.com");
	}

	TEST_CASE("[rest_api_tester] url: empty stays empty") {
		CHECK(rest_ensure_url_scheme("") == "");
		CHECK(rest_ensure_url_scheme("  ") == "");
	}

	TEST_CASE("[rest_api_tester] url: preserves query params") {
		CHECK(rest_ensure_url_scheme("example.com/api?key=val&foo=bar") == "https://example.com/api?key=val&foo=bar");
	}

	TEST_CASE("[rest_api_tester] json: pretty prints valid json") {
		String result = rest_pretty_print_json("{\"a\":1,\"b\":2}");
		CHECK(result.find("\t") != -1); // has indentation
		CHECK(result.find("\"a\"") != -1);
		CHECK(result.find("\"b\"") != -1);
	}

	TEST_CASE("[rest_api_tester] json: invalid json returned as-is") {
		String input = "this is not json";
		CHECK(rest_pretty_print_json(input) == input);
	}

	TEST_CASE("[rest_api_tester] serialize: round-trip") {
		Vector<Pair<String, String>> headers;
		headers.push_back(Pair<String, String>("Accept", "application/json"));
		headers.push_back(Pair<String, String>("X-Custom", "value"));

		Dictionary d = rest_serialize_request("POST", "https://api.test.com/data",
				headers, "{\"name\":\"test\"}", 1, "tok123", "", "", "", "");

		CHECK(String(d["method"]) == "POST");
		CHECK(String(d["url"]) == "https://api.test.com/data");
		CHECK(String(d["body"]) == "{\"name\":\"test\"}");
		CHECK(int(d["auth_type"]) == 1);
		CHECK(String(d["auth_token"]) == "tok123");

		Array h = d["headers"];
		CHECK(h.size() == 2);
		Array pair0 = h[0];
		CHECK(String(pair0[0]) == "Accept");
		CHECK(String(pair0[1]) == "application/json");
	}

	TEST_CASE("[rest_api_tester] serialize: empty request defaults") {
		Vector<Pair<String, String>> empty_headers;
		Dictionary d = rest_serialize_request("GET", "", empty_headers, "", 0, "", "", "", "", "");
		CHECK(String(d["method"]) == "GET");
		CHECK(String(d["url"]) == "");
		CHECK(int(d["auth_type"]) == 0);
		Array h = d["headers"];
		CHECK(h.size() == 0);
	}
}

#endif // DOCTEST
