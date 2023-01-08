/*************************************************************************/
/*  PlayFabHttpClient.h                                                  */
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

/* Auto-generated with SDKGenerator (don't manually edit) */

#include "PlayFabSettings.h"

class PlayFabHTTPClient;

typedef void (*UserCallback)(int h_request, int response_code, Dictionary dict_header, Dictionary parse_data);

struct CResult : public Reference {
	int response_code = 0;
	Dictionary dict_header;
	int size = -1;
	PoolByteArray data;
	int update_count = 0;

	String get_string_from_utf8();
	void update(PlayFabHTTPClient *client);
};

struct CRequest : public Reference {
	int h_request;
	String host;
	int port = 443;
	bool is_ssl = true;
	String url;
	Dictionary dict_request;
	UserCallback user_callback = nullptr;
	Vector<String> list_header;
	Array list_prologue_work;
	Array list_epilogue_work;
	Ref<CResult> o_result = nullptr;

	CRequest(int h_request) :
			h_request(h_request) {}
};

class PlayFabHTTPClient : public HTTPClient {
	GDCLASS(PlayFabHTTPClient, HTTPClient);

	friend int request_queue_size();
	friend int get_status();

	int _request_counter = 0;
	Ref<CRequest> _current_request;
	Array _request_buffers;
	int status_curr = STATUS_DISCONNECTED;
	bool connecting = false;

protected:
	void pro_chk_entity_token() {
		DEV_ASSERT_MSG(
				!PlayFabSettings()._internalSettings.EntityToken.empty(),
				"Must call EntityToken before calling this method");
	}

	void pro_chk_secret_key() {
		DEV_ASSERT_MSG(
				!PlayFabSettings().DeveloperSecretKey.empty(),
				"Must have DeveloperSecretKey set to call this method");
	}

	void pro_chk_session_ticket() {
		DEV_ASSERT_MSG(
				!PlayFabSettings()._internalSettings.ClientSessionTicket.empty(),
				"Must be logged in to call this method");
	}

	String pro_use_auth_authorization() {
		return vformat("%s: %s", "X-Authorization", PlayFabSettings()._internalSettings.ClientSessionTicket);
	}

	String pro_use_auth_entity_token() {
		return vformat("%s: %s", "X-EntityToken", PlayFabSettings()._internalSettings.EntityToken["EntityToken"]);
	}

	String pro_use_auth_secret_key() {
		return vformat("%s: %s", "X-SecretKey", PlayFabSettings().DeveloperSecretKey);
	}

	void pro_use_title_id(Dictionary &dict_request) {
		const String K = "TitleId";

		if (dict_request.has(K)) {
			return;
		} else {
			dict_request[K] = PlayFabSettings().TitleId;
		}

		DEV_ASSERT_MSG(
				dict_request.has(K),
				"Must be have TitleId set to call this method");
	}

	// -- epi --

	void epi_upd_attribute() {
		PlayFabSettings().AdvertisingIdType += "_Successful";
	}

	void epi_upd_entity_token(const Dictionary &json_result) {
		const String K = "EntityToken";
		Dictionary playFabResult = json_result["data"];

		if (playFabResult.has(K)) {
			PlayFabSettings()._internalSettings.EntityToken = playFabResult[K];
		}
	}

	void epi_upd_session_ticket(const Dictionary &json_result) {
		const String K = "SessionTicket";
		Dictionary playFabResult = json_result["data"];

		if (playFabResult.has(K)) {
			PlayFabSettings()._internalSettings.ClientSessionTicket = playFabResult[K];
		}
	}

