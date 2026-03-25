/**************************************************************************/
/*  rest_api_tester.h                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifndef REST_API_TESTER_H
#define REST_API_TESTER_H

#ifdef TOOLS_ENABLED

#include "core/io/json.h"
#include "core/io/marshalls.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/text_edit.h"
#include "scene/gui/tree.h"
#include "scene/main/http_request.h"

// =========================================================================
// Utility functions (testable without UI)
// =========================================================================

String rest_build_auth_header(int p_auth_type, const String &p_token, const String &p_user, const String &p_pass, const String &p_key_name, const String &p_key_value);
String rest_ensure_url_scheme(const String &p_url);
String rest_pretty_print_json(const String &p_json);

Dictionary rest_serialize_request(const String &p_method, const String &p_url,
		const Vector<Pair<String, String>> &p_headers, const String &p_body,
		int p_auth_type, const String &p_auth_token, const String &p_auth_user,
		const String &p_auth_pass, const String &p_auth_key_name, const String &p_auth_key_value);

// =========================================================================
// RestApiTesterDock
// =========================================================================

class RestApiTesterDock : public VBoxContainer {
	GDCLASS(RestApiTesterDock, VBoxContainer);

	// --- Request UI ---
	OptionButton *method_option;
	LineEdit *url_edit;
	Button *send_btn;
	Button *save_btn;
	Button *load_btn;

	// Request tabs
	TabContainer *request_tabs;

	// Headers tab
	Tree *headers_tree;
	Button *add_header_btn;

	// Body tab
	OptionButton *body_type_option;
	TextEdit *body_edit;

	// Auth tab
	OptionButton *auth_type_option;
	LineEdit *auth_token_edit;
	LineEdit *auth_user_edit;
	LineEdit *auth_pass_edit;
	LineEdit *auth_key_name_edit;
	LineEdit *auth_key_value_edit;
	VBoxContainer *auth_bearer_container;
	VBoxContainer *auth_basic_container;
	VBoxContainer *auth_apikey_container;

	// --- Response UI ---
	TabContainer *response_tabs;
	Label *status_label;
	TextEdit *response_body_edit;
	TextEdit *response_headers_edit;

	// History tab
	ItemList *history_list;
	Button *clear_history_btn;

	// --- Layout ---
	Container *split_container;
	VBoxContainer *request_panel;
	VBoxContainer *response_panel;
	bool is_horizontal_layout;
	void _update_layout();

	// --- Internal ---
	HTTPRequest *http_request;
	FileDialog *save_dialog;
	FileDialog *load_dialog;

	uint64_t request_start_time;

	struct HistoryItem {
		String method;
		String url;
		int status_code;
		int elapsed_ms;
	};
	Vector<HistoryItem> history;
	static const int MAX_HISTORY = 20;

	// Stored full requests for history restore
	struct FullHistoryEntry {
		String method;
		String url;
		Vector<Pair<String, String>> headers;
		String body;
		int auth_type;
		String auth_token, auth_user, auth_pass, auth_key_name, auth_key_value;
		// Response
		int status_code;
		String response_body;
		String response_headers_text;
		int elapsed_ms;
	};
	Vector<FullHistoryEntry> full_history;

	// Methods
	Vector<String> _build_request_headers();
	void _on_send_pressed();
	void _on_request_completed(int p_result, int p_code, const PoolStringArray &p_headers, const PoolByteArray &p_body);
	void _on_add_header_pressed();
	void _on_auth_type_changed(int p_idx);
	void _on_save_pressed();
	void _on_load_pressed();
	void _on_save_file_selected(const String &p_path);
	void _on_load_file_selected(const String &p_path);
	void _on_history_selected(int p_idx);
	void _on_clear_history_pressed();
	void _add_to_history(const FullHistoryEntry &p_entry);
	void _restore_request(const FullHistoryEntry &p_entry);
	void _update_auth_visibility();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	RestApiTesterDock();
};

// =========================================================================
// RestApiTesterPlugin
// =========================================================================

class RestApiTesterPlugin : public EditorPlugin {
	GDCLASS(RestApiTesterPlugin, EditorPlugin);

	RestApiTesterDock *dock;

public:
	virtual String get_name() const { return "RestApiTester"; }
	RestApiTesterPlugin(EditorNode *p_node);
	~RestApiTesterPlugin();
};

#endif // TOOLS_ENABLED
#endif // REST_API_TESTER_H
