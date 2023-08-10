// A general purpose preprocessor.
// -------------------------------
// https://github.com/codecat/ccpp/blob/master/ccpp.h
//
// Supported directives:
//   #define <word>
//   #undef <word>
//   #if <condition>
//   #else
//   #elif <condition>
//   #endif

#ifndef USTRING_PREPROCESSOR_H
#define USTRING_PREPROCESSOR_H

#include "core/local_vector.h"
#include "core/ustring.h"

class StringProcessor {
	typedef bool (*include_callback_t)(const String &path);
	typedef bool (*command_callback_t)(const String &command, const String &value);

	CharType *p;
	CharType *p_end;

	size_t line;
	size_t column;

	LocalVector<String> defines;
	LocalVector<uint32_t> stack;

	include_callback_t include_callback;
	command_callback_t command_callback;

	bool test_condition();

	void expect_eol();
	void consume_line();

	void overwrite(CharType *p, size_t len);

public:
	void add_define(const String &name);
	void remove_define(const String &name);

	bool has_define(const String &name);

	void set_include_callback(include_callback_t callback);
	void set_command_callback(command_callback_t callback);

	String process(String buffer);
	void process(CharType *buffer, size_t len);

	StringProcessor();
	StringProcessor(const String &multi_defines);
};

#endif // USTRING_PREPROCESSOR_H
