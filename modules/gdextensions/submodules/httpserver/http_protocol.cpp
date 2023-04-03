/**************************************************************************/
/*  http_protocol.cpp                                                     */
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

#include "http_protocol.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#ifdef DOCTEST
TEST_CASE("http messages constructed correctly") {
	SUBCASE("basic http response construction") {
		const std::string expected = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello world!";

		http::HTTPMessage response;

		response.set_status_code(200)
				.set_header("Content-Type", "text/plain")
				.set_header("Connection", "close")
				.set_message_body("Hello world!");

		REQUIRE(response.to_string() == expected);
	}

	SUBCASE("http response with custom status") {
		const std::string expected = "HTTP/1.1 200 All Good\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello world!";

		http::HTTPMessage response;

		response.set_status_code(200)
				.set_status_message("All Good")
				.set_header("Content-Type", "text/plain")
				.set_header("Connection", "close")
				.set_message_body("Hello world!");

		REQUIRE(response.to_string() == expected);
	}

	SUBCASE("basic http request construction") {
		const std::string expected = "GET / HTTP/1.1\r\nHost: example.com\r\nUser-Agent: Test Agent\r\nConnection: keep-alive\r\n\r\n";

		http::HTTPMessage response;

		response.set_method(http::MessageMethod::GET)
				.set_path("/")
				.set_header("User-Agent", "Test Agent")
				.set_header("Connection", "keep-alive")
				.set_header("Host", "example.com");

		REQUIRE(response.to_string() == expected);
	}
}

TEST_CASE("http message parsing correct") {
	http::HTTPMessageParser parser;

	SUBCASE("parse http request") {
		const std::string request_str = "GET / HTTP/1.1\r\nHost: example.com\r\nUser-Agent: Test Agent\r\nConnection: keep-alive\r\n\r\n";

		http::HTTPMessage request;

		parser.parse(&request, request_str);

		REQUIRE(request.header_count() == 3);
		REQUIRE(request.get_header("Host") == "example.com");
		REQUIRE(request.get_header("User-Agent") == "Test Agent");
		REQUIRE(request.get_header("Connection") == "keep-alive");
		REQUIRE(request.get_method() == http::MessageMethod::GET);
		REQUIRE(request.get_path() == "/");
	}

	SUBCASE("parse http response without body") {
		const std::string response_str = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\n";

		http::HTTPMessage response;

		parser.parse(&response, response_str);

		REQUIRE(response.header_count() == 2);
		REQUIRE(response.get_header("Connection") == "close");
		REQUIRE(response.get_header("Content-Type") == "text/plain");
		REQUIRE(response.get_message_body().empty());
	}

	SUBCASE("parse http response with body") {
		const std::string response_str = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello world!";

		http::HTTPMessage response;

		parser.parse(&response, response_str);

		REQUIRE(response.header_count() == 3);
		REQUIRE(response.get_header("Connection") == "close");
		REQUIRE(response.get_header("Content-Type") == "text/plain");
		REQUIRE(!response.get_message_body().empty());

		// convert the body to a string for easy compare
		std::string body(response.get_message_body().begin(), response.get_message_body().end());

		REQUIRE(body == "Hello world!");
	}
}
#endif // DOCTEST
