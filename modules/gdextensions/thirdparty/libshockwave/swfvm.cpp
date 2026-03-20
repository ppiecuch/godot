#include "swfvm.h"

using namespace SWF;

VM::VM()
{
	stack_top = 0;
	register_count = 4; // default SWF4 registers
	constant_pool = nullptr;
	pool_size = 0;
	current_target = nullptr;
	callbacks = nullptr;
}

VM::~VM()
{
}

void VM::push(const Value &v)
{
	if (stack_top < MAX_STACK)
		stack[stack_top++] = v;
}

Value VM::pop()
{
	if (stack_top > 0)
		return stack[--stack_top];
	return Value::make_undefined();
}

Value &VM::top()
{
	static Value undefined;
	if (stack_top > 0)
		return stack[stack_top - 1];
	undefined = Value::make_undefined();
	return undefined;
}

void VM::emit_timeline(TimelineCommand cmd, int frame, const char *label)
{
	if (!callbacks) return;
	TimelineAction ta;
	ta.command = cmd;
	ta.target_frame = frame;
	ta.target_label = label;
	ta.target_path = current_target;
	callbacks->on_timeline_action(ta);
}

void VM::execute(const ActionBlock &block)
{
	execute(block.actions);
}

void VM::execute(const ActionList &actions)
{
	if (actions.empty()) return;

	// Build offset->index map for jump resolution
	std::map<uint32_t, int> offset_map;
	for (int i = 0; i < (int)actions.size(); i++) {
		offset_map[actions[i].offset] = i;
	}

	int ip = 0;
	int count = (int)actions.size();

	while (ip >= 0 && ip < count) {
		const ActionRecord &ar = actions[ip];

		if (ar.opcode == 0) break; // End of actions

		switch (ar.opcode) {
			// Timeline control
			case ActionPlay:
				emit_timeline(TC_PLAY);
				break;
			case ActionStop:
				emit_timeline(TC_STOP);
				break;
			case ActionNextFrame:
				emit_timeline(TC_NEXT_FRAME);
				break;
			case ActionPreviousFrame:
				emit_timeline(TC_PREV_FRAME);
				break;
			case ActionGotoFrame:
				emit_timeline(TC_GOTO_AND_STOP, ar.frame);
				break;
			case ActionGoToLabel:
				emit_timeline(TC_GOTO_AND_STOP, -1, ar.string1);
				break;
			case ActionGotoFrame2:
			{
				Value v = pop();
				int frame = (int)v.to_number();
				if (ar.play_flag)
					emit_timeline(TC_GOTO_AND_PLAY, frame);
				else
					emit_timeline(TC_GOTO_AND_STOP, frame);
				break;
			}

			// Stack operations
			case ActionPush:
			{
				for (size_t i = 0; i < ar.push_values.size(); i++) {
					const PushValue &pv = ar.push_values[i];
					switch (pv.type) {
						case PUSH_STRING:
							push(Value::make_string(pv.string_value));
							break;
						case PUSH_FLOAT:
							push(Value::make_number(pv.float_value));
							break;
						case PUSH_NULL:
							push(Value::make_null());
							break;
						case PUSH_UNDEFINED:
							push(Value::make_undefined());
							break;
						case PUSH_REGISTER:
							push(registers[pv.register_index]);
							break;
						case PUSH_BOOLEAN:
							push(Value::make_boolean(pv.boolean_value));
							break;
						case PUSH_DOUBLE:
							push(Value::make_number(pv.double_value));
							break;
						case PUSH_INTEGER:
							push(Value::make_number(pv.integer_value));
							break;
						case PUSH_CONSTANT8:
							if (constant_pool && pv.constant8 < pool_size)
								push(Value::make_string(constant_pool[pv.constant8]));
							else
								push(Value::make_undefined());
							break;
						case PUSH_CONSTANT16:
							if (constant_pool && pv.constant16 < pool_size)
								push(Value::make_string(constant_pool[pv.constant16]));
							else
								push(Value::make_undefined());
							break;
					}
				}
				break;
			}
			case ActionPop:
				pop();
				break;
			case ActionPushDuplicate:
				push(top());
				break;
			case ActionStackSwap:
			{
				if (stack_top >= 2) {
					Value tmp = stack[stack_top - 1];
					stack[stack_top - 1] = stack[stack_top - 2];
					stack[stack_top - 2] = tmp;
				}
				break;
			}

			// Arithmetic
			case ActionAdd:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_number(a.to_number() + b.to_number()));
				break;
			}
			case ActionSubtract:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_number(a.to_number() - b.to_number()));
				break;
			}
			case ActionMultiply:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_number(a.to_number() * b.to_number()));
				break;
			}
			case ActionDivide:
			{
				Value b = pop();
				Value a = pop();
				double divisor = b.to_number();
				if (divisor == 0) {
					push(Value::make_string("#ERROR#"));
				} else {
					push(Value::make_number(a.to_number() / divisor));
				}
				break;
			}
			case ActionModulo:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_number(fmod(a.to_number(), b.to_number())));
				break;
			}
			case ActionAdd2:
			{
				Value b = pop();
				Value a = pop();
				if (a.type == Value::STRING || b.type == Value::STRING) {
					// String concatenation
					const char *as = a.to_string();
					const char *bs = b.to_string();
					size_t alen = strlen(as);
					size_t blen = strlen(bs);
					char *result = new char[alen + blen + 1];
					memcpy(result, as, alen);
					memcpy(result + alen, bs, blen + 1);
					push(Value::make_string(result));
				} else {
					push(Value::make_number(a.to_number() + b.to_number()));
				}
				break;
			}
			case ActionIncrement:
			{
				Value a = pop();
				push(Value::make_number(a.to_number() + 1));
				break;
			}
			case ActionDecrement:
			{
				Value a = pop();
				push(Value::make_number(a.to_number() - 1));
				break;
			}

			// Comparison
			case ActionEquals:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_boolean(a.to_number() == b.to_number()));
				break;
			}
			case ActionLess:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_boolean(a.to_number() < b.to_number()));
				break;
			}
			case ActionEquals2:
			{
				Value b = pop();
				Value a = pop();
				// Type-aware equality
				if (a.type == b.type) {
					switch (a.type) {
						case Value::UNDEFINED:
						case Value::VNULL:
							push(Value::make_boolean(true));
							break;
						case Value::BOOLEAN:
							push(Value::make_boolean(a.boolean == b.boolean));
							break;
						case Value::NUMBER:
							push(Value::make_boolean(a.number == b.number));
							break;
						case Value::STRING:
							push(Value::make_boolean(strcmp(a.string ? a.string : "", b.string ? b.string : "") == 0));
							break;
						case Value::OBJECT:
							push(Value::make_boolean(a.object == b.object));
							break;
					}
				} else if ((a.type == Value::VNULL && b.type == Value::UNDEFINED) ||
						   (a.type == Value::UNDEFINED && b.type == Value::VNULL)) {
					push(Value::make_boolean(true));
				} else {
					push(Value::make_boolean(a.to_number() == b.to_number()));
				}
				break;
			}
			case ActionLess2:
			{
				Value b = pop();
				Value a = pop();
				if (a.type == Value::STRING && b.type == Value::STRING) {
					push(Value::make_boolean(strcmp(a.string ? a.string : "", b.string ? b.string : "") < 0));
				} else {
					push(Value::make_boolean(a.to_number() < b.to_number()));
				}
				break;
			}
			case ActionStrictEquals:
			{
				Value b = pop();
				Value a = pop();
				if (a.type != b.type) {
					push(Value::make_boolean(false));
				} else {
					switch (a.type) {
						case Value::UNDEFINED:
						case Value::VNULL:
							push(Value::make_boolean(true));
							break;
						case Value::BOOLEAN:
							push(Value::make_boolean(a.boolean == b.boolean));
							break;
						case Value::NUMBER:
							push(Value::make_boolean(a.number == b.number));
							break;
						case Value::STRING:
							push(Value::make_boolean(strcmp(a.string ? a.string : "", b.string ? b.string : "") == 0));
							break;
						case Value::OBJECT:
							push(Value::make_boolean(a.object == b.object));
							break;
					}
				}
				break;
			}
			case ActionGreater:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_boolean(a.to_number() > b.to_number()));
				break;
			}
			case ActionStringEquals:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_boolean(strcmp(a.to_string(), b.to_string()) == 0));
				break;
			}
			case ActionStringLess:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_boolean(strcmp(a.to_string(), b.to_string()) < 0));
				break;
			}
			case ActionStringGreater:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_boolean(strcmp(a.to_string(), b.to_string()) > 0));
				break;
			}

			// Logic
			case ActionAnd:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_boolean(a.to_bool() && b.to_bool()));
				break;
			}
			case ActionOr:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_boolean(a.to_bool() || b.to_bool()));
				break;
			}
			case ActionNot:
			{
				Value a = pop();
				push(Value::make_boolean(!a.to_bool()));
				break;
			}

			// Bitwise
			case ActionBitAnd:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_number((int32_t)a.to_number() & (int32_t)b.to_number()));
				break;
			}
			case ActionBitOr:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_number((int32_t)a.to_number() | (int32_t)b.to_number()));
				break;
			}
			case ActionBitXor:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_number((int32_t)a.to_number() ^ (int32_t)b.to_number()));
				break;
			}
			case ActionBitLShift:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_number((int32_t)a.to_number() << ((int32_t)b.to_number() & 0x1F)));
				break;
			}
			case ActionBitRShift:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_number((int32_t)a.to_number() >> ((int32_t)b.to_number() & 0x1F)));
				break;
			}
			case ActionBitURShift:
			{
				Value b = pop();
				Value a = pop();
				push(Value::make_number((double)((uint32_t)(int32_t)a.to_number() >> ((int32_t)b.to_number() & 0x1F))));
				break;
			}

			// String operations
			case ActionStringAdd:
			{
				Value b = pop();
				Value a = pop();
				const char *as = a.to_string();
				const char *bs = b.to_string();
				size_t alen = strlen(as);
				size_t blen = strlen(bs);
				char *result = new char[alen + blen + 1];
				memcpy(result, as, alen);
				memcpy(result + alen, bs, blen + 1);
				push(Value::make_string(result));
				break;
			}
			case ActionStringLength:
			{
				Value a = pop();
				push(Value::make_number(strlen(a.to_string())));
				break;
			}
			case ActionStringExtract:
			{
				Value count = pop();
				Value index = pop();
				Value str = pop();
				const char *s = str.to_string();
				int idx = (int)index.to_number();
				int cnt = (int)count.to_number();
				size_t slen = strlen(s);
				if (idx < 0) idx = 0;
				if (idx >= (int)slen) {
					push(Value::make_string(""));
				} else {
					if (cnt < 0 || idx + cnt > (int)slen) cnt = (int)slen - idx;
					char *result = new char[cnt + 1];
					memcpy(result, s + idx, cnt);
					result[cnt] = '\0';
					push(Value::make_string(result));
				}
				break;
			}
			case ActionCharToAscii:
			{
				Value a = pop();
				const char *s = a.to_string();
				push(Value::make_number(s[0]));
				break;
			}
			case ActionAsciiToChar:
			{
				Value a = pop();
				char *s = new char[2];
				s[0] = (char)(int)a.to_number();
				s[1] = '\0';
				push(Value::make_string(s));
				break;
			}
			case ActionToString:
			{
				Value a = pop();
				push(Value::make_string(a.to_string()));
				break;
			}
			case ActionToNumber:
			{
				Value a = pop();
				push(Value::make_number(a.to_number()));
				break;
			}
			case ActionToInteger:
			{
				Value a = pop();
				push(Value::make_number((int32_t)a.to_number()));
				break;
			}

			// Variables
			case ActionGetVariable:
			{
				Value name = pop();
				const char *varname = name.to_string();
				if (callbacks) {
					push(callbacks->get_variable(varname));
				} else if (variables.count(varname)) {
					push(variables[varname]);
				} else {
					push(Value::make_undefined());
				}
				break;
			}
			case ActionSetVariable:
			{
				Value val = pop();
				Value name = pop();
				const char *varname = name.to_string();
				if (callbacks) {
					callbacks->set_variable(varname, val);
				}
				variables[varname] = val;
				break;
			}
			case ActionDefineLocal:
			{
				Value val = pop();
				Value name = pop();
				variables[name.to_string()] = val;
				break;
			}
			case ActionDefineLocal2:
			{
				Value name = pop();
				variables[name.to_string()] = Value::make_undefined();
				break;
			}

			// Properties
			case ActionGetProperty:
			{
				Value index = pop();
				Value target = pop();
				if (callbacks) {
					push(callbacks->get_property(target.to_string(), (int)index.to_number()));
				} else {
					push(Value::make_undefined());
				}
				break;
			}
			case ActionSetProperty:
			{
				Value val = pop();
				Value index = pop();
				Value target = pop();
				if (callbacks) {
					callbacks->set_property(target.to_string(), (int)index.to_number(), val);
				}
				break;
			}

			// Members (stub)
			case ActionGetMember:
			{
				pop(); // name
				pop(); // object
				push(Value::make_undefined());
				break;
			}
			case ActionSetMember:
			{
				pop(); // value
				pop(); // name
				pop(); // object
				break;
			}

			// Control flow
			case ActionJump:
			{
				uint32_t target_offset = ar.offset + 3 + ar.length + ar.branch_offset;
				// Actually for Jump: offset after the action (opcode(1) + length(2) = 3 for data actions)
				// branch_offset is relative to byte after this action
				uint32_t after_action = ar.offset + 1 + 2 + ar.length; // opcode + length_field + data
				target_offset = after_action + ar.branch_offset;
				if (offset_map.count(target_offset)) {
					ip = offset_map[target_offset];
					continue;
				}
				break;
			}
			case ActionIf:
			{
				Value cond = pop();
				if (cond.to_bool()) {
					uint32_t after_action = ar.offset + 1 + 2 + ar.length;
					uint32_t target_offset = after_action + ar.branch_offset;
					if (offset_map.count(target_offset)) {
						ip = offset_map[target_offset];
						continue;
					}
				}
				break;
			}
			case ActionReturn:
			{
				return; // Exit current function
			}

			// Functions (stubs)
			case ActionDefineFunction:
			case ActionDefineFunction2:
			{
				// Function definition - just push undefined as the function value
				push(Value::make_undefined());
				break;
			}
			case ActionCallFunction:
			{
				pop(); // function name
				Value nargs = pop();
				int n = (int)nargs.to_number();
				for (int i = 0; i < n; i++) pop(); // arguments
				push(Value::make_undefined()); // return value
				break;
			}
			case ActionCallMethod:
			{
				pop(); // method name
				pop(); // object
				Value nargs = pop();
				int n = (int)nargs.to_number();
				for (int i = 0; i < n; i++) pop();
				push(Value::make_undefined());
				break;
			}

			// Registers
			case ActionStoreRegister:
			{
				if (ar.register_index < MAX_REGISTERS && stack_top > 0) {
					registers[ar.register_index] = stack[stack_top - 1]; // peek, don't pop
				}
				break;
			}
			case ActionConstantPool:
			{
				pool_size = (uint16_t)ar.constant_pool.size();
				constant_pool = pool_size > 0 ? (const char * const *)ar.constant_pool.data() : nullptr;
				break;
			}

			// Target
			case ActionSetTarget:
			{
				current_target = ar.string1;
				break;
			}
			case ActionSetTarget2:
			{
				Value target = pop();
				current_target = target.to_string();
				break;
			}

			// Misc
			case ActionTrace:
			{
				Value msg = pop();
				if (callbacks)
					callbacks->on_trace(msg.to_string());
				break;
			}
			case ActionGetTime:
			{
				// Return 0 for tests, in real use would return milliseconds
				push(Value::make_number(0));
				break;
			}
			case ActionRandomNumber:
			{
				Value max = pop();
				int maxval = (int)max.to_number();
				if (maxval > 0)
					push(Value::make_number(rand() % maxval));
				else
					push(Value::make_number(0));
				break;
			}
			case ActionTypeOf:
			{
				Value a = pop();
				switch (a.type) {
					case Value::UNDEFINED: push(Value::make_string("undefined")); break;
					case Value::VNULL: push(Value::make_string("null")); break;
					case Value::BOOLEAN: push(Value::make_string("boolean")); break;
					case Value::NUMBER: push(Value::make_string("number")); break;
					case Value::STRING: push(Value::make_string("string")); break;
					case Value::OBJECT: push(Value::make_string("object")); break;
				}
				break;
			}
			case ActionTargetPath:
			{
				pop(); // movie clip
				push(Value::make_string(current_target ? current_target : ""));
				break;
			}

			// No-ops / unsupported
			case ActionToggleQuality:
			case ActionStopSounds:
			case ActionStartDrag:
			case ActionEndDrag:
			case ActionCloneSprite:
			case ActionRemoveSprite:
			case ActionCall:
			case ActionWith:
			case ActionWaitForFrame:
			case ActionWaitForFrame2:
			case ActionGetURL:
			case ActionGetURL2:
			case ActionDelete:
			case ActionNewObject:
			case ActionNewMethod:
			case ActionInitArray:
			case ActionInitObject:
			case ActionEnumerate:
			case ActionEnumerate2:
			case ActionInstanceOf:
			case ActionCastOp:
			case ActionImplementsOp:
			case ActionExtends:
			case ActionThrow:
			case ActionTry:
				break;

			default:
				break;
		}

		ip++;
	}
}

