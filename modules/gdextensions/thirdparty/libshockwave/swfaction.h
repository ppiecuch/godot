#ifndef LIBSHOCKWAVE_SWF_ACTION_H
#define LIBSHOCKWAVE_SWF_ACTION_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

namespace SWF
{
	enum ActionCode : uint8_t
	{
		// SWF3 simple actions
		ActionNextFrame			= 0x04,
		ActionPreviousFrame		= 0x05,
		ActionPlay				= 0x06,
		ActionStop				= 0x07,
		ActionToggleQuality		= 0x08,
		ActionStopSounds		= 0x09,

		// SWF4 stack-based
		ActionAdd				= 0x0A,
		ActionSubtract			= 0x0B,
		ActionMultiply			= 0x0C,
		ActionDivide			= 0x0D,
		ActionEquals			= 0x0E,
		ActionLess				= 0x0F,
		ActionAnd				= 0x10,
		ActionOr				= 0x11,
		ActionNot				= 0x12,
		ActionStringEquals		= 0x13,
		ActionStringLength		= 0x14,
		ActionStringExtract		= 0x15,
		ActionPop				= 0x17,
		ActionToInteger			= 0x18,
		ActionGetVariable		= 0x1C,
		ActionSetVariable		= 0x1D,
		ActionSetTarget2		= 0x20,
		ActionStringAdd			= 0x21,
		ActionGetProperty		= 0x22,
		ActionSetProperty		= 0x23,
		ActionCloneSprite		= 0x24,
		ActionRemoveSprite		= 0x25,
		ActionTrace				= 0x26,
		ActionStartDrag			= 0x27,
		ActionEndDrag			= 0x28,
		ActionStringLess		= 0x29,
		ActionRandomNumber		= 0x30,
		ActionMBStringLength	= 0x31,
		ActionCharToAscii		= 0x32,
		ActionAsciiToChar		= 0x33,
		ActionGetTime			= 0x34,
		ActionMBStringExtract	= 0x35,
		ActionMBCharToAscii		= 0x36,
		ActionMBAsciiToChar		= 0x37,

		// SWF5 object model
		ActionDelete			= 0x3A,
		ActionDefineLocal		= 0x3C,
		ActionCallFunction		= 0x3D,
		ActionReturn			= 0x3E,
		ActionModulo			= 0x3F,
		ActionNewObject			= 0x40,
		ActionDefineLocal2		= 0x41,
		ActionInitArray			= 0x42,
		ActionInitObject		= 0x43,
		ActionTypeOf			= 0x44,
		ActionTargetPath		= 0x45,
		ActionEnumerate			= 0x46,
		ActionAdd2				= 0x47,
		ActionLess2				= 0x48,
		ActionEquals2			= 0x49,
		ActionToNumber			= 0x4A,
		ActionToString			= 0x4B,
		ActionPushDuplicate		= 0x4C,
		ActionStackSwap			= 0x4D,
		ActionGetMember			= 0x4E,
		ActionSetMember			= 0x4F,
		ActionIncrement			= 0x50,
		ActionDecrement			= 0x51,
		ActionCallMethod		= 0x52,
		ActionNewMethod			= 0x53,

		// SWF6
		ActionInstanceOf		= 0x54,
		ActionEnumerate2		= 0x55,

		// SWF5 bitwise
		ActionBitAnd			= 0x60,
		ActionBitOr				= 0x61,
		ActionBitXor			= 0x62,
		ActionBitLShift			= 0x63,
		ActionBitRShift			= 0x64,
		ActionBitURShift		= 0x65,

		// SWF6 comparison
		ActionStrictEquals		= 0x66,
		ActionGreater			= 0x67,
		ActionStringGreater		= 0x68,

		// SWF7
		ActionThrow				= 0x2A,
		ActionCastOp			= 0x2B,
		ActionImplementsOp		= 0x2C,
		ActionExtends			= 0x69,

