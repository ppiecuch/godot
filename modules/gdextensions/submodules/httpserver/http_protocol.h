/*************************************************************************/
/*  http_protocol.h                                                      */
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

#ifndef HTTP_PROTOCOL_H
#define HTTP_PROTOCOL_H

#include "core/int_types.h"
#include "core/variant.h"

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace http {
const std::string CarriageReturn = "\r\n"; // HTTP defines that any lines must be seperated with a carriage return.
const std::string HTTPVersion1 = "HTTP/1.0";
const std::string HTTPVersion11 = "HTTP/1.1";

// Set to NONE for use in a response message, as responses do not use methods.
enum class MessageMethod {
	NONE,
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	CONNECT,
	TRACE,
	PATCH
};

// Available http headers.
#define HTTP_HEADER_USER_AGENT                      "User-Agent:"
#define HTTP_HEADER_ACCEPT                          "Accept:"
#define HTTP_HEADER_ACCEPT_LANGUAGE                 "Accept-Language:"
#define HTTP_HEADER_ACCEPT_ENCODING                 "Accept-Encoding:"
#define HTTP_HEADER_ACCEPT_CHARSET                  "Accept-Charset:"
#define HTTP_HEADER_PROXY_CONNECTION                "Proxy-Connection:"
#define HTTP_HEADER_CONNECTION                      "Connection:"
#define HTTP_HEADER_COOKIE                          "Cookie:"
#define HTTP_HEADER_HOST                            "Host:"
#define HTTP_HEADER_CACHE_CONTROL                   "Cache-Control:"
#define HTTP_HEADER_CONTENT_TYPE                    "Content-Type:"
#define HTTP_HEADER_CONTENT_LENGTH                  "Content-Length:"
#define HTTP_HEADER_CONTENT_ENCODING                "Content-Encoding:"
#define HTTP_HEADER_SERVER                          "Server:"
#define HTTP_HEADER_DATE                            "Date:"
#define HTTP_HEADER_RANGE                           "Range:"
#define HTTP_HEADER_ETAG                            "Etag:"
#define HTTP_HEADER_EXPIRES                         "Expires:"
#define HTTP_HEADER_REFERER                         "Referer:"
#define HTTP_HEADER_LAST_MODIFIED                   "Last-Modified:"
#define HTTP_HEADER_IF_MOD_SINCE                    "If-Modified-Since:"
#define HTTP_HEADER_IF_NONE_MATCH                   "If-None-Match:"
#define HTTP_HEADER_ACCEPT_RANGES                   "Accept-Ranges:"
#define HTTP_HEADER_TRANSFER_ENCODING               "Transfer-Encoding:"
#define HTTP_HEADER_AUTHORIZATION                   "Authorization:"

// Available http response status codes.
#define HTTP_STATUS_CONTINUE                        100
#define HTTP_STATUS_SWITCHING_PROTOCOLS             101

#define HTTP_STATUS_OK                              200
#define HTTP_STATUS_CREATED                         201
#define HTTP_STATUS_ACCEPTED                        202
#define HTTP_STATUS_NON_AUTHORITATIVE_INFORMATION   203
#define HTTP_STATUS_NO_CONTENT                      204
#define HTTP_STATUS_RESET_CONTENT                   205
#define HTTP_STATUS_PARTIAL_CONTENT                 206

#define HTTP_STATUS_MULTIPLE_CHOICES                300
#define HTTP_STATUS_MOVED_PERMANENTLY               301
#define HTTP_STATUS_FOUND                           302
#define HTTP_STATUS_SEE_OTHER                       303
#define HTTP_STATUS_NOT_MODIFIED                    304
#define HTTP_STATUS_USE_PROXY                       305
#define HTTP_STATUS_TEMPORARY_REDIRECT              307

#define HTTP_STATUS_BAD_REQUEST                     400
#define HTTP_STATUS_UNAUTHORIZED                    401
#define HTTP_STATUS_PAYMENT_REQUIRED                402
#define HTTP_STATUS_FORBIDDEN                       403
#define HTTP_STATUS_NOT_FOUND                       404
#define HTTP_STATUS_METHOD_NOT_ALLOWED              405
#define HTTP_STATUS_NOT_ACCEPTABLE                  406
#define HTTP_STATUS_PROXY_AUTHENICATION_REQUIRED    407
#define HTTP_STATUS_REQUEST_TIME_OUT                408
#define HTTP_STATUS_CONFLICT                        409
#define HTTP_STATUS_GONE                            410
#define HTTP_STATUS_LENGTH_REQUIRED                 411
#define HTTP_STATUS_PRECONDITION_FAILED             412
#define HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE        413
#define HTTP_STATUS_REQUEST_URI_TOO_LARGE           414
#define HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE          415
#define HTTP_STATUS_REQUEST_RANGE_NOT_SATISFIABLE   416
#define HTTP_STATUS_EXPECTATION_FAILED              417

#define HTTP_STATUS_INTERNAL_SERVER_ERROR           500
#define HTTP_STATUS_NOT_IMPLEMENTED                 501
#define HTTP_STATUS_BAD_GATEWAY                     502
#define HTTP_STATUS_SERVICE_UNAVAILABLE             503
#define HTTP_STATUS_GATEWAY_TIME_OUT                504
#define HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED      505

// Available http response status.
#define HTTP_STATUS_CONTINUE_S                      "100 Continue"
#define HTTP_STATUS_SWITCHING_PROTOCOLS_S           "101 Switching Protocols"

#define HTTP_STATUS_OK_S                            "200 OK"
#define HTTP_STATUS_CREATED_S                       "201 Created"
#define HTTP_STATUS_ACCEPTED_S                      "202 Accepted"
#define HTTP_STATUS_NON_AUTHORITATIVE_INFORMATION_S "203 Non-Authoritative Information"
#define HTTP_STATUS_NO_CONTENT_S                    "204 No Content"
#define HTTP_STATUS_RESET_CONTENT_S                 "205 Reset Content"
#define HTTP_STATUS_PARTIAL_CONTENT_S               "206 Partial Content"

#define HTTP_STATUS_MULTIPLE_CHOICES_S              "300 Multiple Choices"
#define HTTP_STATUS_MOVED_PERMANENTLY_S             "301 Moved Permanently"
#define HTTP_STATUS_FOUND_S                         "302 Found"
#define HTTP_STATUS_SEE_OTHER_S                     "303 See Other"
#define HTTP_STATUS_NOT_MODIFIED_S                  "304 Not Modified"
#define HTTP_STATUS_USE_PROXY_S                     "305 Use Proxy"
#define HTTP_STATUS_TEMPORARY_REDIRECT_S            "307 Temporary Redirect"

#define HTTP_STATUS_BAD_REQUEST_S                   "400 Bad Request"
#define HTTP_STATUS_UNAUTHORIZED_S                  "401 Unauthorized"
#define HTTP_STATUS_PAYMENT_REQUIRED_S              "402 Payment Required"
#define HTTP_STATUS_FORBIDDEN_S                     "403 Forbidden"
#define HTTP_STATUS_NOT_FOUND_S                     "404 Not Found"
#define HTTP_STATUS_METHOD_NOT_ALLOWED_S            "405 Not Allowed"
#define HTTP_STATUS_NOT_ACCEPTABLE_S                "406 Not Acceptable"
#define HTTP_STATUS_PROXY_AUTHENICATION_REQUIRED_S  "407 Authenication Required"
#define HTTP_STATUS_REQUEST_TIME_OUT_S              "408 Request Time-out"
#define HTTP_STATUS_CONFLICT_S                      "409 Conflict"
#define HTTP_STATUS_GONE_S                          "410 Gone"
#define HTTP_STATUS_LENGTH_REQUIRED_S               "411 Length Required"
#define HTTP_STATUS_PRECONDITION_FAILED_S           "412 Precondition Required"
#define HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE_S      "413 Request Enity Too Large"
#define HTTP_STATUS_REQUEST_URI_TOO_LARGE_S         "414 Request-URI Too Large"
#define HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE_S        "415 Unsupported Media Type"
#define HTTP_STATUS_REQUEST_RANGE_NOT_SATISFIABLE_S "416 Requested range not satisfiable"
#define HTTP_STATUS_EXPECTATION_FAILED_S            "417 Expectation Failed"

#define HTTP_STATUS_INTERNAL_SERVER_ERROR_S         "500 Internal Server Error"
#define HTTP_STATUS_NOT_IMPLEMENTED_S               "501 Not Implemented"
#define HTTP_STATUS_BAD_GATEWAY_S                   "502 Bad Gateway"
#define HTTP_STATUS_SERVICE_UNAVAILABLE_S           "503 Service Unavailable"
#define HTTP_STATUS_GATEWAY_TIME_OUT_S              "504 Gateway Time-out"
#define HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED_S    "505 HTTP Version not supported"

_FORCE_INLINE_ std::string message_method_to_string(const MessageMethod &method) {
	switch (method) {
		case MessageMethod::NONE:
			return "NONE";
		case MessageMethod::GET:
			return "GET";
		case MessageMethod::HEAD:
			return "HEAD";
		case MessageMethod::POST:
			return "POST";
		case MessageMethod::PUT:
			return "PUT";
		case MessageMethod::DELETE:
			return "DELETE";
		case MessageMethod::CONNECT:
			return "CONNECT";
		case MessageMethod::TRACE:
			return "TRACE";
		case MessageMethod::PATCH:
			return "PATCH";
	}
}

_FORCE_INLINE_ MessageMethod string_to_message_method(const std::string &method) {
	if (method == "NONE")
		return MessageMethod::NONE;
	if (method == "GET")
		return MessageMethod::GET;
	if (method == "HEAD")
		return MessageMethod::HEAD;
	if (method == "POST")
		return MessageMethod::POST;
	if (method == "PUT")
		return MessageMethod::PUT;
	if (method == "DELETE")
		return MessageMethod::DELETE;
	if (method == "CONNECT")
		return MessageMethod::CONNECT;
	if (method == "TRACE")
		return MessageMethod::TRACE;
	if (method == "PATCH")
		return MessageMethod::PATCH;

	return MessageMethod::NONE;
}

// To be returned with a status code in a response is a status text describing the
// status code by text rather than by a code.
//
// This method takes in one of those codes and tries to return a text for it.
_FORCE_INLINE_ std::string status_text_from_status_code(const uint16_t statusCode) {
	switch (statusCode) {
		case 100:
			return "Continue";
		case 101:
			return "Switching Protocol";
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 202:
			return "Accepted";
		case 203:
			return "Non-Authoritative Information";
		case 204:
			return "No Content";
		case 205:
			return "Reset Content";
		case 206:
			return "Partial Content";
		case 300:
			return "Multiple Choice";
		case 301:
			return "Moved Permanently";
		case 302:
			return "Found";
		case 303:
			return "See Other";
		case 304:
			return "Not Modified";
		// 305 is deprecated and 306 is only reserved, skip
		case 307:
			return "Temporary Redirect";
		case 308:
			return "Permanent Redirect";
		case 400:
			return "Bad Request";
		case 401:
			return "Unauthorized";
		case 402:
			// 402 is reserved for future use but has a status message, adding it
			return "Payment Required";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 406:
			return "Not Acceptable";
		case 407:
			return "Proxy Authentication Required";
		case 408:
			return "Request Timeout";
		case 409:
			return "Conflict";
		case 410:
			return "Gone";
		case 411:
			return "Length Required";
		case 412:
			return "Precondition Failed";
		case 413:
			return "Payload Too Large";
		case 414:
			return "URI Too Long";
		case 415:
			return "Unsupported Media Type";
		case 416:
			return "Requested Range Not Satisfiable";
		case 417:
			return "Expectation Failed";
		case 418:
			// might as well return the teapot joke
			return "I'm a teapot";
		case 421:
			return "Misdirected Request";
		case 425:
			return "Too Early";
		case 426:
			return "Upgrade Required";
		case 428:
			return "Precondition Required";
		case 429:
			return "Too Many Requests";
		case 431:
			return "Request Header Fields Too Large";
		case 451:
			return "Unavailable for Legal Reasons";
		case 500:
			return "Internal Server Error";
		case 501:
			return "Not Implemented";
		case 502:
			return "Bad Gateway";
		case 503:
			return "Service Unavailable";
		case 504:
			return "Gateway Timeout";
		case 505:
			return "HTTP Version Not Supported";
		case 506:
			return "Variant Also Negotiates";
		case 507:
			return "Insufficient Storage";
		case 510:
			return "Not Extended";
		case 511:
			return "Network Authentication Required";
		default:
			return "Undefined";
	}
}

/// Case insensitive map-container

_FORCE_INLINE_ static bool case_insensitive_equal(const std::string &str1, const std::string &str2) noexcept {
	return str1.size() == str2.size() && std::equal(str1.begin(), str1.end(), str2.begin(), [](char a, char b) {
		return tolower(a) == tolower(b);
	});
}

struct CaseInsensitiveEqual {
	bool operator()(const std::string &str1, const std::string &str2) const noexcept {
		return case_insensitive_equal(str1, str2);
	}
};

// Based on https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/2595226#2595226
struct CaseInsensitiveHash {
	std::size_t operator()(const std::string &str) const noexcept {
		std::size_t h = 0;
		std::hash<int> hash;
		for (auto c : str) {
			h ^= hash(tolower(c)) + 0x9e3779b9 + (h << 6) + (h >> 2);
		}
		return h;
	}
};

using CaseInsensitiveMultimap = std::unordered_map<std::string, std::string, CaseInsensitiveHash, CaseInsensitiveEqual>;

/// Percent encoding and decoding

// Returns percent-encoded string
static std::string percent_encode(const std::string &value) noexcept {
	static auto hex_chars = "0123456789ABCDEF";

	std::string result;
	result.reserve(value.size()); // Minimum size of result

	for(auto &chr : value) {
		if (!((chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z') || chr == '-' || chr == '.' || chr == '_' || chr == '~')) {
			result += std::string("%") + hex_chars[static_cast<unsigned char>(chr) >> 4] + hex_chars[static_cast<unsigned char>(chr) & 15];
		} else {
			result += chr;
		}
	}

	return result;
}

// Returns percent-decoded string
static std::string percent_decode(const std::string &value) noexcept {
	std::string result;
	result.reserve(value.size() / 3 + (value.size() % 3)); // Minimum size of result

	for (std::size_t i = 0; i < value.size(); ++i) {
		auto &chr = value[i];
		if (chr == '%' && i + 2 < value.size()) {
			auto hex = value.substr(i + 1, 2);
			auto decoded_chr = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
			result += decoded_chr;
			i += 2;
		} else if (chr == '+') {
			result += ' ';
		} else {
			result += chr;
		}
	}

	return result;
}

/// Query string creation and parsing

// Returns query string created from given field names and values
static std::string query_create(const CaseInsensitiveMultimap &fields) noexcept {
	std::string result;

	bool first = true;
	for(auto &field : fields) {
		if (field.second.empty()) {
			result += (!first ? "&" : "") + field.first;
		} else {
			result += (!first ? "&" : "") + field.first + '=' + percent_encode(field.second);
		}
		first = false;
	}

	return result;
}

// Returns query keys with percent-decoded values.
static CaseInsensitiveMultimap query_parse(const std::string &query_string) noexcept {
	CaseInsensitiveMultimap result;

	if(query_string.empty()) {
		return result;
	}
	std::size_t name_pos = 0;
	auto name_end_pos = std::string::npos;
	auto value_pos = std::string::npos;
	for (std::size_t c = 0; c < query_string.size(); ++c) {
		if (query_string[c] == '&') {
			auto name = query_string.substr(name_pos, (name_end_pos == std::string::npos ? c : name_end_pos) - name_pos);
			if (!name.empty()) {
				auto value = value_pos == std::string::npos ? std::string() : query_string.substr(value_pos, c - value_pos);
				result.emplace(std::move(name), percent_decode(value));
			}
			name_pos = c + 1;
			name_end_pos = std::string::npos;
			value_pos = std::string::npos;
		} else if (query_string[c] == '=') {
			name_end_pos = c;
			value_pos = c + 1;
		}
	}
	if (name_pos < query_string.size()) {
		auto name = query_string.substr(name_pos, name_end_pos - name_pos);
		if (!name.empty()) {
			auto value = value_pos >= query_string.size() ? std::string() : query_string.substr(value_pos);
			result.emplace(std::move(name), percent_decode(value));
		}
	}

	return result;
}

// The basic class to represent both HTTP requests and responses.
//
// Contains a method for grabbing the message as a string formatted for
// sending through a server or other means.
//
// Allows for setting a message body that is represented by a vector of
// 8-bit unsigned integers. This is to allow binary data to be sent through
// the message. But, you also can use the included methods to send plain strings.
//
// This class returns as many methods as possible with a reference to the current
// object, to try and allow for chaining methods.
//
// Note: for convenience, the header for Content-Length is automatically included
// as it is grabbed from the `m_body` member. Though, it is only included if the
// body member isn't empty.
class HTTPMessage {
public:
	// Set a header in the map to the value provided.
	_FORCE_INLINE_ HTTPMessage &set_header(const std::string &name, const std::string &p_value) {
		headers[name] = p_value;
		return *this;
	}

	// Set a number of headers based on a generic map of keys and values.
	_FORCE_INLINE_ HTTPMessage &set_headers(const CaseInsensitiveMultimap &p_headers) {
		headers.insert(p_headers.begin(), p_headers.end());
		return *this;
	}

	// Get the string value of a single header from the message.
	// Will return an empty string if the header does not exist.
	_FORCE_INLINE_ std::string get_header(const std::string &p_name) const {
		auto find = headers.find(p_name);
		if (find != headers.end()) {
			return find->second;
		}
		return "";
	}

	// Set the associated message method for this message.
	// Use `NONE` to switch this into a response.
	_FORCE_INLINE_ HTTPMessage &set_method(const MessageMethod &p_method) {
		method = p_method;
		return *this;
	}

	// Grab the current method for this message.
	// Returns `NONE` if this is a response.
	_FORCE_INLINE_ MessageMethod get_method() const {
		return method;
	}

	// Set the path of this message, which will be used if it is a request.
	_FORCE_INLINE_ HTTPMessage &set_path(const std::string &p_path) {
		path = p_path;
		return *this;
	}

	// Grab the current associated path of this message.
	_FORCE_INLINE_ std::string get_path() const {
		return path;
	}

	// Set the path's query values of this message, which will be used if it is a request.
	_FORCE_INLINE_ HTTPMessage &set_query(const CaseInsensitiveMultimap &p_params) {
		params = p_params;
		return *this;
	}

	// Grab the path's query of this message.
	_FORCE_INLINE_ const CaseInsensitiveMultimap &get_query() const {
		return params;
	}

	// Grab the path's query of this message as a string.
	_FORCE_INLINE_ std::string get_query_string() const {
		return query_create(params);
	}

	// Set the version of this HTTP message to the string specified.
	HTTPMessage &set_version(const std::string &p_version) {
		version = p_version;
		return *this;
	}

	// Get the current HTTP version for this message.
	_FORCE_INLINE_ std::string get_version() const {
		return version;
	}

	// Set the status code of this HTTP message.
	HTTPMessage &set_status_code(uint16_t p_code) {
		status_code = p_code;
		return *this;
	}

	// Get the status code for this message.
	_FORCE_INLINE_ uint16_t get_status_code() const {
		return status_code;
	}

	// Set the status message of this HTTP message.
	HTTPMessage &set_status_message(const std::string &p_message) {
		status_message = p_message;
		return *this;
	}

	// Get the current status message for this message.
	// Returns an autogenerated status if one isn't specified.
	_FORCE_INLINE_ std::string get_status_message() const {
		if (status_message.empty()) {
			return status_text_from_status_code(status_code);
		} else {
			return status_message;
		}
	}

	// Takes the headers added to the message along with
	// the body and outputs it to a `std::string` for use
	// in client/server HTTP messages.
	std::string to_string() const {
		std::stringstream output;

		// begin by forming the start line of the message
		if (method == MessageMethod::NONE) {
			output << HTTPVersion11 << " " << status_code << " ";

			if (status_message.empty())
				output << status_text_from_status_code(status_code);
			else
				output << status_message;
		} else {
			output << message_method_to_string(method) << " ";
			output << path << " ";
			output << HTTPVersion11;
		}

		output << CarriageReturn; // output the status lines line break to move on

		// output headers to the message string
		for (auto &header : headers) {
			output << header.first << ": " << header.second << CarriageReturn;
		}
		// automatically output the content length based on
		// the size of the body member if body isn't empty
		if (!body.empty()) {
			output << "Content-Length: " << body.size() << CarriageReturn;
		}
		// seperate headers and body with an extra carriage return
		output << CarriageReturn;

		// convert the 8-bit unsigned body to a std::string for output
		std::string body_str(body.begin(), body.end());
		output << body_str;
		return output.str();
	}

	// Set the body of this message to a string value.
	HTTPMessage &set_message_body(const std::string &p_body) {
		return set_message_body(std::vector<uint8_t>(p_body.begin(), p_body.end()));
	}

	// Set the body of this message to an unsigned 8-bit binary value.
	HTTPMessage &set_message_body(const std::vector<uint8_t> &p_body) {
		body = std::vector<uint8_t>(p_body);
		return *this;
	}

	// Get the body vector for this message.
	_FORCE_INLINE_ std::vector<uint8_t> &get_message_body() {
		return body;
	}

	// Return the size of the binary body vector.
	_FORCE_INLINE_ size_t content_length() {
		return body.size();
	}

	// Return the amount of headers in the message.
	_FORCE_INLINE_ size_t header_count() {
		return headers.size();
	}

private:
	// The HTTP method for this message.
	// Defaults to `NONE` denoting a response.
	MessageMethod method = MessageMethod::NONE;
	// A status code for this message.
	// This is ignored if this is a request, as requests have no notion of statuses.
	uint16_t status_code = 0;
	// A status message to be associated with the status code for this message.
	// Keep blank to use an automatically generated status message.
	std::string status_message = "";
	// The path for the resource specified in the message. Only used for a request.
	// Defaults to blank.
	std::string path = "";
	// The version used for this HTTP message as a string.
	// Defaults to "HTTP/1.1"
	std::string version = HTTPVersion11;
	// A map of headers using a `std::string` for both the key and the value.
	CaseInsensitiveMultimap headers;
	// A map of query params using a `std::string` for both the key and the value.
	CaseInsensitiveMultimap params;
	// A vector of unsigned 8-bit integers used to store message bodies.
	std::vector<uint8_t> body;
};

// An enum of states that the HTTPMessageParser can be in.
enum class MessageParserState {
	NONE,
	PARSING_START_LINE,
	START_LINE_REQUEST,
	START_LINE_RESPONSE,
	HEADER_KEY,
	HEADER_VALUE,
	PARSING_BODY,
};

// A basic class to parse a HTTP message, both request and response.
//
// Allows for either string data to be passed in, or for a vector of
// unsigned 8-bit integers to be passed in.
//
// Requires that a whole message be sent in for parsing.
class HTTPMessageParser {
public:
	// Parse a std::string to a HTTP message.
	//
	// Pass in a pointer to an HTTPMessage which is then written to for headers
	// and other message data.
	//
	// note: this must be a complete HTTP message
	void parse(HTTPMessage *http_message, const std::string &p_buffer) {
		parse(http_message, std::vector<uint8_t>(p_buffer.begin(), p_buffer.end()));
	}

	// Parse a binary vector to an HTTP message.
	//
	// Pass in a pointer to an HTTPMessage which is written to for headers and
	// other message data.
	//
	// note: shrink the vector buffer before passing it in with `shrink_to_fit`
	// otherwise empty characters will show up for the body
	// note: must be a complete HTTP message.
	void parse(HTTPMessage *http_message, const std::vector<uint8_t> &p_buffer) {
		// begin by parsing the start line without knowing if it is a
		// request or a response by setting as undetermined
		MessageParserState state = MessageParserState::PARSING_START_LINE;

		// a temporary string instance used for storing characters of a
		// current line in the message being parsed
		std::string temp = "";

		bool skip_next = false; // whether to skip the next character (for a carriage return)
		std::string header_key = ""; // the current key for a header
		bool has_message_body = false; // whether or not a message body is present
		size_t query_start_index = -1; // the index at which the request's query part begins
		size_t body_start_index = 0; // the index at which the message body begins

		for (size_t index = 0; index < p_buffer.size(); index++) {
			const uint8_t character = p_buffer[index];

			// skip this character as it was marked
			if (skip_next) {
				skip_next = false;
				continue;
			}
			// if we are parsing the body, then we only need to grab an index and break
			// out of this loop as we want to merely insert the data from this vector
			// into the body vector
			if (state == MessageParserState::PARSING_BODY) {
				has_message_body = true;
				body_start_index = index;
				break;
			}
			// if we are parsing the start line but neither a response or request
			if (state == MessageParserState::PARSING_START_LINE) {
				// if we hit a space, we have to check if the start line begins
				// with the HTTP version or the method verb
				if (character == ' ') {
					// this message has a leading version string, thus it is
					// a response and not a request
					if (temp == HTTPVersion1 || temp == HTTPVersion11) {
						http_message->set_method(MessageMethod::NONE);
						state = MessageParserState::START_LINE_RESPONSE;
						temp = "";
						continue;
					}
					// this must be a request, so grab the MessageMethod type
					// for the request, set it, and move on
					else {
						http_message->set_method(string_to_message_method(temp));
						state = MessageParserState::START_LINE_REQUEST;
						temp = "";
						continue;
					}
				}
			}
			// do actions for when the start line is a request
			else if (state == MessageParserState::START_LINE_REQUEST) {
				// once a space is hit, add the path to the message
				if (character == ' ') {
					if (query_start_index < 0) {
						http_message->set_path(temp);
					} else {
						http_message->set_path(temp.substr(0, query_start_index));
						http_message->set_query(query_parse(temp.substr(query_start_index + 1)));
					}
					temp = "";
					continue;
				}
				// remember where query part is starting
				else if (character == '?') {
					query_start_index = temp.size();
				}
				// when the beginning of a carriage return is hit, add the version string
				// to the message and then skip the following new line character, setting
				// the state of the parser to be parsing headers
				else if (character == '\r') {
					http_message->set_version(temp);
					temp = "";
					state = MessageParserState::HEADER_KEY;
					skip_next = true;
					continue;
				}
			}
			// do actions for when the start line is a response
			else if (state == MessageParserState::START_LINE_RESPONSE) {
				// if we are at a space, then we have hit the status code for the response
				if (character == ' ') {
					const int code = std::stoi(temp);
					http_message->set_status_code(static_cast<uint16_t>(code));
					temp = "";
					continue;
				}
				// if we are at a carriage return start, then set the status message for
				// the response, this can be blank in which it will use a generated status
				// this will also set the state of the parser to move on to headers
				else if (character == '\r') {
					http_message->set_status_message(temp);
					temp = "";
					state = MessageParserState::HEADER_KEY;
					skip_next = true;
					continue;
				}
			}
			// if we are parsing header keys and hit a colon, then the key for the header has
			// been fully parsed and should be added to the temporary key holder
			else if (state == MessageParserState::HEADER_KEY && character == ':') {
				header_key = temp;
				temp = "";
				state = MessageParserState::HEADER_VALUE;
				// HTTP defines that the next character in a header should be a space
				// so skip that for parsing the value of the header
				skip_next = true;
				continue;
			}
			// if we are parsing header values and hit the beginning of a carriage return then
			// it is time to add the header to the message with the key and value, and move the
			// state back to parsing keys
			else if (state == MessageParserState::HEADER_VALUE && character == '\r') {
				http_message->set_header(header_key, temp);
				header_key = "";
				temp = "";
				state = MessageParserState::HEADER_KEY;
				skip_next = true; // skip the next character as it will just be a newline
				continue;
			}
			// if we are parsing header keys and we hit a carriage return, then we should assume
			// that the headers have ended, and that we are now parsing a message body.
			else if (state == MessageParserState::HEADER_KEY && character == '\r') {
				temp = "";
				state = MessageParserState::PARSING_BODY;
				skip_next = true; // skip the next character as it'll be a newline
				continue;
			}
			temp += character;
		}
		// add the body to the message if it is present
		if (has_message_body)
			http_message->get_message_body().insert(
					http_message->get_message_body().begin(),
					p_buffer.begin() + body_start_index,
					p_buffer.end());
	}
};
} //namespace http
#endif // HTTP_PROTOCOL_H