	void epi_req_multi_step_client_login(const Dictionary &json_result) {
		extern int ClientAttributeInstall(Dictionary dict_request);

		const String K = "SettingsForUser";
		Dictionary playFabResult = json_result["data"];
		Dictionary settingsForUser = playFabResult[K];
		bool disabledAds = PlayFabSettings().DisableAdvertising;
		String adIdType = PlayFabSettings().AdvertisingIdType;
		String adIdVal = PlayFabSettings().AdvertisingIdValue;

		if (settingsForUser["NeedsAttribution"] && !disabledAds && !adIdType.empty() && !adIdVal.empty()) {
			Dictionary dict_request;
			if (adIdType == PlayFabSettings().AD_TYPE_IDFA) {
				dict_request["Idfa"] = adIdVal;
			} else if (adIdType == PlayFabSettings().AD_TYPE_ANDROID_ID) {
				dict_request["Adid"] = adIdVal;
			}
			ClientAttributeInstall(dict_request);
		}
	}

public:
	PoolStringArray get_response_headers() { return _get_response_headers(); }
	Dictionary get_response_headers_as_dictionary() { return _get_response_headers_as_dictionary(); }

	Array build_host() {
		DEV_ASSERT_MSG(!PlayFabSettings().TitleId.empty(), "Missing TitleId in Settings");

		String host = "";
		int port = 443;
		bool is_ssl = true;
		if (!PlayFabSettings().ProductionEnvironmentURL.begins_with("http")) {
			if (!PlayFabSettings().VerticalName.empty()) {
				host = PlayFabSettings().VerticalName;
			} else {
				host = PlayFabSettings().TitleId;
			}
		}

		host += PlayFabSettings().ProductionEnvironmentURL;

		return array(host, port, is_ssl);
	}

	int request_append(String url_path,
			Dictionary dict_request,
			UserCallback user_callback = nullptr,
			Dictionary dict_header_extra = Dictionary(),
			Array list_prologue_work = Array(),
			Array list_epilogue_work = Array()) {
		Vector<String> list_header;

		list_header.push_back("Content-Type: application/json");
		list_header.push_back("X-PlayFabSDK: " + PlayFabSettings()._internalSettings.SdkVersionString);
		list_header.push_back("X-ReportErrorAsSuccess: true");

		if (!PlayFabSettings()._internalSettings.RequestGetParams.empty()) {
			for (const String k : PlayFabSettings()._internalSettings.RequestGetParams.keys()) {
				String v = PlayFabSettings()._internalSettings.RequestGetParams[k];
				if (url_path.empty()) {
					url_path += "?";
					url_path += k;
				} else {
					url_path += "&";
					url_path += k;
				}
				url_path += "=";
				url_path += v;
			}
		}

		for (const int e_val : list_prologue_work) {
			switch (e_val) {
				case PlayFab::CHK_ENTITY_TOKEN: {
					pro_chk_entity_token();
				} break;
				case PlayFab::CHK_SECRET_KEY: {
					pro_chk_secret_key();
				} break;
				case PlayFab::CHK_SESSION_TICKET: {
					pro_chk_session_ticket();
				} break;
				case PlayFab::USE_AUTH_AUTHORIZATION: {
					list_header.push_back(pro_use_auth_authorization());
				} break;
				case PlayFab::USE_AUTH_ENTITY_TOKEN: {
					list_header.push_back(pro_use_auth_entity_token());
				} break;
				case PlayFab::USE_AUTH_SECRET_KEY: {
					list_header.push_back(pro_use_auth_secret_key());
				} break;
				case PlayFab::USE_TITLE_ID: {
					pro_use_title_id(dict_request);
				} break;
			}
		}

		for (const auto &k : dict_header_extra.keys()) {
			list_header.push_back(vformat("%s: %s", k, dict_header_extra[k]));
		}

		_request_counter++;
		Ref<CRequest> o = newref(CRequest, _request_counter);
		Array list_host = build_host();

		o->host = list_host[0];
		o->port = list_host[1];
		o->is_ssl = list_host[2];
		o->url = url_path;
		o->dict_request = dict_request;
		o->user_callback = user_callback;
		o->list_header = list_header;
		o->list_prologue_work = list_prologue_work;
		o->list_epilogue_work = list_epilogue_work;

		_request_buffers.append(o);

		return o->h_request;
	}