		// Actions with data (>= 0x80)
		ActionGotoFrame			= 0x81,
		ActionGetURL			= 0x83,
		ActionStoreRegister		= 0x87,
		ActionConstantPool		= 0x88,
		ActionWaitForFrame		= 0x8A,
		ActionSetTarget			= 0x8B,
		ActionGoToLabel			= 0x8C,
		ActionWaitForFrame2		= 0x8D,
		ActionDefineFunction2	= 0x8E,
		ActionTry				= 0x8F,
		ActionWith				= 0x94,
		ActionPush				= 0x96,
		ActionJump				= 0x99,
		ActionGetURL2			= 0x9A,
		ActionDefineFunction	= 0x9B,
		ActionIf				= 0x9D,
		ActionCall				= 0x9E,
		ActionGotoFrame2		= 0x9F
	};

	enum PushType : uint8_t
	{
		PUSH_STRING		= 0,
		PUSH_FLOAT		= 1,
		PUSH_NULL		= 2,
		PUSH_UNDEFINED	= 3,
		PUSH_REGISTER	= 4,
		PUSH_BOOLEAN	= 5,
		PUSH_DOUBLE		= 6,
		PUSH_INTEGER	= 7,
		PUSH_CONSTANT8	= 8,
		PUSH_CONSTANT16	= 9
	};

	struct PushValue
	{
		PushType type = PUSH_UNDEFINED;
		union {
			float float_value;
			double double_value;
			int32_t integer_value;
			uint8_t register_index;
			bool boolean_value;
			uint8_t constant8;
			uint16_t constant16;
		};
		const char *string_value = nullptr;

		PushValue() : float_value(0) {}
	};

	struct FunctionParam
	{
		uint8_t register_index = 0;
		const char *name = nullptr;
	};

	struct ActionRecord;
	typedef std::vector<ActionRecord> ActionList;

	struct FunctionDef
	{
		const char *name = nullptr;
		uint16_t num_params = 0;
		std::vector<FunctionParam> params;
		uint8_t register_count = 0;
		uint16_t preload_flags = 0;
		uint16_t code_size = 0;
		ActionList body;
	};

	struct TryDef
	{
		bool has_catch = false;
		bool has_finally = false;
		bool catch_in_register = false;
		uint16_t try_size = 0;
		uint16_t catch_size = 0;
		uint16_t finally_size = 0;
		uint8_t catch_register = 0;
		const char *catch_name = nullptr;
	};

	struct ActionRecord
	{
		uint8_t opcode = 0;
		uint16_t length = 0;
		uint32_t offset = 0;

		// Decoded payload fields
		uint16_t frame = 0;
		int16_t branch_offset = 0;
		uint8_t register_index = 0;
		uint8_t url2_flags = 0;
		uint8_t skip_count = 0;
		bool play_flag = false;
		uint16_t scene_bias = 0;

		const char *string1 = nullptr;
		const char *string2 = nullptr;

		std::vector<PushValue> push_values;
		std::vector<const char *> constant_pool;
		FunctionDef function_def;
		TryDef try_def;
		uint16_t with_size = 0;
	};

	struct ActionBlock
	{
		uint16_t sprite_id = 0;
		uint16_t frame = 0;
		bool is_init = false;
		ActionList actions;
	};
	typedef std::vector<ActionBlock> ActionBlockList;

	// Clip event flag constants
	enum ClipEventFlag : uint32_t
	{
		ClipEventLoad			= 0x00001,
		ClipEventEnterFrame		= 0x00002,
		ClipEventUnload			= 0x00004,
		ClipEventMouseMove		= 0x00008,
		ClipEventMouseDown		= 0x00010,
		ClipEventMouseUp		= 0x00020,
		ClipEventKeyDown		= 0x00040,
		ClipEventKeyUp			= 0x00080,
		ClipEventData			= 0x00100,
		ClipEventInitialize		= 0x00200,
		ClipEventPress			= 0x00400,
		ClipEventRelease		= 0x00800,
		ClipEventReleaseOutside	= 0x01000,
		ClipEventRollOver		= 0x02000,
		ClipEventRollOut		= 0x04000,
		ClipEventDragOver		= 0x08000,
		ClipEventDragOut		= 0x10000,
		ClipEventKeyPress		= 0x20000,
		ClipEventConstruct		= 0x40000
	};

	struct ClipActionRecord
	{
		uint32_t event_flags = 0;
		uint8_t key_code = 0;
		ActionList actions;
	};

	struct ClipActions
	{
		uint32_t all_event_flags = 0;
		std::vector<ClipActionRecord> records;
	};

}

#endif // LIBSHOCKWAVE_SWF_ACTION_H