#ifdef DOCTEST
#include "doctest/doctest.h"

struct MockVMCallbacks : public VMCallbacks
{
	std::vector<TimelineAction> timeline_actions;
	std::vector<std::string> trace_messages;
	std::map<std::string, Value> variables;
	std::map<std::pair<std::string, int>, Value> properties;

	void on_timeline_action(const TimelineAction &action) override { timeline_actions.push_back(action); }
	void on_trace(const char *message) override { trace_messages.push_back(message ? message : ""); }
	Value get_property(const char *target, int index) override {
		auto key = std::make_pair(std::string(target ? target : ""), index);
		if (properties.count(key)) return properties[key];
		return Value::make_undefined();
	}
	void set_property(const char *target, int index, const Value &value) override {
		properties[std::make_pair(std::string(target ? target : ""), index)] = value;
	}
	Value get_variable(const char *name) override {
		if (variables.count(name)) return variables[name];
		return Value::make_undefined();
	}
	void set_variable(const char *name, const Value &value) override { variables[name] = value; }
};

// Helper to make a simple ActionList with one op and an end sentinel
static ActionList make_single_op(uint8_t opcode) {
	ActionList actions;
	ActionRecord ar; ar.opcode = opcode; ar.offset = 0;
	actions.push_back(ar);
	ActionRecord end; end.opcode = 0; end.offset = 1;
	actions.push_back(end);
	return actions;
}