	Ref<CRequest> dispatch(Ref<CRequest> o_request) {
		String raw_text = o_request->o_result->get_string_from_utf8();
		int err_line;
		String err_message;
		Variant json;
		Error parse_result = JSON::parse(raw_text, json, err_message, err_line);
		Dictionary parse_data = json;

		if (parse_result == OK) {
			if (parse_data.has("code")) {
				if (int(parse_data["code"]) == 200) {
					for (const int e_val : o_request->list_epilogue_work) {
						switch (e_val) {
							case PlayFab::REQ_MULTI_STEP_CLIENT_LOGIN: {
								epi_req_multi_step_client_login(parse_data);
							} break;
							case PlayFab::UPD_ATTRIBUTE: {
								epi_upd_attribute();
							} break;
							case PlayFab::UPD_ENTITY_TOKEN: {
								epi_upd_entity_token(parse_data);
							} break;
							case PlayFab::UPD_SESSION_TICKET: {
								epi_upd_session_ticket(parse_data);
							} break;
						}
					}
				}
			}
		}
		if (o_request->user_callback) {
			o_request->user_callback(
					o_request->h_request,
					o_request->o_result->response_code,
					o_request->o_result->dict_header,
					parse_data);
		}

		return nullptr;
	}

	void reset() {
		close();

		_current_request = nullref(CRequest);
		_request_buffers.clear();
		status_curr = get_status();

		connecting = false;
	}

	bool request_cancel(int h_request) {
		int pos = 0;
		Array list_remove_target;
		for (const Ref<CRequest> req : _request_buffers) {
			if (req->h_request == h_request) {
				list_remove_target.append(pos);
			}
			pos++;
		}
		for (const auto &n : list_remove_target) {
			_request_buffers.remove(n);
		}
		return list_remove_target.size() > 0;
	}

	void update(real_t delta) {
		if (poll() == OK) {
			return;
		}
		status_curr = get_status();

		if (!_current_request) {
			if (_request_buffers.size() > 0) {
				_current_request = _request_buffers.pop_front();
				status_curr = STATUS_DISCONNECTED;
			} else {
				if (status_curr == STATUS_CONNECTION_ERROR) {
					close();
				}
			}
		} else {
			if (status_curr == STATUS_DISCONNECTED) {
				if (!connecting) {
					if (connect_to_host(_current_request->host, _current_request->port, _current_request->is_ssl)) {
						connecting = true;
					} else {
						WARN_PRINT("Connection failed.");
					}
				}
			} else if (status_curr == STATUS_CONNECTED) {
				connecting = false;

				if (!_current_request->o_result) {
					request(
							HTTPClient::METHOD_POST,
							_current_request->url,
							_current_request->list_header,
							JSON::print(_current_request->dict_request));
					_current_request->o_result = newref(CResult);
				}
			} else if (status_curr == STATUS_BODY) {
				if (has_response()) {
					_current_request->o_result->update(this);
				}
			}
			if (status_curr != STATUS_BODY) {
				if (_current_request->o_result) {
					if (_current_request->o_result->update_count > 0) {
						_current_request = dispatch(_current_request);
					}
				}
			}
		}
	}
};

void CResult::update(PlayFabHTTPClient *client) {
	if (size < 0) {
		response_code = client->get_response_code();
		dict_header = client->get_response_headers_as_dictionary();

		if (!client->is_response_chunked()) {
			size = client->get_response_body_length();
			data = PoolByteArray();
		}
	}
	PoolByteArray chunk = client->read_response_body_chunk();
	if (chunk.size() > 0) {
		data.append_array(chunk);
	}
	update_count++;
}

String CResult::get_string_from_utf8() {
	String s;
	if (data.size() > 0) {
		PoolByteArray::Read r = data.read();
		s.parse_utf8((const char *)r.ptr(), data.size());
	}
	return s;
}
