#ifndef LIBSHOCKWAVE_SWF_VM_H
#define LIBSHOCKWAVE_SWF_VM_H

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include "swfaction.h"

namespace SWF
{
	struct Value
	{
		enum Type {
			UNDEFINED,
			VNULL,
			BOOLEAN,
			NUMBER,
			STRING,
			OBJECT
		} type;

		double number;
		bool boolean;
		const char *string;
		void *object;

		Value() : type(UNDEFINED), number(0), boolean(false), string(nullptr), object(nullptr) {}

		static Value make_undefined() { Value v; return v; }

		static Value make_null() { Value v; v.type = VNULL; return v; }

		static Value make_number(double n) { Value v; v.type = NUMBER; v.number = n; return v; }

		static Value make_boolean(bool b) { Value v; v.type = BOOLEAN; v.boolean = b; return v; }

		static Value make_string(const char *s) {
			Value v;
			v.type = STRING;
			v.string = s;
			return v;
		}

		static Value make_object(void *o) { Value v; v.type = OBJECT; v.object = o; return v; }

		double to_number() const {
			switch (type) {
				case UNDEFINED: return NAN;
				case VNULL: return 0;
				case BOOLEAN: return boolean ? 1.0 : 0.0;
				case NUMBER: return number;
				case STRING: {
					if (!string || string[0] == '\0') return NAN;
					char *end;
					double result = strtod(string, &end);
					if (*end != '\0') return NAN;
					return result;
				}
				case OBJECT: return NAN;
			}
			return NAN;
		}

		bool to_bool() const {
			switch (type) {
				case UNDEFINED: return false;
				case VNULL: return false;
				case BOOLEAN: return boolean;
				case NUMBER: return !(number == 0 || std::isnan(number));
				case STRING: return string && string[0] != '\0';
				case OBJECT: return true;
			}
			return false;
		}

		const char *to_string() const {
			switch (type) {
				case UNDEFINED: return "undefined";
				case VNULL: return "null";
				case BOOLEAN: return boolean ? "true" : "false";
				case NUMBER: {
					if (std::isnan(number)) return "NaN";
					if (std::isinf(number)) return number > 0 ? "Infinity" : "-Infinity";
					// Use static buffer for number->string conversion
					static char buf[64];
					if (number == (int64_t)number && std::abs(number) < 1e15)
						snprintf(buf, sizeof(buf), "%lld", (long long)(int64_t)number);
					else
						snprintf(buf, sizeof(buf), "%g", number);
					return buf;
				}
				case STRING: return string ? string : "";
				case OBJECT: return "[object Object]";
			}
			return "undefined";
		}
	};

	enum TimelineCommand
	{
		TC_NONE,
		TC_PLAY,
		TC_STOP,
		TC_GOTO_AND_PLAY,
		TC_GOTO_AND_STOP,
		TC_NEXT_FRAME,
		TC_PREV_FRAME
	};

	struct TimelineAction
	{
		TimelineCommand command = TC_NONE;
		int target_frame = -1;
		const char *target_label = nullptr;
		const char *target_path = nullptr;
	};

	enum PropertyIndex
	{
		PROP_X = 0,
		PROP_Y = 1,
		PROP_XSCALE = 2,
		PROP_YSCALE = 3,
		PROP_CURRENTFRAME = 4,
		PROP_TOTALFRAMES = 5,
		PROP_ALPHA = 6,
		PROP_VISIBLE = 7,
		PROP_WIDTH = 8,
		PROP_HEIGHT = 9,
		PROP_ROTATION = 10,
		PROP_NAME = 13,
		PROP_QUALITY = 18,
		PROP_XMOUSE = 20,
		PROP_YMOUSE = 21
	};

	struct VMCallbacks
	{
		virtual ~VMCallbacks() {}
		virtual void on_timeline_action(const TimelineAction &action) = 0;
		virtual void on_trace(const char *message) = 0;
		virtual Value get_property(const char *target, int index) = 0;
		virtual void set_property(const char *target, int index, const Value &value) = 0;
		virtual Value get_variable(const char *name) = 0;
		virtual void set_variable(const char *name, const Value &value) = 0;
	};

	class VM
	{
	public:
		static const int MAX_STACK = 256;
		static const int MAX_REGISTERS = 256;

		Value stack[MAX_STACK];
		int stack_top;

		Value registers[MAX_REGISTERS];
		int register_count;

		const char * const *constant_pool;
		uint16_t pool_size;

		std::map<std::string, Value> variables;

		const char *current_target;

		VMCallbacks *callbacks;

		VM();
		~VM();

		void push(const Value &v);
		Value pop();
		Value &top();

		void execute(const ActionBlock &block);
		void execute(const ActionList &actions);

	private:
		void emit_timeline(TimelineCommand cmd, int frame = -1, const char *label = nullptr);
	};

}

#endif // LIBSHOCKWAVE_SWF_VM_H