TEST_SUITE("[[libshockwave]] AVM1 Value Type") {

	TEST_CASE("[vm] construction defaults") {
		Value v;
		CHECK(v.type == Value::UNDEFINED);
	}

	TEST_CASE("[vm] to_number coercion") {
		CHECK(std::isnan(Value::make_undefined().to_number()));
		CHECK(Value::make_null().to_number() == 0);
		CHECK(Value::make_boolean(true).to_number() == 1);
		CHECK(Value::make_boolean(false).to_number() == 0);
		CHECK(Value::make_number(42).to_number() == 42);
		CHECK(Value::make_string("123").to_number() == 123);
		CHECK(std::isnan(Value::make_string("abc").to_number()));
		CHECK(std::isnan(Value::make_string("").to_number()));
	}

	TEST_CASE("[vm] to_bool coercion") {
		CHECK(Value::make_undefined().to_bool() == false);
		CHECK(Value::make_null().to_bool() == false);
		CHECK(Value::make_number(0).to_bool() == false);
		CHECK(Value::make_number(NAN).to_bool() == false);
		CHECK(Value::make_number(1).to_bool() == true);
		CHECK(Value::make_number(-1).to_bool() == true);
		CHECK(Value::make_string("").to_bool() == false);
		CHECK(Value::make_string("x").to_bool() == true);
	}

	TEST_CASE("[vm] to_string coercion") {
		CHECK(strcmp(Value::make_undefined().to_string(), "undefined") == 0);
		CHECK(strcmp(Value::make_null().to_string(), "null") == 0);
		CHECK(strcmp(Value::make_boolean(true).to_string(), "true") == 0);
		CHECK(strcmp(Value::make_number(0).to_string(), "0") == 0);
		CHECK(strcmp(Value::make_number(1.5).to_string(), "1.5") == 0);
	}
}

TEST_SUITE("[[libshockwave]] AVM1 Stack Machine") {

	TEST_CASE("[vm] push and pop") {
		VM vm;
		vm.push(Value::make_number(42));
		CHECK(vm.pop().number == 42);
	}

	TEST_CASE("[vm] stack underflow") {
		VM vm;
		CHECK(vm.pop().type == Value::UNDEFINED);
	}

	TEST_CASE("[vm] PushDuplicate") {
		VM vm;
		vm.push(Value::make_number(5));
		vm.execute(make_single_op(ActionPushDuplicate));
		CHECK(vm.stack_top == 2);
		CHECK(vm.stack[0].number == 5);
		CHECK(vm.stack[1].number == 5);
	}

	TEST_CASE("[vm] StackSwap") {
		VM vm;
		vm.push(Value::make_number(1));
		vm.push(Value::make_number(2));
		vm.execute(make_single_op(ActionStackSwap));
		CHECK(vm.stack[0].number == 2);
		CHECK(vm.stack[1].number == 1);
	}

	TEST_CASE("[vm] Arithmetic: Add 2+3=5") {
		VM vm;
		vm.push(Value::make_number(2));
		vm.push(Value::make_number(3));
		vm.execute(make_single_op(ActionAdd));
		CHECK(vm.pop().number == 5);
	}

	TEST_CASE("[vm] Arithmetic: Subtract 10-4=6") {
		VM vm;
		vm.push(Value::make_number(10));
		vm.push(Value::make_number(4));
		vm.execute(make_single_op(ActionSubtract));
		CHECK(vm.pop().number == 6);
	}

	TEST_CASE("[vm] Arithmetic: Multiply 3*7=21") {
		VM vm;
		vm.push(Value::make_number(3));
		vm.push(Value::make_number(7));
		vm.execute(make_single_op(ActionMultiply));
		CHECK(vm.pop().number == 21);
	}

	TEST_CASE("[vm] Arithmetic: Divide 15/4=3.75") {
		VM vm;
		vm.push(Value::make_number(15));
		vm.push(Value::make_number(4));
		vm.execute(make_single_op(ActionDivide));
		CHECK(vm.pop().number == doctest::Approx(3.75));
	}

	TEST_CASE("[vm] Division by zero returns #ERROR#") {
		VM vm;
		vm.push(Value::make_number(10));
		vm.push(Value::make_number(0));
		vm.execute(make_single_op(ActionDivide));
		Value r = vm.pop();
		CHECK(r.type == Value::STRING);
		CHECK(strcmp(r.string, "#ERROR#") == 0);
	}

	TEST_CASE("[vm] Modulo 10%3=1") {
		VM vm;
		vm.push(Value::make_number(10));
		vm.push(Value::make_number(3));
		vm.execute(make_single_op(ActionModulo));
		CHECK(vm.pop().number == 1);
	}

	TEST_CASE("[vm] Add2 string concat") {
		VM vm;
		vm.push(Value::make_string("hello"));
		vm.push(Value::make_string(" world"));
		vm.execute(make_single_op(ActionAdd2));
		CHECK(strcmp(vm.pop().string, "hello world") == 0);
	}

	TEST_CASE("[vm] Add2 string+number") {
		VM vm;
		vm.push(Value::make_string("num"));
		vm.push(Value::make_number(3));
		vm.execute(make_single_op(ActionAdd2));
		CHECK(strcmp(vm.pop().string, "num3") == 0);
	}

	TEST_CASE("[vm] Add2 number+number") {
		VM vm;
		vm.push(Value::make_number(2));
		vm.push(Value::make_number(3));
		vm.execute(make_single_op(ActionAdd2));
		CHECK(vm.pop().number == 5);
	}

	TEST_CASE("[vm] Increment") {
		VM vm;
		vm.push(Value::make_number(5));
		vm.execute(make_single_op(ActionIncrement));
		CHECK(vm.pop().number == 6);
	}

	TEST_CASE("[vm] Decrement") {
		VM vm;
		vm.push(Value::make_number(5));
		vm.execute(make_single_op(ActionDecrement));
		CHECK(vm.pop().number == 4);
	}

	TEST_CASE("[vm] Equals(5,5)") {
		VM vm;
		vm.push(Value::make_number(5));
		vm.push(Value::make_number(5));
		vm.execute(make_single_op(ActionEquals));
		CHECK(vm.pop().to_bool() == true);
	}

	TEST_CASE("[vm] Less(3,5)") {
		VM vm;
		vm.push(Value::make_number(3));
		vm.push(Value::make_number(5));
		vm.execute(make_single_op(ActionLess));
		CHECK(vm.pop().to_bool() == true);
	}

	TEST_CASE("[vm] Less(5,3) -> false") {
		VM vm;
		vm.push(Value::make_number(5));
		vm.push(Value::make_number(3));
		vm.execute(make_single_op(ActionLess));
		CHECK(vm.pop().to_bool() == false);
	}

	TEST_CASE("[vm] Equals2 null==undefined") {
		VM vm;
		vm.push(Value::make_null());
		vm.push(Value::make_undefined());
		vm.execute(make_single_op(ActionEquals2));
		CHECK(vm.pop().to_bool() == true);
	}

	TEST_CASE("[vm] StrictEquals 0===false -> false") {
		VM vm;
		vm.push(Value::make_number(0));
		vm.push(Value::make_boolean(false));
		vm.execute(make_single_op(ActionStrictEquals));
		CHECK(vm.pop().boolean == false);
	}

	TEST_CASE("[vm] And(1,0)") {
		VM vm;
		vm.push(Value::make_number(1));
		vm.push(Value::make_number(0));
		vm.execute(make_single_op(ActionAnd));
		CHECK(vm.pop().boolean == false);
	}

	TEST_CASE("[vm] Or(0,1)") {
		VM vm;
		vm.push(Value::make_number(0));
		vm.push(Value::make_number(1));
		vm.execute(make_single_op(ActionOr));
		CHECK(vm.pop().boolean == true);
	}

	TEST_CASE("[vm] Not(true)") {
		VM vm;
		vm.push(Value::make_boolean(true));
		vm.execute(make_single_op(ActionNot));
		CHECK(vm.pop().boolean == false);
	}

	TEST_CASE("[vm] BitAnd 0xFF & 0x0F") {
		VM vm;
		vm.push(Value::make_number(0xFF));
		vm.push(Value::make_number(0x0F));
		vm.execute(make_single_op(ActionBitAnd));
		CHECK(vm.pop().number == 0x0F);
	}

	TEST_CASE("[vm] BitOr 0xFF | 0x100") {
		VM vm;
		vm.push(Value::make_number(0xFF));
		vm.push(Value::make_number(0x100));
		vm.execute(make_single_op(ActionBitOr));
		CHECK(vm.pop().number == 0x1FF);
	}

	TEST_CASE("[vm] BitLShift 1<<4=16") {
		VM vm;
		vm.push(Value::make_number(1));
		vm.push(Value::make_number(4));
		vm.execute(make_single_op(ActionBitLShift));
		CHECK(vm.pop().number == 16);
	}

	TEST_CASE("[vm] BitURShift -1>>>0") {
		VM vm;
		vm.push(Value::make_number(-1));
		vm.push(Value::make_number(0));
		vm.execute(make_single_op(ActionBitURShift));
		CHECK(vm.pop().number == (double)0xFFFFFFFF);
	}

	TEST_CASE("[vm] StringAdd") {
		VM vm;
		vm.push(Value::make_string("a"));
		vm.push(Value::make_string("b"));
		vm.execute(make_single_op(ActionStringAdd));
		CHECK(strcmp(vm.pop().string, "ab") == 0);
	}

	TEST_CASE("[vm] StringLength") {
		VM vm;
		vm.push(Value::make_string("hello"));
		vm.execute(make_single_op(ActionStringLength));
		CHECK(vm.pop().number == 5);
	}

	TEST_CASE("[vm] StringExtract") {
		VM vm;
		vm.push(Value::make_string("hello"));
		vm.push(Value::make_number(1));
		vm.push(Value::make_number(3));
		vm.execute(make_single_op(ActionStringExtract));
		CHECK(strcmp(vm.pop().string, "ell") == 0);
	}

	TEST_CASE("[vm] GetVariable/SetVariable") {
		VM vm;
		MockVMCallbacks cb;
		vm.callbacks = &cb;

		vm.push(Value::make_string("myVar"));
		vm.push(Value::make_number(42));
		vm.execute(make_single_op(ActionSetVariable));

		vm.push(Value::make_string("myVar"));
		vm.execute(make_single_op(ActionGetVariable));
		CHECK(vm.pop().number == 42);
	}

	TEST_CASE("[vm] StoreRegister") {
		VM vm;
		vm.push(Value::make_number(99));

		ActionList actions;
		ActionRecord sr; sr.opcode = ActionStoreRegister; sr.offset = 0; sr.register_index = 2;
		actions.push_back(sr);
		ActionRecord end; end.opcode = 0; end.offset = 1;
		actions.push_back(end);
		vm.execute(actions);

		CHECK(vm.registers[2].number == 99);
		CHECK(vm.stack_top == 1); // StoreRegister peeks
	}

	TEST_CASE("[vm] ConstantPool with push") {
		VM vm;

		ActionRecord cp_ar;
		cp_ar.opcode = ActionConstantPool; cp_ar.offset = 0;
		cp_ar.constant_pool.push_back("hello");
		cp_ar.constant_pool.push_back("world");

		ActionRecord push_ar;
		push_ar.opcode = ActionPush; push_ar.offset = 10;
		PushValue pv; pv.type = PUSH_CONSTANT8; pv.constant8 = 1;
		push_ar.push_values.push_back(pv);

		ActionRecord end_ar; end_ar.opcode = 0; end_ar.offset = 20;

		ActionList actions;
		actions.push_back(cp_ar);
		actions.push_back(push_ar);
		actions.push_back(end_ar);
		vm.execute(actions);

		CHECK(strcmp(vm.pop().string, "world") == 0);
	}

	TEST_CASE("[vm] Timeline: Play") {
		VM vm; MockVMCallbacks cb; vm.callbacks = &cb;
		vm.execute(make_single_op(ActionPlay));
		REQUIRE(cb.timeline_actions.size() == 1);
		CHECK(cb.timeline_actions[0].command == TC_PLAY);
	}

	TEST_CASE("[vm] Timeline: Stop") {
		VM vm; MockVMCallbacks cb; vm.callbacks = &cb;
		vm.execute(make_single_op(ActionStop));
		REQUIRE(cb.timeline_actions.size() == 1);
		CHECK(cb.timeline_actions[0].command == TC_STOP);
	}

	TEST_CASE("[vm] Timeline: NextFrame") {
		VM vm; MockVMCallbacks cb; vm.callbacks = &cb;
		vm.execute(make_single_op(ActionNextFrame));
		REQUIRE(cb.timeline_actions.size() == 1);
		CHECK(cb.timeline_actions[0].command == TC_NEXT_FRAME);
	}

	TEST_CASE("[vm] Timeline: GotoFrame(5)") {
		VM vm; MockVMCallbacks cb; vm.callbacks = &cb;
		ActionList actions;
		ActionRecord ar; ar.opcode = ActionGotoFrame; ar.offset = 0; ar.frame = 5;
		actions.push_back(ar);
		ActionRecord end; end.opcode = 0; end.offset = 5;
		actions.push_back(end);
		vm.execute(actions);
		REQUIRE(cb.timeline_actions.size() == 1);
		CHECK(cb.timeline_actions[0].command == TC_GOTO_AND_STOP);
		CHECK(cb.timeline_actions[0].target_frame == 5);
	}

	TEST_CASE("[vm] GoToLabel") {
		VM vm; MockVMCallbacks cb; vm.callbacks = &cb;
		ActionList actions;
		ActionRecord ar; ar.opcode = ActionGoToLabel; ar.offset = 0; ar.string1 = "myLabel";
		actions.push_back(ar);
		ActionRecord end; end.opcode = 0; end.offset = 10;
		actions.push_back(end);
		vm.execute(actions);
		REQUIRE(cb.timeline_actions.size() == 1);
		CHECK(strcmp(cb.timeline_actions[0].target_label, "myLabel") == 0);
	}

	TEST_CASE("[vm] GotoFrame2 with play flag") {
		VM vm; MockVMCallbacks cb; vm.callbacks = &cb;
		vm.push(Value::make_number(10));
		ActionList actions;
		ActionRecord ar; ar.opcode = ActionGotoFrame2; ar.offset = 0; ar.play_flag = true;
		actions.push_back(ar);
		ActionRecord end; end.opcode = 0; end.offset = 5;
		actions.push_back(end);
		vm.execute(actions);
		REQUIRE(cb.timeline_actions.size() == 1);
		CHECK(cb.timeline_actions[0].command == TC_GOTO_AND_PLAY);
		CHECK(cb.timeline_actions[0].target_frame == 10);
	}

	TEST_CASE("[vm] SetTarget affects timeline actions") {
		VM vm; MockVMCallbacks cb; vm.callbacks = &cb;
		ActionList actions;
		ActionRecord st; st.opcode = ActionSetTarget; st.offset = 0; st.string1 = "/mc1";
		actions.push_back(st);
		ActionRecord play; play.opcode = ActionPlay; play.offset = 10;
		actions.push_back(play);
		ActionRecord end; end.opcode = 0; end.offset = 11;
		actions.push_back(end);
		vm.execute(actions);
		REQUIRE(cb.timeline_actions.size() == 1);
		CHECK(strcmp(cb.timeline_actions[0].target_path, "/mc1") == 0);
	}

	TEST_CASE("[vm] GetProperty/SetProperty") {
		VM vm; MockVMCallbacks cb; vm.callbacks = &cb;
		vm.push(Value::make_string(""));
		vm.push(Value::make_number(PROP_X));
		vm.push(Value::make_number(100));
		vm.execute(make_single_op(ActionSetProperty));
		auto key = std::make_pair(std::string(""), (int)PROP_X);
		CHECK(cb.properties[key].number == 100);
	}

	TEST_CASE("[vm] Trace") {
		VM vm; MockVMCallbacks cb; vm.callbacks = &cb;
		vm.push(Value::make_string("debug message"));
		vm.execute(make_single_op(ActionTrace));
		REQUIRE(cb.trace_messages.size() == 1);
		CHECK(cb.trace_messages[0] == "debug message");
	}

	TEST_CASE("[vm] RandomNumber in range") {
		VM vm;
		vm.push(Value::make_number(100));
		vm.execute(make_single_op(ActionRandomNumber));
		Value r = vm.pop();
		CHECK(r.type == Value::NUMBER);
		CHECK(r.number >= 0);
		CHECK(r.number < 100);
	}

	TEST_CASE("[vm] GetTime returns number") {
		VM vm;
		vm.execute(make_single_op(ActionGetTime));
		CHECK(vm.pop().type == Value::NUMBER);
	}

	TEST_CASE("[vm] TypeOf") {
		VM vm;

		SUBCASE("number") {
			vm.push(Value::make_number(42));
			vm.execute(make_single_op(ActionTypeOf));
			CHECK(strcmp(vm.pop().string, "number") == 0);
		}
		SUBCASE("string") {
			vm.push(Value::make_string("hi"));
			vm.execute(make_single_op(ActionTypeOf));
			CHECK(strcmp(vm.pop().string, "string") == 0);
		}
		SUBCASE("boolean") {
			vm.push(Value::make_boolean(true));
			vm.execute(make_single_op(ActionTypeOf));
			CHECK(strcmp(vm.pop().string, "boolean") == 0);
		}
		SUBCASE("undefined") {
			vm.push(Value::make_undefined());
			vm.execute(make_single_op(ActionTypeOf));
			CHECK(strcmp(vm.pop().string, "undefined") == 0);
		}
		SUBCASE("null") {
			vm.push(Value::make_null());
			vm.execute(make_single_op(ActionTypeOf));
			CHECK(strcmp(vm.pop().string, "null") == 0);
		}
	}
}

#endif // DOCTEST
