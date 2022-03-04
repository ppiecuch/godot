const PROTO_VERSION = 3

#
# BSD 3-Clause License
#
# Copyright (c) 2018, Oleg Malyavkin
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# DEBUG_TAB redefine this "  " if you need, example: const DEBUG_TAB = "\t"
const DEBUG_TAB = "  "

enum PB_ERR {
	NO_ERRORS = 0,
	VARINT_NOT_FOUND = -1,
	REPEATED_COUNT_NOT_FOUND = -2,
	REPEATED_COUNT_MISMATCH = -3,
	LENGTHDEL_SIZE_NOT_FOUND = -4,
	LENGTHDEL_SIZE_MISMATCH = -5,
	PACKAGE_SIZE_MISMATCH = -6,
	UNDEFINED_STATE = -7,
	PARSE_INCOMPLETE = -8,
	REQUIRED_FIELDS = -9
}

enum PB_DATA_TYPE {
	INT32 = 0,
	SINT32 = 1,
	UINT32 = 2,
	INT64 = 3,
	SINT64 = 4,
	UINT64 = 5,
	BOOL = 6,
	ENUM = 7,
	FIXED32 = 8,
	SFIXED32 = 9,
	FLOAT = 10,
	FIXED64 = 11,
	SFIXED64 = 12,
	DOUBLE = 13,
	STRING = 14,
	BYTES = 15,
	MESSAGE = 16,
	MAP = 17
}

const DEFAULT_VALUES_2 = {
	PB_DATA_TYPE.INT32: null,
	PB_DATA_TYPE.SINT32: null,
	PB_DATA_TYPE.UINT32: null,
	PB_DATA_TYPE.INT64: null,
	PB_DATA_TYPE.SINT64: null,
	PB_DATA_TYPE.UINT64: null,
	PB_DATA_TYPE.BOOL: null,
	PB_DATA_TYPE.ENUM: null,
	PB_DATA_TYPE.FIXED32: null,
	PB_DATA_TYPE.SFIXED32: null,
	PB_DATA_TYPE.FLOAT: null,
	PB_DATA_TYPE.FIXED64: null,
	PB_DATA_TYPE.SFIXED64: null,
	PB_DATA_TYPE.DOUBLE: null,
	PB_DATA_TYPE.STRING: null,
	PB_DATA_TYPE.BYTES: null,
	PB_DATA_TYPE.MESSAGE: null,
	PB_DATA_TYPE.MAP: null
}

const DEFAULT_VALUES_3 = {
	PB_DATA_TYPE.INT32: 0,
	PB_DATA_TYPE.SINT32: 0,
	PB_DATA_TYPE.UINT32: 0,
	PB_DATA_TYPE.INT64: 0,
	PB_DATA_TYPE.SINT64: 0,
	PB_DATA_TYPE.UINT64: 0,
	PB_DATA_TYPE.BOOL: false,
	PB_DATA_TYPE.ENUM: 0,
	PB_DATA_TYPE.FIXED32: 0,
	PB_DATA_TYPE.SFIXED32: 0,
	PB_DATA_TYPE.FLOAT: 0.0,
	PB_DATA_TYPE.FIXED64: 0,
	PB_DATA_TYPE.SFIXED64: 0,
	PB_DATA_TYPE.DOUBLE: 0.0,
	PB_DATA_TYPE.STRING: "",
	PB_DATA_TYPE.BYTES: [],
	PB_DATA_TYPE.MESSAGE: null,
	PB_DATA_TYPE.MAP: []
}

enum PB_TYPE {
	VARINT = 0,
	FIX64 = 1,
	LENGTHDEL = 2,
	STARTGROUP = 3,
	ENDGROUP = 4,
	FIX32 = 5,
	UNDEFINED = 8
}

enum PB_RULE {
	OPTIONAL = 0,
	REQUIRED = 1,
	REPEATED = 2,
	RESERVED = 3
}

enum PB_SERVICE_STATE {
	FILLED = 0,
	UNFILLED = 1
}

class PBField:
	func _init(a_name, a_type, a_rule, a_tag, packed, a_value = null):
		name = a_name
		type = a_type
		rule = a_rule
		tag = a_tag
		option_packed = packed
		value = a_value
	var name
	var type
	var rule
	var tag
	var option_packed
	var value
	var option_default = false

class PBLengthDelimitedField:
	var type = null
	var tag = null
	var begin = null
	var size = null

class PBUnpackedField:
	var offset
	var field

class PBTypeTag:
	var type = null
	var tag = null
	var offset = null

class PBServiceField:
	var field
	var func_ref = null
	var state = PB_SERVICE_STATE.UNFILLED

class PBPacker:
	static func convert_signed(n):
		if n < -2147483648:
			return (n << 1) ^ (n >> 63)
		else:
			return (n << 1) ^ (n >> 31)

	static func deconvert_signed(n):
		if n & 0x01:
			return ~(n >> 1)
		else:
			return (n >> 1)

	static func pack_varint(value):
		var varint = PoolByteArray()
		if typeof(value) == TYPE_BOOL:
			if value:
				value = 1
			else:
				value = 0
		for i in range(9):
			var b = value & 0x7F
			value >>= 7
			if value:
				varint.append(b | 0x80)
			else:
				varint.append(b)
				break
		if varint.size() == 9 && varint[8] == 0xFF:
			varint.append(0x01)
		return varint

	static func pack_bytes(value, count, data_type):
		var bytes = PoolByteArray()
		if data_type == PB_DATA_TYPE.FLOAT:
			var spb = StreamPeerBuffer.new()
			spb.put_float(value)
			bytes = spb.get_data_array()
		elif data_type == PB_DATA_TYPE.DOUBLE:
			var spb = StreamPeerBuffer.new()
			spb.put_double(value)
			bytes = spb.get_data_array()
		else:
			for i in range(count):
				bytes.append(value & 0xFF)
				value >>= 8
		return bytes

	static func unpack_bytes(bytes, index, count, data_type):
		var value = 0
		if data_type == PB_DATA_TYPE.FLOAT:
			var spb = StreamPeerBuffer.new()
			for i in range(index, count + index):
				spb.put_u8(bytes[i])
			spb.seek(0)
			value = spb.get_float()
		elif data_type == PB_DATA_TYPE.DOUBLE:
			var spb = StreamPeerBuffer.new()
			for i in range(index, count + index):
				spb.put_u8(bytes[i])
			spb.seek(0)
			value = spb.get_double()
		else:
			for i in range(index + count - 1, index - 1, -1):
				value |= (bytes[i] & 0xFF)
				if i != index:
					value <<= 8
		return value

	static func unpack_varint(varint_bytes):
		var value = 0
		for i in range(varint_bytes.size() - 1, -1, -1):
			value |= varint_bytes[i] & 0x7F
			if i != 0:
				value <<= 7
		return value

	static func pack_type_tag(type, tag):
		return pack_varint((tag << 3) | type)

	static func isolate_varint(bytes, index):
		var result = PoolByteArray()
		for i in range(index, bytes.size()):
			result.append(bytes[i])
			if !(bytes[i] & 0x80):
				break
		return result

	static func unpack_type_tag(bytes, index):
		var varint_bytes = isolate_varint(bytes, index)
		var result = PBTypeTag.new()
		if varint_bytes.size() != 0:
			result.offset = varint_bytes.size()
			var unpacked = unpack_varint(varint_bytes)
			result.type = unpacked & 0x07
			result.tag = unpacked >> 3
		return result

	static func pack_length_delimeted(type, tag, bytes):
		var result = pack_type_tag(type, tag)
		result.append_array(pack_varint(bytes.size()))
		result.append_array(bytes)
		return result

	static func unpack_length_delimiter(bytes, index):
		var result = PBLengthDelimitedField.new()
		var type_tag = unpack_type_tag(bytes, index)
		var offset = type_tag.offset
		if offset != null:
			result.type = type_tag.type
			result.tag = type_tag.tag
			var size = isolate_varint(bytes, offset)
			if size > 0:
				offset += size
				if bytes.size() >= size + offset:
					result.begin = offset
					result.size = size
		return result

	static func pb_type_from_data_type(data_type):
		if data_type == PB_DATA_TYPE.INT32 || data_type == PB_DATA_TYPE.SINT32 || data_type == PB_DATA_TYPE.UINT32 || data_type == PB_DATA_TYPE.INT64 || data_type == PB_DATA_TYPE.SINT64 || data_type == PB_DATA_TYPE.UINT64 || data_type == PB_DATA_TYPE.BOOL || data_type == PB_DATA_TYPE.ENUM:
			return PB_TYPE.VARINT
		elif data_type == PB_DATA_TYPE.FIXED32 || data_type == PB_DATA_TYPE.SFIXED32 || data_type == PB_DATA_TYPE.FLOAT:
			return PB_TYPE.FIX32
		elif data_type == PB_DATA_TYPE.FIXED64 || data_type == PB_DATA_TYPE.SFIXED64 || data_type == PB_DATA_TYPE.DOUBLE:
			return PB_TYPE.FIX64
		elif data_type == PB_DATA_TYPE.STRING || data_type == PB_DATA_TYPE.BYTES || data_type == PB_DATA_TYPE.MESSAGE || data_type == PB_DATA_TYPE.MAP:
			return PB_TYPE.LENGTHDEL
		else:
			return PB_TYPE.UNDEFINED

	static func pack_field(field):
		var type = pb_type_from_data_type(field.type)
		var type_copy = type
		if field.rule == PB_RULE.REPEATED && field.option_packed:
			type = PB_TYPE.LENGTHDEL
		var head = pack_type_tag(type, field.tag)
		var data = PoolByteArray()
		if type == PB_TYPE.VARINT:
			var value
			if field.rule == PB_RULE.REPEATED:
				for v in field.value:
					data.append_array(head)
					if field.type == PB_DATA_TYPE.SINT32 || field.type == PB_DATA_TYPE.SINT64:
						value = convert_signed(v)
					else:
						value = v
					data.append_array(pack_varint(value))
				return data
			else:
				if field.type == PB_DATA_TYPE.SINT32 || field.type == PB_DATA_TYPE.SINT64:
					value = convert_signed(field.value)
				else:
					value = field.value
				data = pack_varint(value)
		elif type == PB_TYPE.FIX32:
			if field.rule == PB_RULE.REPEATED:
				for v in field.value:
					data.append_array(head)
					data.append_array(pack_bytes(v, 4, field.type))
				return data
			else:
				data.append_array(pack_bytes(field.value, 4, field.type))
		elif type == PB_TYPE.FIX64:
			if field.rule == PB_RULE.REPEATED:
				for v in field.value:
					data.append_array(head)
					data.append_array(pack_bytes(v, 8, field.type))
				return data
			else:
				data.append_array(pack_bytes(field.value, 8, field.type))
		elif type == PB_TYPE.LENGTHDEL:
			if field.rule == PB_RULE.REPEATED:
				if type_copy == PB_TYPE.VARINT:
					if field.type == PB_DATA_TYPE.SINT32 || field.type == PB_DATA_TYPE.SINT64:
						var signed_value
						for v in field.value:
							signed_value = convert_signed(v)
							data.append_array(pack_varint(signed_value))
					else:
						for v in field.value:
							data.append_array(pack_varint(v))
					return pack_length_delimeted(type, field.tag, data)
				elif type_copy == PB_TYPE.FIX32:
					for v in field.value:
						data.append_array(pack_bytes(v, 4, field.type))
					return pack_length_delimeted(type, field.tag, data)
				elif type_copy == PB_TYPE.FIX64:
					for v in field.value:
						data.append_array(pack_bytes(v, 8, field.type))
					return pack_length_delimeted(type, field.tag, data)
				elif field.type == PB_DATA_TYPE.STRING:
					for v in field.value:
						var obj = v.to_utf8()
						data.append_array(pack_length_delimeted(type, field.tag, obj))
					return data
				elif field.type == PB_DATA_TYPE.BYTES:
					for v in field.value:
						data.append_array(pack_length_delimeted(type, field.tag, v))
					return data
				elif typeof(field.value[0]) == TYPE_OBJECT:
					for v in field.value:
						var obj = v.to_bytes()
						#if obj != null && obj.size() > 0:
						#	data.append_array(pack_length_delimeted(type, field.tag, obj))
						#else:
						#	data = PoolByteArray()
						#	return data
						if obj != null:#
							data.append_array(pack_length_delimeted(type, field.tag, obj))#
						else:#
							data = PoolByteArray()#
							return data#
					return data
			else:
				if field.type == PB_DATA_TYPE.STRING:
					var str_bytes = field.value.to_utf8()
					if PROTO_VERSION == 2 || (PROTO_VERSION == 3 && str_bytes.size() > 0):
						data.append_array(str_bytes)
						return pack_length_delimeted(type, field.tag, data)
				if field.type == PB_DATA_TYPE.BYTES:
					if PROTO_VERSION == 2 || (PROTO_VERSION == 3 && field.value.size() > 0):
						data.append_array(field.value)
						return pack_length_delimeted(type, field.tag, data)
				elif typeof(field.value) == TYPE_OBJECT:
					var obj = field.value.to_bytes()
					#if obj != null && obj.size() > 0:
					#	data.append_array(obj)
					#	return pack_length_delimeted(type, field.tag, data)
					if obj != null:#
						if obj.size() > 0:#
							data.append_array(obj)#
						return pack_length_delimeted(type, field.tag, data)#
				else:
					pass
		if data.size() > 0:
			head.append_array(data)
			return head
		else:
			return data

	static func unpack_field(bytes, offset, field, type, message_func_ref):
		if field.rule == PB_RULE.REPEATED && type != PB_TYPE.LENGTHDEL && field.option_packed:
			var count = isolate_varint(bytes, offset)
			if count.size() > 0:
				offset += count.size()
				count = unpack_varint(count)
				if type == PB_TYPE.VARINT:
					var val
					var counter = offset + count
					while offset < counter:
						val = isolate_varint(bytes, offset)
						if val.size() > 0:
							offset += val.size()
							val = unpack_varint(val)
							if field.type == PB_DATA_TYPE.SINT32 || field.type == PB_DATA_TYPE.SINT64:
								val = deconvert_signed(val)
							elif field.type == PB_DATA_TYPE.BOOL:
								if val:
									val = true
								else:
									val = false
							field.value.append(val)
						else:
							return PB_ERR.REPEATED_COUNT_MISMATCH
					return offset
				elif type == PB_TYPE.FIX32 || type == PB_TYPE.FIX64:
					var type_size
					if type == PB_TYPE.FIX32:
						type_size = 4
					else:
						type_size = 8
					var val
					var counter = offset + count
					while offset < counter:
						if (offset + type_size) > bytes.size():
							return PB_ERR.REPEATED_COUNT_MISMATCH
						val = unpack_bytes(bytes, offset, type_size, field.type)
						offset += type_size
						field.value.append(val)
					return offset
			else:
				return PB_ERR.REPEATED_COUNT_NOT_FOUND
		else:
			if type == PB_TYPE.VARINT:
				var val = isolate_varint(bytes, offset)
				if val.size() > 0:
					offset += val.size()
					val = unpack_varint(val)
					if field.type == PB_DATA_TYPE.SINT32 || field.type == PB_DATA_TYPE.SINT64:
						val = deconvert_signed(val)
					elif field.type == PB_DATA_TYPE.BOOL:
						if val:
							val = true
						else:
							val = false
					if field.rule == PB_RULE.REPEATED:
						field.value.append(val)
					else:
						field.value = val
				else:
					return PB_ERR.VARINT_NOT_FOUND
				return offset
			elif type == PB_TYPE.FIX32 || type == PB_TYPE.FIX64:
				var type_size
				if type == PB_TYPE.FIX32:
					type_size = 4
				else:
					type_size = 8
				var val
				if (offset + type_size) > bytes.size():
					return PB_ERR.REPEATED_COUNT_MISMATCH
				val = unpack_bytes(bytes, offset, type_size, field.type)
				offset += type_size
				if field.rule == PB_RULE.REPEATED:
					field.value.append(val)
				else:
					field.value = val
				return offset
			elif type == PB_TYPE.LENGTHDEL:
				var inner_size = isolate_varint(bytes, offset)
				if inner_size.size() > 0:
					offset += inner_size.size()
					inner_size = unpack_varint(inner_size)
					if inner_size >= 0:
						if inner_size + offset > bytes.size():
							return PB_ERR.LENGTHDEL_SIZE_MISMATCH
						if message_func_ref != null:
							var message = message_func_ref.call_func()
							if inner_size > 0:
								var sub_offset = message.from_bytes(bytes, offset, inner_size + offset)
								if sub_offset > 0:
									if sub_offset - offset >= inner_size:
										offset = sub_offset
										return offset
									else:
										return PB_ERR.LENGTHDEL_SIZE_MISMATCH
								return sub_offset
							else:
								return offset
						elif field.type == PB_DATA_TYPE.STRING:
							var str_bytes = PoolByteArray()
							for i in range(offset, inner_size + offset):
								str_bytes.append(bytes[i])
							if field.rule == PB_RULE.REPEATED:
								field.value.append(str_bytes.get_string_from_utf8())
							else:
								field.value = str_bytes.get_string_from_utf8()
							return offset + inner_size
						elif field.type == PB_DATA_TYPE.BYTES:
							var val_bytes = PoolByteArray()
							for i in range(offset, inner_size + offset):
								val_bytes.append(bytes[i])
							if field.rule == PB_RULE.REPEATED:
								field.value.append(val_bytes)
							else:
								field.value = val_bytes
							return offset + inner_size
					else:
						return PB_ERR.LENGTHDEL_SIZE_NOT_FOUND
				else:
					return PB_ERR.LENGTHDEL_SIZE_NOT_FOUND
		return PB_ERR.UNDEFINED_STATE

	static func unpack_message(data, bytes, offset, limit):
		while true:
			var tt = unpack_type_tag(bytes, offset)
			if tt.offset != null:
				offset += tt.offset
				if data.has(tt.tag):
					var service = data[tt.tag]
					var type = pb_type_from_data_type(service.field.type)
					if type == tt.type || (tt.type == PB_TYPE.LENGTHDEL && service.field.rule == PB_RULE.REPEATED && service.field.option_packed):
						var res = unpack_field(bytes, offset, service.field, type, service.func_ref)
						if res > 0:
							service.state = PB_SERVICE_STATE.FILLED
							offset = res
							if offset == limit:
								return offset
							elif offset > limit:
								return PB_ERR.PACKAGE_SIZE_MISMATCH
						elif res < 0:
							return res
						else:
							break
			else:
				return offset
		return PB_ERR.UNDEFINED_STATE

	static func pack_message(data):
		var DEFAULT_VALUES
		if PROTO_VERSION == 2:
			DEFAULT_VALUES = DEFAULT_VALUES_2
		elif PROTO_VERSION == 3:
			DEFAULT_VALUES = DEFAULT_VALUES_3
		var result = PoolByteArray()
		var keys = data.keys()
		keys.sort()
		for i in keys:
			if data[i].field.value != null:
				if typeof(data[i].field.value) == typeof(DEFAULT_VALUES[data[i].field.type]) && data[i].field.value == DEFAULT_VALUES[data[i].field.type]:
					continue
				elif data[i].field.rule == PB_RULE.REPEATED && data[i].field.value.size() == 0:
					continue
				result.append_array(pack_field(data[i].field))
			elif data[i].field.rule == PB_RULE.REQUIRED:
				print("Error: required field is not filled: Tag:", data[i].field.tag)
				return null
		return result

	static func check_required(data):
		var keys = data.keys()
		for i in keys:
			if data[i].field.rule == PB_RULE.REQUIRED && data[i].state == PB_SERVICE_STATE.UNFILLED:
				return false
		return true

	static func construct_map(key_values):
		var result = {}
		for kv in key_values:
			result[kv.get_key()] = kv.get_value()
		return result
	
	static func tabulate(text, nesting):
		var tab = ""
		for i in range(nesting):
			tab += DEBUG_TAB
		return tab + text
	
	static func value_to_string(value, field, nesting):
		var result = ""
		var text
		if field.type == PB_DATA_TYPE.MESSAGE:
			result += "{"
			nesting += 1
			text = message_to_string(value.data, nesting)
			if text != "":
				result += "\n" + text
				nesting -= 1
				result += tabulate("}", nesting)
			else:
				nesting -= 1
				result += "}"
		elif field.type == PB_DATA_TYPE.BYTES:
			result += "<"
			for i in range(value.size()):
				result += String(value[i])
				if i != (value.size() - 1):
					result += ", "
			result += ">"
		elif field.type == PB_DATA_TYPE.STRING:
			result += "\"" + value + "\""
		elif field.type == PB_DATA_TYPE.ENUM:
			result += "ENUM::" + String(value)
		else:
			result += String(value)
		return result
	
	static func field_to_string(field, nesting):
		var result = tabulate(field.name + ": ", nesting)
		if field.type == PB_DATA_TYPE.MAP:
			if field.value.size() > 0:
				result += "(\n"
				nesting += 1
				for i in range(field.value.size()):
					var local_key_value = field.value[i].data[1].field
					result += tabulate(value_to_string(local_key_value.value, local_key_value, nesting), nesting) + ": "
					local_key_value = field.value[i].data[2].field
					result += value_to_string(local_key_value.value, local_key_value, nesting)
					if i != (field.value.size() - 1):
						result += ","
					result += "\n"
				nesting -= 1
				result += tabulate(")", nesting)
			else:
				result += "()"
		elif field.rule == PB_RULE.REPEATED:
			if field.value.size() > 0:
				result += "[\n"
				nesting += 1
				for i in range(field.value.size()):
					result += tabulate(String(i) + ": ", nesting)
					result += value_to_string(field.value[i], field, nesting)
					if i != (field.value.size() - 1):
						result += ","
					result += "\n"
				nesting -= 1
				result += tabulate("]", nesting)
			else:
				result += "[]"
		else:
			result += value_to_string(field.value, field, nesting)
		result += ";\n"
		return result
		
	static func message_to_string(data, nesting = 0):
		var DEFAULT_VALUES
		if PROTO_VERSION == 2:
			DEFAULT_VALUES = DEFAULT_VALUES_2
		elif PROTO_VERSION == 3:
			DEFAULT_VALUES = DEFAULT_VALUES_3
		var result = ""
		var keys = data.keys()
		keys.sort()
		for i in keys:
			if data[i].field.value != null:
				if typeof(data[i].field.value) == typeof(DEFAULT_VALUES[data[i].field.type]) && data[i].field.value == DEFAULT_VALUES[data[i].field.type]:
					continue
				elif data[i].field.rule == PB_RULE.REPEATED && data[i].field.value.size() == 0:
					continue
				result += field_to_string(data[i].field, nesting)
			elif data[i].field.rule == PB_RULE.REQUIRED:
				result += data[i].field.name + ": " + "error"
		return result


############### USER DATA BEGIN ################


class Heartbeat:
	func _init():
		var service
		
		_timestamp = PBField.new("timestamp", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _timestamp
		data[_timestamp.tag] = service
		
	var data = {}
	
	var _timestamp
	func get_timestamp():
		return _timestamp.value
	func clear_timestamp():
		_timestamp.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_timestamp(value):
		_timestamp.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class Error:
	func _init():
		var service
		
		_code = PBField.new("code", PB_DATA_TYPE.INT32, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT32])
		service = PBServiceField.new()
		service.field = _code
		data[_code.tag] = service
		
		_msg = PBField.new("msg", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _msg
		data[_msg.tag] = service
		
	var data = {}
	
	var _code
	func get_code():
		return _code.value
	func clear_code():
		_code.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT32]
	func set_code(value):
		_code.value = value
	
	var _msg
	func get_msg():
		return _msg.value
	func clear_msg():
		_msg.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_msg(value):
		_msg.value = value
	
	enum Code {
		RUNTIME_EXCEPTION = 0,
		UNRECOGNIZED_PAYLOAD = 1,
		MISSING_PAYLOAD = 2,
		BAD_INPUT = 3,
		AUTH_ERROR = 4,
		USER_NOT_FOUND = 5,
		USER_REGISTER_INUSE = 6,
		USER_LINK_INUSE = 7,
		USER_LINK_PROVIDER_UNAVAILABLE = 8,
		USER_UNLINK_DISALLOWED = 9,
		USER_HANDLE_INUSE = 10,
		GROUP_NAME_INUSE = 11,
		GROUP_LAST_ADMIN = 12,
		STORAGE_REJECTED = 13,
		MATCH_NOT_FOUND = 14,
		RUNTIME_FUNCTION_NOT_FOUND = 15,
		RUNTIME_FUNCTION_EXCEPTION = 16
	}
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class AuthenticateRequest:
	func _init():
		var service
		
		_collationId = PBField.new("collationId", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _collationId
		data[_collationId.tag] = service
		
		_email = PBField.new("email", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _email
		service.func_ref = funcref(self, "new_email")
		data[_email.tag] = service
		
		_facebook = PBField.new("facebook", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _facebook
		data[_facebook.tag] = service
		
		_google = PBField.new("google", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _google
		data[_google.tag] = service
		
		_game_center = PBField.new("game_center", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _game_center
		service.func_ref = funcref(self, "new_game_center")
		data[_game_center.tag] = service
		
		_steam = PBField.new("steam", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _steam
		data[_steam.tag] = service
		
		_device = PBField.new("device", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _device
		data[_device.tag] = service
		
		_custom = PBField.new("custom", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _custom
		data[_custom.tag] = service
		
	var data = {}
	
	var _collationId
	func get_collationId():
		return _collationId.value
	func clear_collationId():
		_collationId.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_collationId(value):
		_collationId.value = value
	
	var _email
	func has_email():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_email():
		return _email.value
	func clear_email():
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_email():
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_email.value = AuthenticateRequest.Email.new()
		return _email.value
	
	var _facebook
	func has_facebook():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_facebook():
		return _facebook.value
	func clear_facebook():
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_facebook(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_facebook.value = value
	
	var _google
	func has_google():
		if data[4].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_google():
		return _google.value
	func clear_google():
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_google(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = value
	
	var _game_center
	func has_game_center():
		if data[5].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_game_center():
		return _game_center.value
	func clear_game_center():
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_game_center():
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = AuthenticateRequest.GameCenter.new()
		return _game_center.value
	
	var _steam
	func has_steam():
		if data[6].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_steam():
		return _steam.value
	func clear_steam():
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_steam(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = value
	
	var _device
	func has_device():
		if data[7].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_device():
		return _device.value
	func clear_device():
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_device(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = value
	
	var _custom
	func has_custom():
		if data[8].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_custom():
		return _custom.value
	func clear_custom():
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_custom(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = value
	
	class Email:
		func _init():
			var service
			
			_email = PBField.new("email", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _email
			data[_email.tag] = service
			
			_password = PBField.new("password", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _password
			data[_password.tag] = service
			
		var data = {}
		
		var _email
		func get_email():
			return _email.value
		func clear_email():
			_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_email(value):
			_email.value = value
		
		var _password
		func get_password():
			return _password.value
		func clear_password():
			_password.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_password(value):
			_password.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	class GameCenter:
		func _init():
			var service
			
			_player_id = PBField.new("player_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _player_id
			data[_player_id.tag] = service
			
			_bundle_id = PBField.new("bundle_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _bundle_id
			data[_bundle_id.tag] = service
			
			_timestamp = PBField.new("timestamp", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _timestamp
			data[_timestamp.tag] = service
			
			_salt = PBField.new("salt", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _salt
			data[_salt.tag] = service
			
			_signature = PBField.new("signature", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _signature
			data[_signature.tag] = service
			
			_public_key_url = PBField.new("public_key_url", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _public_key_url
			data[_public_key_url.tag] = service
			
		var data = {}
		
		var _player_id
		func get_player_id():
			return _player_id.value
		func clear_player_id():
			_player_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_player_id(value):
			_player_id.value = value
		
		var _bundle_id
		func get_bundle_id():
			return _bundle_id.value
		func clear_bundle_id():
			_bundle_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_bundle_id(value):
			_bundle_id.value = value
		
		var _timestamp
		func get_timestamp():
			return _timestamp.value
		func clear_timestamp():
			_timestamp.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_timestamp(value):
			_timestamp.value = value
		
		var _salt
		func get_salt():
			return _salt.value
		func clear_salt():
			_salt.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_salt(value):
			_salt.value = value
		
		var _signature
		func get_signature():
			return _signature.value
		func clear_signature():
			_signature.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_signature(value):
			_signature.value = value
		
		var _public_key_url
		func get_public_key_url():
			return _public_key_url.value
		func clear_public_key_url():
			_public_key_url.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_public_key_url(value):
			_public_key_url.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class AuthenticateResponse:
	func _init():
		var service
		
		_collation_id = PBField.new("collation_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _collation_id
		data[_collation_id.tag] = service
		
		_session = PBField.new("session", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _session
		service.func_ref = funcref(self, "new_session")
		data[_session.tag] = service
		
		_error = PBField.new("error", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _error
		service.func_ref = funcref(self, "new_error")
		data[_error.tag] = service
		
	var data = {}
	
	var _collation_id
	func get_collation_id():
		return _collation_id.value
	func clear_collation_id():
		_collation_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_collation_id(value):
		_collation_id.value = value
	
	var _session
	func has_session():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_session():
		return _session.value
	func clear_session():
		_session.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_session():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_session.value = AuthenticateResponse.Session.new()
		return _session.value
	
	var _error
	func has_error():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_error():
		return _error.value
	func clear_error():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_error():
		_session.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_error.value = AuthenticateResponse.Error.new()
		return _error.value
	
	class Session:
		func _init():
			var service
			
			_token = PBField.new("token", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _token
			data[_token.tag] = service
			
			_udp_token = PBField.new("udp_token", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _udp_token
			data[_udp_token.tag] = service
			
		var data = {}
		
		var _token
		func get_token():
			return _token.value
		func clear_token():
			_token.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_token(value):
			_token.value = value
		
		var _udp_token
		func get_udp_token():
			return _udp_token.value
		func clear_udp_token():
			_udp_token.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_udp_token(value):
			_udp_token.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	class Error:
		func _init():
			var service
			
			_code = PBField.new("code", PB_DATA_TYPE.INT32, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT32])
			service = PBServiceField.new()
			service.field = _code
			data[_code.tag] = service
			
			_msg = PBField.new("msg", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _msg
			data[_msg.tag] = service
			
			_request = PBField.new("request", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
			service = PBServiceField.new()
			service.field = _request
			service.func_ref = funcref(self, "new_request")
			data[_request.tag] = service
			
		var data = {}
		
		var _code
		func get_code():
			return _code.value
		func clear_code():
			_code.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT32]
		func set_code(value):
			_code.value = value
		
		var _msg
		func get_msg():
			return _msg.value
		func clear_msg():
			_msg.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_msg(value):
			_msg.value = value
		
		var _request
		func get_request():
			return _request.value
		func clear_request():
			_request.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		func new_request():
			_request.value = AuthenticateRequest.new()
			return _request.value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class Envelope:
	func _init():
		var service
		
		_collation_id = PBField.new("collation_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _collation_id
		data[_collation_id.tag] = service
		
		_error = PBField.new("error", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _error
		service.func_ref = funcref(self, "new_error")
		data[_error.tag] = service
		
		_heartbeat = PBField.new("heartbeat", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _heartbeat
		service.func_ref = funcref(self, "new_heartbeat")
		data[_heartbeat.tag] = service
		
		_logout = PBField.new("logout", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _logout
		service.func_ref = funcref(self, "new_logout")
		data[_logout.tag] = service
		
		_link = PBField.new("link", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _link
		service.func_ref = funcref(self, "new_link")
		data[_link.tag] = service
		
		_unlink = PBField.new("unlink", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _unlink
		service.func_ref = funcref(self, "new_unlink")
		data[_unlink.tag] = service
		
		_self_fetch = PBField.new("self_fetch", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _self_fetch
		service.func_ref = funcref(self, "new_self_fetch")
		data[_self_fetch.tag] = service
		
		_self_update = PBField.new("self_update", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _self_update
		service.func_ref = funcref(self, "new_self_update")
		data[_self_update.tag] = service
		
		_users_fetch = PBField.new("users_fetch", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 9, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _users_fetch
		service.func_ref = funcref(self, "new_users_fetch")
		data[_users_fetch.tag] = service
		
		_self = PBField.new("self", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 10, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _self
		service.func_ref = funcref(self, "new_self")
		data[_self.tag] = service
		
		_users = PBField.new("users", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 11, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _users
		service.func_ref = funcref(self, "new_users")
		data[_users.tag] = service
		
		_friends_add = PBField.new("friends_add", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 12, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _friends_add
		service.func_ref = funcref(self, "new_friends_add")
		data[_friends_add.tag] = service
		
		_friends_remove = PBField.new("friends_remove", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 13, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _friends_remove
		service.func_ref = funcref(self, "new_friends_remove")
		data[_friends_remove.tag] = service
		
		_friends_block = PBField.new("friends_block", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 14, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _friends_block
		service.func_ref = funcref(self, "new_friends_block")
		data[_friends_block.tag] = service
		
		_friends_list = PBField.new("friends_list", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 15, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _friends_list
		service.func_ref = funcref(self, "new_friends_list")
		data[_friends_list.tag] = service
		
		_friends = PBField.new("friends", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 16, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _friends
		service.func_ref = funcref(self, "new_friends")
		data[_friends.tag] = service
		
		_groups_create = PBField.new("groups_create", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 17, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _groups_create
		service.func_ref = funcref(self, "new_groups_create")
		data[_groups_create.tag] = service
		
		_groups_update = PBField.new("groups_update", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 18, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _groups_update
		service.func_ref = funcref(self, "new_groups_update")
		data[_groups_update.tag] = service
		
		_groups_remove = PBField.new("groups_remove", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 19, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _groups_remove
		service.func_ref = funcref(self, "new_groups_remove")
		data[_groups_remove.tag] = service
		
		_groups_fetch = PBField.new("groups_fetch", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 20, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _groups_fetch
		service.func_ref = funcref(self, "new_groups_fetch")
		data[_groups_fetch.tag] = service
		
		_groups_list = PBField.new("groups_list", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 21, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _groups_list
		service.func_ref = funcref(self, "new_groups_list")
		data[_groups_list.tag] = service
		
		_groups_self_list = PBField.new("groups_self_list", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 22, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _groups_self_list
		service.func_ref = funcref(self, "new_groups_self_list")
		data[_groups_self_list.tag] = service
		
		_group_users_list = PBField.new("group_users_list", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 23, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _group_users_list
		service.func_ref = funcref(self, "new_group_users_list")
		data[_group_users_list.tag] = service
		
		_groups_join = PBField.new("groups_join", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 24, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _groups_join
		service.func_ref = funcref(self, "new_groups_join")
		data[_groups_join.tag] = service
		
		_groups_leave = PBField.new("groups_leave", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 25, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _groups_leave
		service.func_ref = funcref(self, "new_groups_leave")
		data[_groups_leave.tag] = service
		
		_group_users_add = PBField.new("group_users_add", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 26, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _group_users_add
		service.func_ref = funcref(self, "new_group_users_add")
		data[_group_users_add.tag] = service
		
		_group_users_kick = PBField.new("group_users_kick", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 27, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _group_users_kick
		service.func_ref = funcref(self, "new_group_users_kick")
		data[_group_users_kick.tag] = service
		
		_group_users_promote = PBField.new("group_users_promote", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 28, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _group_users_promote
		service.func_ref = funcref(self, "new_group_users_promote")
		data[_group_users_promote.tag] = service
		
		_groups = PBField.new("groups", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 29, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _groups
		service.func_ref = funcref(self, "new_groups")
		data[_groups.tag] = service
		
		_groups_self = PBField.new("groups_self", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 30, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _groups_self
		service.func_ref = funcref(self, "new_groups_self")
		data[_groups_self.tag] = service
		
		_group_users = PBField.new("group_users", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 31, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _group_users
		service.func_ref = funcref(self, "new_group_users")
		data[_group_users.tag] = service
		
		_topics_join = PBField.new("topics_join", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 32, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topics_join
		service.func_ref = funcref(self, "new_topics_join")
		data[_topics_join.tag] = service
		
		_topics_leave = PBField.new("topics_leave", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 33, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topics_leave
		service.func_ref = funcref(self, "new_topics_leave")
		data[_topics_leave.tag] = service
		
		_topic_message_send = PBField.new("topic_message_send", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 34, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topic_message_send
		service.func_ref = funcref(self, "new_topic_message_send")
		data[_topic_message_send.tag] = service
		
		_topic_messages_list = PBField.new("topic_messages_list", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 35, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topic_messages_list
		service.func_ref = funcref(self, "new_topic_messages_list")
		data[_topic_messages_list.tag] = service
		
		_topics = PBField.new("topics", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 36, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topics
		service.func_ref = funcref(self, "new_topics")
		data[_topics.tag] = service
		
		_topic_message_ack = PBField.new("topic_message_ack", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 37, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topic_message_ack
		service.func_ref = funcref(self, "new_topic_message_ack")
		data[_topic_message_ack.tag] = service
		
		_topic_message = PBField.new("topic_message", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 38, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topic_message
		service.func_ref = funcref(self, "new_topic_message")
		data[_topic_message.tag] = service
		
		_topic_messages = PBField.new("topic_messages", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 39, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topic_messages
		service.func_ref = funcref(self, "new_topic_messages")
		data[_topic_messages.tag] = service
		
		_topic_presence = PBField.new("topic_presence", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 40, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topic_presence
		service.func_ref = funcref(self, "new_topic_presence")
		data[_topic_presence.tag] = service
		
		_match_create = PBField.new("match_create", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 41, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _match_create
		service.func_ref = funcref(self, "new_match_create")
		data[_match_create.tag] = service
		
		_matches_join = PBField.new("matches_join", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 42, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _matches_join
		service.func_ref = funcref(self, "new_matches_join")
		data[_matches_join.tag] = service
		
		_matches_leave = PBField.new("matches_leave", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 43, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _matches_leave
		service.func_ref = funcref(self, "new_matches_leave")
		data[_matches_leave.tag] = service
		
		_match_data_send = PBField.new("match_data_send", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 44, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _match_data_send
		service.func_ref = funcref(self, "new_match_data_send")
		data[_match_data_send.tag] = service
		
		_match = PBField.new("match", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 45, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _match
		service.func_ref = funcref(self, "new_match")
		data[_match.tag] = service
		
		_matches = PBField.new("matches", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 46, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _matches
		service.func_ref = funcref(self, "new_matches")
		data[_matches.tag] = service
		
		_match_data = PBField.new("match_data", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 47, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _match_data
		service.func_ref = funcref(self, "new_match_data")
		data[_match_data.tag] = service
		
		_match_presence = PBField.new("match_presence", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 48, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _match_presence
		service.func_ref = funcref(self, "new_match_presence")
		data[_match_presence.tag] = service
		
		_storage_list = PBField.new("storage_list", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 49, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _storage_list
		service.func_ref = funcref(self, "new_storage_list")
		data[_storage_list.tag] = service
		
		_storage_fetch = PBField.new("storage_fetch", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 50, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _storage_fetch
		service.func_ref = funcref(self, "new_storage_fetch")
		data[_storage_fetch.tag] = service
		
		_storage_write = PBField.new("storage_write", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 51, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _storage_write
		service.func_ref = funcref(self, "new_storage_write")
		data[_storage_write.tag] = service
		
		_storage_update = PBField.new("storage_update", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 52, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _storage_update
		service.func_ref = funcref(self, "new_storage_update")
		data[_storage_update.tag] = service
		
		_storage_remove = PBField.new("storage_remove", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 53, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _storage_remove
		service.func_ref = funcref(self, "new_storage_remove")
		data[_storage_remove.tag] = service
		
		_storage_data = PBField.new("storage_data", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 54, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _storage_data
		service.func_ref = funcref(self, "new_storage_data")
		data[_storage_data.tag] = service
		
		_storage_keys = PBField.new("storage_keys", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 55, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _storage_keys
		service.func_ref = funcref(self, "new_storage_keys")
		data[_storage_keys.tag] = service
		
		_leaderboards_list = PBField.new("leaderboards_list", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 56, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _leaderboards_list
		service.func_ref = funcref(self, "new_leaderboards_list")
		data[_leaderboards_list.tag] = service
		
		_leaderboard_records_write = PBField.new("leaderboard_records_write", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 57, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _leaderboard_records_write
		service.func_ref = funcref(self, "new_leaderboard_records_write")
		data[_leaderboard_records_write.tag] = service
		
		_leaderboard_records_fetch = PBField.new("leaderboard_records_fetch", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 58, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _leaderboard_records_fetch
		service.func_ref = funcref(self, "new_leaderboard_records_fetch")
		data[_leaderboard_records_fetch.tag] = service
		
		_leaderboard_records_list = PBField.new("leaderboard_records_list", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 59, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _leaderboard_records_list
		service.func_ref = funcref(self, "new_leaderboard_records_list")
		data[_leaderboard_records_list.tag] = service
		
		_leaderboards = PBField.new("leaderboards", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 60, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _leaderboards
		service.func_ref = funcref(self, "new_leaderboards")
		data[_leaderboards.tag] = service
		
		_leaderboard_records = PBField.new("leaderboard_records", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 61, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _leaderboard_records
		service.func_ref = funcref(self, "new_leaderboard_records")
		data[_leaderboard_records.tag] = service
		
		_matchmake_add = PBField.new("matchmake_add", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 62, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _matchmake_add
		service.func_ref = funcref(self, "new_matchmake_add")
		data[_matchmake_add.tag] = service
		
		_matchmake_remove = PBField.new("matchmake_remove", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 63, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _matchmake_remove
		service.func_ref = funcref(self, "new_matchmake_remove")
		data[_matchmake_remove.tag] = service
		
		_matchmake_ticket = PBField.new("matchmake_ticket", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 64, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _matchmake_ticket
		service.func_ref = funcref(self, "new_matchmake_ticket")
		data[_matchmake_ticket.tag] = service
		
		_matchmake_matched = PBField.new("matchmake_matched", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 65, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _matchmake_matched
		service.func_ref = funcref(self, "new_matchmake_matched")
		data[_matchmake_matched.tag] = service
		
		_rpc = PBField.new("rpc", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 66, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _rpc
		service.func_ref = funcref(self, "new_rpc")
		data[_rpc.tag] = service
		
		_purchase = PBField.new("purchase", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 67, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _purchase
		service.func_ref = funcref(self, "new_purchase")
		data[_purchase.tag] = service
		
		_purchase_record = PBField.new("purchase_record", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 68, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _purchase_record
		service.func_ref = funcref(self, "new_purchase_record")
		data[_purchase_record.tag] = service
		
		_notifications_list = PBField.new("notifications_list", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 69, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _notifications_list
		service.func_ref = funcref(self, "new_notifications_list")
		data[_notifications_list.tag] = service
		
		_notifications_remove = PBField.new("notifications_remove", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 70, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _notifications_remove
		service.func_ref = funcref(self, "new_notifications_remove")
		data[_notifications_remove.tag] = service
		
		_notifications = PBField.new("notifications", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 71, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _notifications
		service.func_ref = funcref(self, "new_notifications")
		data[_notifications.tag] = service
		
		_live_notifications = PBField.new("live_notifications", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 72, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _live_notifications
		service.func_ref = funcref(self, "new_live_notifications")
		data[_live_notifications.tag] = service
		
	var data = {}
	
	var _collation_id
	func get_collation_id():
		return _collation_id.value
	func clear_collation_id():
		_collation_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_collation_id(value):
		_collation_id.value = value
	
	var _error
	func has_error():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_error():
		return _error.value
	func clear_error():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_error():
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_error.value = Error.new()
		return _error.value
	
	var _heartbeat
	func has_heartbeat():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_heartbeat():
		return _heartbeat.value
	func clear_heartbeat():
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_heartbeat():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = Heartbeat.new()
		return _heartbeat.value
	
	var _logout
	func has_logout():
		if data[4].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_logout():
		return _logout.value
	func clear_logout():
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_logout():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = Logout.new()
		return _logout.value
	
	var _link
	func has_link():
		if data[5].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_link():
		return _link.value
	func clear_link():
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_link():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = TLink.new()
		return _link.value
	
	var _unlink
	func has_unlink():
		if data[6].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_unlink():
		return _unlink.value
	func clear_unlink():
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_unlink():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = TUnlink.new()
		return _unlink.value
	
	var _self_fetch
	func has_self_fetch():
		if data[7].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_self_fetch():
		return _self_fetch.value
	func clear_self_fetch():
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_self_fetch():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = TSelfFetch.new()
		return _self_fetch.value
	
	var _self_update
	func has_self_update():
		if data[8].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_self_update():
		return _self_update.value
	func clear_self_update():
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_self_update():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = TSelfUpdate.new()
		return _self_update.value
	
	var _users_fetch
	func has_users_fetch():
		if data[9].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_users_fetch():
		return _users_fetch.value
	func clear_users_fetch():
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_users_fetch():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = TUsersFetch.new()
		return _users_fetch.value
	
	var _self
	func has_self():
		if data[10].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_self():
		return _self.value
	func clear_self():
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_self():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = TSelf.new()
		return _self.value
	
	var _users
	func has_users():
		if data[11].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_users():
		return _users.value
	func clear_users():
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_users():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = TUsers.new()
		return _users.value
	
	var _friends_add
	func has_friends_add():
		if data[12].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_friends_add():
		return _friends_add.value
	func clear_friends_add():
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_friends_add():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = TFriendsAdd.new()
		return _friends_add.value
	
	var _friends_remove
	func has_friends_remove():
		if data[13].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_friends_remove():
		return _friends_remove.value
	func clear_friends_remove():
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_friends_remove():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = TFriendsRemove.new()
		return _friends_remove.value
	
	var _friends_block
	func has_friends_block():
		if data[14].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_friends_block():
		return _friends_block.value
	func clear_friends_block():
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_friends_block():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = TFriendsBlock.new()
		return _friends_block.value
	
	var _friends_list
	func has_friends_list():
		if data[15].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_friends_list():
		return _friends_list.value
	func clear_friends_list():
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_friends_list():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = TFriendsList.new()
		return _friends_list.value
	
	var _friends
	func has_friends():
		if data[16].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_friends():
		return _friends.value
	func clear_friends():
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_friends():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = TFriends.new()
		return _friends.value
	
	var _groups_create
	func has_groups_create():
		if data[17].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_groups_create():
		return _groups_create.value
	func clear_groups_create():
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_groups_create():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = TGroupsCreate.new()
		return _groups_create.value
	
	var _groups_update
	func has_groups_update():
		if data[18].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_groups_update():
		return _groups_update.value
	func clear_groups_update():
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_groups_update():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = TGroupsUpdate.new()
		return _groups_update.value
	
	var _groups_remove
	func has_groups_remove():
		if data[19].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_groups_remove():
		return _groups_remove.value
	func clear_groups_remove():
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_groups_remove():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = TGroupsRemove.new()
		return _groups_remove.value
	
	var _groups_fetch
	func has_groups_fetch():
		if data[20].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_groups_fetch():
		return _groups_fetch.value
	func clear_groups_fetch():
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_groups_fetch():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = TGroupsFetch.new()
		return _groups_fetch.value
	
	var _groups_list
	func has_groups_list():
		if data[21].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_groups_list():
		return _groups_list.value
	func clear_groups_list():
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_groups_list():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = TGroupsList.new()
		return _groups_list.value
	
	var _groups_self_list
	func has_groups_self_list():
		if data[22].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_groups_self_list():
		return _groups_self_list.value
	func clear_groups_self_list():
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_groups_self_list():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = TGroupsSelfList.new()
		return _groups_self_list.value
	
	var _group_users_list
	func has_group_users_list():
		if data[23].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_group_users_list():
		return _group_users_list.value
	func clear_group_users_list():
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_group_users_list():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = TGroupUsersList.new()
		return _group_users_list.value
	
	var _groups_join
	func has_groups_join():
		if data[24].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_groups_join():
		return _groups_join.value
	func clear_groups_join():
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_groups_join():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = TGroupsJoin.new()
		return _groups_join.value
	
	var _groups_leave
	func has_groups_leave():
		if data[25].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_groups_leave():
		return _groups_leave.value
	func clear_groups_leave():
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_groups_leave():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = TGroupsLeave.new()
		return _groups_leave.value
	
	var _group_users_add
	func has_group_users_add():
		if data[26].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_group_users_add():
		return _group_users_add.value
	func clear_group_users_add():
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_group_users_add():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = TGroupUsersAdd.new()
		return _group_users_add.value
	
	var _group_users_kick
	func has_group_users_kick():
		if data[27].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_group_users_kick():
		return _group_users_kick.value
	func clear_group_users_kick():
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_group_users_kick():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = TGroupUsersKick.new()
		return _group_users_kick.value
	
	var _group_users_promote
	func has_group_users_promote():
		if data[28].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_group_users_promote():
		return _group_users_promote.value
	func clear_group_users_promote():
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_group_users_promote():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = TGroupUsersPromote.new()
		return _group_users_promote.value
	
	var _groups
	func has_groups():
		if data[29].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_groups():
		return _groups.value
	func clear_groups():
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_groups():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = TGroups.new()
		return _groups.value
	
	var _groups_self
	func has_groups_self():
		if data[30].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_groups_self():
		return _groups_self.value
	func clear_groups_self():
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_groups_self():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = TGroupsSelf.new()
		return _groups_self.value
	
	var _group_users
	func has_group_users():
		if data[31].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_group_users():
		return _group_users.value
	func clear_group_users():
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_group_users():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = TGroupUsers.new()
		return _group_users.value
	
	var _topics_join
	func has_topics_join():
		if data[32].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_topics_join():
		return _topics_join.value
	func clear_topics_join():
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topics_join():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = TTopicsJoin.new()
		return _topics_join.value
	
	var _topics_leave
	func has_topics_leave():
		if data[33].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_topics_leave():
		return _topics_leave.value
	func clear_topics_leave():
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topics_leave():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = TTopicsLeave.new()
		return _topics_leave.value
	
	var _topic_message_send
	func has_topic_message_send():
		if data[34].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_topic_message_send():
		return _topic_message_send.value
	func clear_topic_message_send():
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topic_message_send():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = TTopicMessageSend.new()
		return _topic_message_send.value
	
	var _topic_messages_list
	func has_topic_messages_list():
		if data[35].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_topic_messages_list():
		return _topic_messages_list.value
	func clear_topic_messages_list():
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topic_messages_list():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = TTopicMessagesList.new()
		return _topic_messages_list.value
	
	var _topics
	func has_topics():
		if data[36].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_topics():
		return _topics.value
	func clear_topics():
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topics():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = TTopics.new()
		return _topics.value
	
	var _topic_message_ack
	func has_topic_message_ack():
		if data[37].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_topic_message_ack():
		return _topic_message_ack.value
	func clear_topic_message_ack():
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topic_message_ack():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = TTopicMessageAck.new()
		return _topic_message_ack.value
	
	var _topic_message
	func has_topic_message():
		if data[38].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_topic_message():
		return _topic_message.value
	func clear_topic_message():
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topic_message():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = TopicMessage.new()
		return _topic_message.value
	
	var _topic_messages
	func has_topic_messages():
		if data[39].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_topic_messages():
		return _topic_messages.value
	func clear_topic_messages():
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topic_messages():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = TTopicMessages.new()
		return _topic_messages.value
	
	var _topic_presence
	func has_topic_presence():
		if data[40].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_topic_presence():
		return _topic_presence.value
	func clear_topic_presence():
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topic_presence():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = TopicPresence.new()
		return _topic_presence.value
	
	var _match_create
	func has_match_create():
		if data[41].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_match_create():
		return _match_create.value
	func clear_match_create():
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_match_create():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = TMatchCreate.new()
		return _match_create.value
	
	var _matches_join
	func has_matches_join():
		if data[42].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_matches_join():
		return _matches_join.value
	func clear_matches_join():
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_matches_join():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = TMatchesJoin.new()
		return _matches_join.value
	
	var _matches_leave
	func has_matches_leave():
		if data[43].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_matches_leave():
		return _matches_leave.value
	func clear_matches_leave():
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_matches_leave():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = TMatchesLeave.new()
		return _matches_leave.value
	
	var _match_data_send
	func has_match_data_send():
		if data[44].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_match_data_send():
		return _match_data_send.value
	func clear_match_data_send():
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_match_data_send():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = MatchDataSend.new()
		return _match_data_send.value
	
	var _match
	func has_match():
		if data[45].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_match():
		return _match.value
	func clear_match():
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_match():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = TMatch.new()
		return _match.value
	
	var _matches
	func has_matches():
		if data[46].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_matches():
		return _matches.value
	func clear_matches():
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_matches():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = TMatches.new()
		return _matches.value
	
	var _match_data
	func has_match_data():
		if data[47].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_match_data():
		return _match_data.value
	func clear_match_data():
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_match_data():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = MatchData.new()
		return _match_data.value
	
	var _match_presence
	func has_match_presence():
		if data[48].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_match_presence():
		return _match_presence.value
	func clear_match_presence():
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_match_presence():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = MatchPresence.new()
		return _match_presence.value
	
	var _storage_list
	func has_storage_list():
		if data[49].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_storage_list():
		return _storage_list.value
	func clear_storage_list():
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_storage_list():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = TStorageList.new()
		return _storage_list.value
	
	var _storage_fetch
	func has_storage_fetch():
		if data[50].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_storage_fetch():
		return _storage_fetch.value
	func clear_storage_fetch():
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_storage_fetch():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = TStorageFetch.new()
		return _storage_fetch.value
	
	var _storage_write
	func has_storage_write():
		if data[51].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_storage_write():
		return _storage_write.value
	func clear_storage_write():
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_storage_write():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = TStorageWrite.new()
		return _storage_write.value
	
	var _storage_update
	func has_storage_update():
		if data[52].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_storage_update():
		return _storage_update.value
	func clear_storage_update():
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_storage_update():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = TStorageUpdate.new()
		return _storage_update.value
	
	var _storage_remove
	func has_storage_remove():
		if data[53].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_storage_remove():
		return _storage_remove.value
	func clear_storage_remove():
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_storage_remove():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = TStorageRemove.new()
		return _storage_remove.value
	
	var _storage_data
	func has_storage_data():
		if data[54].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_storage_data():
		return _storage_data.value
	func clear_storage_data():
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_storage_data():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = TStorageData.new()
		return _storage_data.value
	
	var _storage_keys
	func has_storage_keys():
		if data[55].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_storage_keys():
		return _storage_keys.value
	func clear_storage_keys():
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_storage_keys():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = TStorageKeys.new()
		return _storage_keys.value
	
	var _leaderboards_list
	func has_leaderboards_list():
		if data[56].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_leaderboards_list():
		return _leaderboards_list.value
	func clear_leaderboards_list():
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_leaderboards_list():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = TLeaderboardsList.new()
		return _leaderboards_list.value
	
	var _leaderboard_records_write
	func has_leaderboard_records_write():
		if data[57].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_leaderboard_records_write():
		return _leaderboard_records_write.value
	func clear_leaderboard_records_write():
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_leaderboard_records_write():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = TLeaderboardRecordsWrite.new()
		return _leaderboard_records_write.value
	
	var _leaderboard_records_fetch
	func has_leaderboard_records_fetch():
		if data[58].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_leaderboard_records_fetch():
		return _leaderboard_records_fetch.value
	func clear_leaderboard_records_fetch():
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_leaderboard_records_fetch():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = TLeaderboardRecordsFetch.new()
		return _leaderboard_records_fetch.value
	
	var _leaderboard_records_list
	func has_leaderboard_records_list():
		if data[59].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_leaderboard_records_list():
		return _leaderboard_records_list.value
	func clear_leaderboard_records_list():
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_leaderboard_records_list():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = TLeaderboardRecordsList.new()
		return _leaderboard_records_list.value
	
	var _leaderboards
	func has_leaderboards():
		if data[60].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_leaderboards():
		return _leaderboards.value
	func clear_leaderboards():
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_leaderboards():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = TLeaderboards.new()
		return _leaderboards.value
	
	var _leaderboard_records
	func has_leaderboard_records():
		if data[61].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_leaderboard_records():
		return _leaderboard_records.value
	func clear_leaderboard_records():
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_leaderboard_records():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = TLeaderboardRecords.new()
		return _leaderboard_records.value
	
	var _matchmake_add
	func has_matchmake_add():
		if data[62].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_matchmake_add():
		return _matchmake_add.value
	func clear_matchmake_add():
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_matchmake_add():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = TMatchmakeAdd.new()
		return _matchmake_add.value
	
	var _matchmake_remove
	func has_matchmake_remove():
		if data[63].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_matchmake_remove():
		return _matchmake_remove.value
	func clear_matchmake_remove():
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_matchmake_remove():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = TMatchmakeRemove.new()
		return _matchmake_remove.value
	
	var _matchmake_ticket
	func has_matchmake_ticket():
		if data[64].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_matchmake_ticket():
		return _matchmake_ticket.value
	func clear_matchmake_ticket():
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_matchmake_ticket():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = TMatchmakeTicket.new()
		return _matchmake_ticket.value
	
	var _matchmake_matched
	func has_matchmake_matched():
		if data[65].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_matchmake_matched():
		return _matchmake_matched.value
	func clear_matchmake_matched():
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_matchmake_matched():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = MatchmakeMatched.new()
		return _matchmake_matched.value
	
	var _rpc
	func has_rpc():
		if data[66].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_rpc():
		return _rpc.value
	func clear_rpc():
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_rpc():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = TRpc.new()
		return _rpc.value
	
	var _purchase
	func has_purchase():
		if data[67].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_purchase():
		return _purchase.value
	func clear_purchase():
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_purchase():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = TPurchaseValidation.new()
		return _purchase.value
	
	var _purchase_record
	func has_purchase_record():
		if data[68].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_purchase_record():
		return _purchase_record.value
	func clear_purchase_record():
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_purchase_record():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = TPurchaseRecord.new()
		return _purchase_record.value
	
	var _notifications_list
	func has_notifications_list():
		if data[69].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_notifications_list():
		return _notifications_list.value
	func clear_notifications_list():
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_notifications_list():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = TNotificationsList.new()
		return _notifications_list.value
	
	var _notifications_remove
	func has_notifications_remove():
		if data[70].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_notifications_remove():
		return _notifications_remove.value
	func clear_notifications_remove():
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_notifications_remove():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = TNotificationsRemove.new()
		return _notifications_remove.value
	
	var _notifications
	func has_notifications():
		if data[71].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_notifications():
		return _notifications.value
	func clear_notifications():
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_notifications():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = TNotifications.new()
		return _notifications.value
	
	var _live_notifications
	func has_live_notifications():
		if data[72].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_live_notifications():
		return _live_notifications.value
	func clear_live_notifications():
		_live_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_live_notifications():
		_error.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_heartbeat.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_logout.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_link.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_unlink.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_block.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_kick.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users_promote.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message_ack.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_message.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_topic_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_create.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_join.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches_leave.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data_send.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_match_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_update.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_storage_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_fetch.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_leaderboard_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_add.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_matchmake_matched.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_rpc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_purchase_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_list.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications_remove.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_live_notifications.value = Notifications.new()
		return _live_notifications.value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class Logout:
	func _init():
		var service
		
	var data = {}
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TLink:
	func _init():
		var service
		
		_email = PBField.new("email", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _email
		service.func_ref = funcref(self, "new_email")
		data[_email.tag] = service
		
		_facebook = PBField.new("facebook", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _facebook
		data[_facebook.tag] = service
		
		_google = PBField.new("google", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _google
		data[_google.tag] = service
		
		_game_center = PBField.new("game_center", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _game_center
		service.func_ref = funcref(self, "new_game_center")
		data[_game_center.tag] = service
		
		_steam = PBField.new("steam", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _steam
		data[_steam.tag] = service
		
		_device = PBField.new("device", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _device
		data[_device.tag] = service
		
		_custom = PBField.new("custom", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _custom
		data[_custom.tag] = service
		
	var data = {}
	
	var _email
	func has_email():
		if data[1].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_email():
		return _email.value
	func clear_email():
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_email():
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_email.value = AuthenticateRequest.Email.new()
		return _email.value
	
	var _facebook
	func has_facebook():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_facebook():
		return _facebook.value
	func clear_facebook():
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_facebook(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_facebook.value = value
	
	var _google
	func has_google():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_google():
		return _google.value
	func clear_google():
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_google(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = value
	
	var _game_center
	func has_game_center():
		if data[4].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_game_center():
		return _game_center.value
	func clear_game_center():
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_game_center():
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = AuthenticateRequest.GameCenter.new()
		return _game_center.value
	
	var _steam
	func has_steam():
		if data[5].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_steam():
		return _steam.value
	func clear_steam():
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_steam(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = value
	
	var _device
	func has_device():
		if data[6].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_device():
		return _device.value
	func clear_device():
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_device(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = value
	
	var _custom
	func has_custom():
		if data[7].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_custom():
		return _custom.value
	func clear_custom():
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_custom(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TUnlink:
	func _init():
		var service
		
		_email = PBField.new("email", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _email
		data[_email.tag] = service
		
		_facebook = PBField.new("facebook", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _facebook
		data[_facebook.tag] = service
		
		_google = PBField.new("google", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _google
		data[_google.tag] = service
		
		_game_center = PBField.new("game_center", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _game_center
		data[_game_center.tag] = service
		
		_steam = PBField.new("steam", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _steam
		data[_steam.tag] = service
		
		_device = PBField.new("device", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _device
		data[_device.tag] = service
		
		_custom = PBField.new("custom", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _custom
		data[_custom.tag] = service
		
	var data = {}
	
	var _email
	func has_email():
		if data[1].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_email():
		return _email.value
	func clear_email():
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_email(value):
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_email.value = value
	
	var _facebook
	func has_facebook():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_facebook():
		return _facebook.value
	func clear_facebook():
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_facebook(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_facebook.value = value
	
	var _google
	func has_google():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_google():
		return _google.value
	func clear_google():
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_google(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = value
	
	var _game_center
	func has_game_center():
		if data[4].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_game_center():
		return _game_center.value
	func clear_game_center():
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_game_center(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = value
	
	var _steam
	func has_steam():
		if data[5].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_steam():
		return _steam.value
	func clear_steam():
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_steam(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = value
	
	var _device
	func has_device():
		if data[6].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_device():
		return _device.value
	func clear_device():
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_device(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = value
	
	var _custom
	func has_custom():
		if data[7].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_custom():
		return _custom.value
	func clear_custom():
		_custom.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_custom(value):
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_facebook.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_google.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_game_center.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_steam.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_device.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_custom.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class User:
	func _init():
		var service
		
		_id = PBField.new("id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _id
		data[_id.tag] = service
		
		_handle = PBField.new("handle", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _handle
		data[_handle.tag] = service
		
		_fullname = PBField.new("fullname", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _fullname
		data[_fullname.tag] = service
		
		_avatar_url = PBField.new("avatar_url", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _avatar_url
		data[_avatar_url.tag] = service
		
		_lang = PBField.new("lang", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _lang
		data[_lang.tag] = service
		
		_location = PBField.new("location", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _location
		data[_location.tag] = service
		
		_timezone = PBField.new("timezone", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _timezone
		data[_timezone.tag] = service
		
		_metadata = PBField.new("metadata", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _metadata
		data[_metadata.tag] = service
		
		_created_at = PBField.new("created_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 9, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _created_at
		data[_created_at.tag] = service
		
		_updated_at = PBField.new("updated_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 10, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _updated_at
		data[_updated_at.tag] = service
		
		_last_online_at = PBField.new("last_online_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 11, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _last_online_at
		data[_last_online_at.tag] = service
		
	var data = {}
	
	var _id
	func get_id():
		return _id.value
	func clear_id():
		_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_id(value):
		_id.value = value
	
	var _handle
	func get_handle():
		return _handle.value
	func clear_handle():
		_handle.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_handle(value):
		_handle.value = value
	
	var _fullname
	func get_fullname():
		return _fullname.value
	func clear_fullname():
		_fullname.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_fullname(value):
		_fullname.value = value
	
	var _avatar_url
	func get_avatar_url():
		return _avatar_url.value
	func clear_avatar_url():
		_avatar_url.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_avatar_url(value):
		_avatar_url.value = value
	
	var _lang
	func get_lang():
		return _lang.value
	func clear_lang():
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_lang(value):
		_lang.value = value
	
	var _location
	func get_location():
		return _location.value
	func clear_location():
		_location.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_location(value):
		_location.value = value
	
	var _timezone
	func get_timezone():
		return _timezone.value
	func clear_timezone():
		_timezone.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_timezone(value):
		_timezone.value = value
	
	var _metadata
	func get_metadata():
		return _metadata.value
	func clear_metadata():
		_metadata.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_metadata(value):
		_metadata.value = value
	
	var _created_at
	func get_created_at():
		return _created_at.value
	func clear_created_at():
		_created_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_created_at(value):
		_created_at.value = value
	
	var _updated_at
	func get_updated_at():
		return _updated_at.value
	func clear_updated_at():
		_updated_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_updated_at(value):
		_updated_at.value = value
	
	var _last_online_at
	func get_last_online_at():
		return _last_online_at.value
	func clear_last_online_at():
		_last_online_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_last_online_at(value):
		_last_online_at.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class Self:
	func _init():
		var service
		
		_user = PBField.new("user", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _user
		service.func_ref = funcref(self, "new_user")
		data[_user.tag] = service
		
		_verified = PBField.new("verified", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _verified
		data[_verified.tag] = service
		
		_email = PBField.new("email", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _email
		data[_email.tag] = service
		
		_device_ids = PBField.new("device_ids", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 4, true, [])
		service = PBServiceField.new()
		service.field = _device_ids
		data[_device_ids.tag] = service
		
		_facebook_id = PBField.new("facebook_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _facebook_id
		data[_facebook_id.tag] = service
		
		_google_id = PBField.new("google_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _google_id
		data[_google_id.tag] = service
		
		_gamecenter_id = PBField.new("gamecenter_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _gamecenter_id
		data[_gamecenter_id.tag] = service
		
		_steam_id = PBField.new("steam_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _steam_id
		data[_steam_id.tag] = service
		
		_custom_id = PBField.new("custom_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 9, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _custom_id
		data[_custom_id.tag] = service
		
	var data = {}
	
	var _user
	func get_user():
		return _user.value
	func clear_user():
		_user.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_user():
		_user.value = User.new()
		return _user.value
	
	var _verified
	func get_verified():
		return _verified.value
	func clear_verified():
		_verified.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_verified(value):
		_verified.value = value
	
	var _email
	func get_email():
		return _email.value
	func clear_email():
		_email.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_email(value):
		_email.value = value
	
	var _device_ids
	func get_device_ids():
		return _device_ids.value
	func clear_device_ids():
		_device_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func add_device_ids(value):
		_device_ids.value.append(value)
	
	var _facebook_id
	func get_facebook_id():
		return _facebook_id.value
	func clear_facebook_id():
		_facebook_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_facebook_id(value):
		_facebook_id.value = value
	
	var _google_id
	func get_google_id():
		return _google_id.value
	func clear_google_id():
		_google_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_google_id(value):
		_google_id.value = value
	
	var _gamecenter_id
	func get_gamecenter_id():
		return _gamecenter_id.value
	func clear_gamecenter_id():
		_gamecenter_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_gamecenter_id(value):
		_gamecenter_id.value = value
	
	var _steam_id
	func get_steam_id():
		return _steam_id.value
	func clear_steam_id():
		_steam_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_steam_id(value):
		_steam_id.value = value
	
	var _custom_id
	func get_custom_id():
		return _custom_id.value
	func clear_custom_id():
		_custom_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_custom_id(value):
		_custom_id.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TSelfFetch:
	func _init():
		var service
		
	var data = {}
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TSelf:
	func _init():
		var service
		
		_self = PBField.new("self", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _self
		service.func_ref = funcref(self, "new_self")
		data[_self.tag] = service
		
	var data = {}
	
	var _self
	func get_self():
		return _self.value
	func clear_self():
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_self():
		_self.value = Self.new()
		return _self.value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TSelfUpdate:
	func _init():
		var service
		
		_handle = PBField.new("handle", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _handle
		data[_handle.tag] = service
		
		_fullname = PBField.new("fullname", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _fullname
		data[_fullname.tag] = service
		
		_timezone = PBField.new("timezone", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _timezone
		data[_timezone.tag] = service
		
		_location = PBField.new("location", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _location
		data[_location.tag] = service
		
		_lang = PBField.new("lang", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _lang
		data[_lang.tag] = service
		
		_metadata = PBField.new("metadata", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _metadata
		data[_metadata.tag] = service
		
		_avatar_url = PBField.new("avatar_url", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _avatar_url
		data[_avatar_url.tag] = service
		
	var data = {}
	
	var _handle
	func get_handle():
		return _handle.value
	func clear_handle():
		_handle.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_handle(value):
		_handle.value = value
	
	var _fullname
	func get_fullname():
		return _fullname.value
	func clear_fullname():
		_fullname.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_fullname(value):
		_fullname.value = value
	
	var _timezone
	func get_timezone():
		return _timezone.value
	func clear_timezone():
		_timezone.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_timezone(value):
		_timezone.value = value
	
	var _location
	func get_location():
		return _location.value
	func clear_location():
		_location.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_location(value):
		_location.value = value
	
	var _lang
	func get_lang():
		return _lang.value
	func clear_lang():
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_lang(value):
		_lang.value = value
	
	var _metadata
	func get_metadata():
		return _metadata.value
	func clear_metadata():
		_metadata.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_metadata(value):
		_metadata.value = value
	
	var _avatar_url
	func get_avatar_url():
		return _avatar_url.value
	func clear_avatar_url():
		_avatar_url.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_avatar_url(value):
		_avatar_url.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TUsersFetch:
	func _init():
		var service
		
		_users = PBField.new("users", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _users
		service.func_ref = funcref(self, "add_users")
		data[_users.tag] = service
		
	var data = {}
	
	var _users
	func get_users():
		return _users.value
	func clear_users():
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_users():
		var element = TUsersFetch.UsersFetch.new()
		_users.value.append(element)
		return element
	
	class UsersFetch:
		func _init():
			var service
			
			_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _user_id
			data[_user_id.tag] = service
			
			_handle = PBField.new("handle", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _handle
			data[_handle.tag] = service
			
		var data = {}
		
		var _user_id
		func has_user_id():
			if data[1].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_user_id():
			return _user_id.value
		func clear_user_id():
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_user_id(value):
			_handle.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_user_id.value = value
		
		var _handle
		func has_handle():
			if data[2].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_handle():
			return _handle.value
		func clear_handle():
			_handle.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_handle(value):
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_handle.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TUsers:
	func _init():
		var service
		
		_users = PBField.new("users", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _users
		service.func_ref = funcref(self, "add_users")
		data[_users.tag] = service
		
	var data = {}
	
	var _users
	func get_users():
		return _users.value
	func clear_users():
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_users():
		var element = User.new()
		_users.value.append(element)
		return element
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class Friend:
	func _init():
		var service
		
		_user = PBField.new("user", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _user
		service.func_ref = funcref(self, "new_user")
		data[_user.tag] = service
		
		_state = PBField.new("state", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _state
		data[_state.tag] = service
		
	var data = {}
	
	var _user
	func get_user():
		return _user.value
	func clear_user():
		_user.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_user():
		_user.value = User.new()
		return _user.value
	
	var _state
	func get_state():
		return _state.value
	func clear_state():
		_state.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_state(value):
		_state.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TFriendsAdd:
	func _init():
		var service
		
		_friends = PBField.new("friends", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _friends
		service.func_ref = funcref(self, "add_friends")
		data[_friends.tag] = service
		
	var data = {}
	
	var _friends
	func get_friends():
		return _friends.value
	func clear_friends():
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_friends():
		var element = TFriendsAdd.FriendsAdd.new()
		_friends.value.append(element)
		return element
	
	class FriendsAdd:
		func _init():
			var service
			
			_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _user_id
			data[_user_id.tag] = service
			
			_handle = PBField.new("handle", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _handle
			data[_handle.tag] = service
			
		var data = {}
		
		var _user_id
		func has_user_id():
			if data[1].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_user_id():
			return _user_id.value
		func clear_user_id():
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_user_id(value):
			_handle.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_user_id.value = value
		
		var _handle
		func has_handle():
			if data[2].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_handle():
			return _handle.value
		func clear_handle():
			_handle.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_handle(value):
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_handle.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TFriendsRemove:
	func _init():
		var service
		
		_user_ids = PBField.new("user_ids", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _user_ids
		data[_user_ids.tag] = service
		
	var data = {}
	
	var _user_ids
	func get_user_ids():
		return _user_ids.value
	func clear_user_ids():
		_user_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func add_user_ids(value):
		_user_ids.value.append(value)
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TFriendsBlock:
	func _init():
		var service
		
		_user_ids = PBField.new("user_ids", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _user_ids
		data[_user_ids.tag] = service
		
	var data = {}
	
	var _user_ids
	func get_user_ids():
		return _user_ids.value
	func clear_user_ids():
		_user_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func add_user_ids(value):
		_user_ids.value.append(value)
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TFriendsList:
	func _init():
		var service
		
	var data = {}
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TFriends:
	func _init():
		var service
		
		_friends = PBField.new("friends", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _friends
		service.func_ref = funcref(self, "add_friends")
		data[_friends.tag] = service
		
	var data = {}
	
	var _friends
	func get_friends():
		return _friends.value
	func clear_friends():
		_friends.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_friends():
		var element = Friend.new()
		_friends.value.append(element)
		return element
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class Group:
	func _init():
		var service
		
		_id = PBField.new("id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _id
		data[_id.tag] = service
		
		_private = PBField.new("private", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _private
		data[_private.tag] = service
		
		_creator_id = PBField.new("creator_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _creator_id
		data[_creator_id.tag] = service
		
		_name = PBField.new("name", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _name
		data[_name.tag] = service
		
		_description = PBField.new("description", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _description
		data[_description.tag] = service
		
		_avatar_url = PBField.new("avatar_url", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _avatar_url
		data[_avatar_url.tag] = service
		
		_lang = PBField.new("lang", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _lang
		data[_lang.tag] = service
		
		_utc_offset_ms = PBField.new("utc_offset_ms", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _utc_offset_ms
		data[_utc_offset_ms.tag] = service
		
		_metadata = PBField.new("metadata", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 9, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _metadata
		data[_metadata.tag] = service
		
		_count = PBField.new("count", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 10, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _count
		data[_count.tag] = service
		
		_created_at = PBField.new("created_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 11, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _created_at
		data[_created_at.tag] = service
		
		_updated_at = PBField.new("updated_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 12, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _updated_at
		data[_updated_at.tag] = service
		
	var data = {}
	
	var _id
	func get_id():
		return _id.value
	func clear_id():
		_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_id(value):
		_id.value = value
	
	var _private
	func get_private():
		return _private.value
	func clear_private():
		_private.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_private(value):
		_private.value = value
	
	var _creator_id
	func get_creator_id():
		return _creator_id.value
	func clear_creator_id():
		_creator_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_creator_id(value):
		_creator_id.value = value
	
	var _name
	func get_name():
		return _name.value
	func clear_name():
		_name.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_name(value):
		_name.value = value
	
	var _description
	func get_description():
		return _description.value
	func clear_description():
		_description.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_description(value):
		_description.value = value
	
	var _avatar_url
	func get_avatar_url():
		return _avatar_url.value
	func clear_avatar_url():
		_avatar_url.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_avatar_url(value):
		_avatar_url.value = value
	
	var _lang
	func get_lang():
		return _lang.value
	func clear_lang():
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_lang(value):
		_lang.value = value
	
	var _utc_offset_ms
	func get_utc_offset_ms():
		return _utc_offset_ms.value
	func clear_utc_offset_ms():
		_utc_offset_ms.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_utc_offset_ms(value):
		_utc_offset_ms.value = value
	
	var _metadata
	func get_metadata():
		return _metadata.value
	func clear_metadata():
		_metadata.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_metadata(value):
		_metadata.value = value
	
	var _count
	func get_count():
		return _count.value
	func clear_count():
		_count.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_count(value):
		_count.value = value
	
	var _created_at
	func get_created_at():
		return _created_at.value
	func clear_created_at():
		_created_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_created_at(value):
		_created_at.value = value
	
	var _updated_at
	func get_updated_at():
		return _updated_at.value
	func clear_updated_at():
		_updated_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_updated_at(value):
		_updated_at.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupsCreate:
	func _init():
		var service
		
		_groups = PBField.new("groups", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _groups
		service.func_ref = funcref(self, "add_groups")
		data[_groups.tag] = service
		
	var data = {}
	
	var _groups
	func get_groups():
		return _groups.value
	func clear_groups():
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_groups():
		var element = TGroupsCreate.GroupCreate.new()
		_groups.value.append(element)
		return element
	
	class GroupCreate:
		func _init():
			var service
			
			_name = PBField.new("name", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _name
			data[_name.tag] = service
			
			_description = PBField.new("description", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _description
			data[_description.tag] = service
			
			_avatar_url = PBField.new("avatar_url", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _avatar_url
			data[_avatar_url.tag] = service
			
			_lang = PBField.new("lang", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _lang
			data[_lang.tag] = service
			
			_metadata = PBField.new("metadata", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _metadata
			data[_metadata.tag] = service
			
			_private = PBField.new("private", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
			service = PBServiceField.new()
			service.field = _private
			data[_private.tag] = service
			
		var data = {}
		
		var _name
		func get_name():
			return _name.value
		func clear_name():
			_name.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_name(value):
			_name.value = value
		
		var _description
		func get_description():
			return _description.value
		func clear_description():
			_description.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_description(value):
			_description.value = value
		
		var _avatar_url
		func get_avatar_url():
			return _avatar_url.value
		func clear_avatar_url():
			_avatar_url.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_avatar_url(value):
			_avatar_url.value = value
		
		var _lang
		func get_lang():
			return _lang.value
		func clear_lang():
			_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_lang(value):
			_lang.value = value
		
		var _metadata
		func get_metadata():
			return _metadata.value
		func clear_metadata():
			_metadata.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_metadata(value):
			_metadata.value = value
		
		var _private
		func get_private():
			return _private.value
		func clear_private():
			_private.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
		func set_private(value):
			_private.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupsUpdate:
	func _init():
		var service
		
		_groups = PBField.new("groups", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _groups
		service.func_ref = funcref(self, "add_groups")
		data[_groups.tag] = service
		
	var data = {}
	
	var _groups
	func get_groups():
		return _groups.value
	func clear_groups():
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_groups():
		var element = TGroupsUpdate.GroupUpdate.new()
		_groups.value.append(element)
		return element
	
	class GroupUpdate:
		func _init():
			var service
			
			_group_id = PBField.new("group_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _group_id
			data[_group_id.tag] = service
			
			_private = PBField.new("private", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
			service = PBServiceField.new()
			service.field = _private
			data[_private.tag] = service
			
			_name = PBField.new("name", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _name
			data[_name.tag] = service
			
			_description = PBField.new("description", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _description
			data[_description.tag] = service
			
			_avatar_url = PBField.new("avatar_url", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _avatar_url
			data[_avatar_url.tag] = service
			
			_lang = PBField.new("lang", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _lang
			data[_lang.tag] = service
			
			_metadata = PBField.new("metadata", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _metadata
			data[_metadata.tag] = service
			
		var data = {}
		
		var _group_id
		func get_group_id():
			return _group_id.value
		func clear_group_id():
			_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_group_id(value):
			_group_id.value = value
		
		var _private
		func get_private():
			return _private.value
		func clear_private():
			_private.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
		func set_private(value):
			_private.value = value
		
		var _name
		func get_name():
			return _name.value
		func clear_name():
			_name.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_name(value):
			_name.value = value
		
		var _description
		func get_description():
			return _description.value
		func clear_description():
			_description.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_description(value):
			_description.value = value
		
		var _avatar_url
		func get_avatar_url():
			return _avatar_url.value
		func clear_avatar_url():
			_avatar_url.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_avatar_url(value):
			_avatar_url.value = value
		
		var _lang
		func get_lang():
			return _lang.value
		func clear_lang():
			_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_lang(value):
			_lang.value = value
		
		var _metadata
		func get_metadata():
			return _metadata.value
		func clear_metadata():
			_metadata.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_metadata(value):
			_metadata.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupsRemove:
	func _init():
		var service
		
		_group_ids = PBField.new("group_ids", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _group_ids
		data[_group_ids.tag] = service
		
	var data = {}
	
	var _group_ids
	func get_group_ids():
		return _group_ids.value
	func clear_group_ids():
		_group_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func add_group_ids(value):
		_group_ids.value.append(value)
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupsSelfList:
	func _init():
		var service
		
	var data = {}
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupsFetch:
	func _init():
		var service
		
		_groups = PBField.new("groups", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _groups
		service.func_ref = funcref(self, "add_groups")
		data[_groups.tag] = service
		
	var data = {}
	
	var _groups
	func get_groups():
		return _groups.value
	func clear_groups():
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_groups():
		var element = TGroupsFetch.GroupFetch.new()
		_groups.value.append(element)
		return element
	
	class GroupFetch:
		func _init():
			var service
			
			_group_id = PBField.new("group_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _group_id
			data[_group_id.tag] = service
			
			_name = PBField.new("name", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _name
			data[_name.tag] = service
			
		var data = {}
		
		var _group_id
		func has_group_id():
			if data[1].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_group_id():
			return _group_id.value
		func clear_group_id():
			_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_group_id(value):
			_name.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_group_id.value = value
		
		var _name
		func has_name():
			if data[2].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_name():
			return _name.value
		func clear_name():
			_name.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_name(value):
			_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_name.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupsList:
	func _init():
		var service
		
		_page_limit = PBField.new("page_limit", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _page_limit
		data[_page_limit.tag] = service
		
		_order_by_asc = PBField.new("order_by_asc", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _order_by_asc
		data[_order_by_asc.tag] = service
		
		_lang = PBField.new("lang", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _lang
		data[_lang.tag] = service
		
		_created_at = PBField.new("created_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _created_at
		data[_created_at.tag] = service
		
		_count = PBField.new("count", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _count
		data[_count.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
	var data = {}
	
	var _page_limit
	func get_page_limit():
		return _page_limit.value
	func clear_page_limit():
		_page_limit.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_page_limit(value):
		_page_limit.value = value
	
	var _order_by_asc
	func get_order_by_asc():
		return _order_by_asc.value
	func clear_order_by_asc():
		_order_by_asc.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_order_by_asc(value):
		_order_by_asc.value = value
	
	var _lang
	func has_lang():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_lang():
		return _lang.value
	func clear_lang():
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_lang(value):
		_created_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		_count.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		_lang.value = value
	
	var _created_at
	func has_created_at():
		if data[4].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_created_at():
		return _created_at.value
	func clear_created_at():
		_created_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_created_at(value):
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_count.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		_created_at.value = value
	
	var _count
	func has_count():
		if data[5].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_count():
		return _count.value
	func clear_count():
		_count.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_count(value):
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_created_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		_count.value = value
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroups:
	func _init():
		var service
		
		_groups = PBField.new("groups", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _groups
		service.func_ref = funcref(self, "add_groups")
		data[_groups.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
	var data = {}
	
	var _groups
	func get_groups():
		return _groups.value
	func clear_groups():
		_groups.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_groups():
		var element = Group.new()
		_groups.value.append(element)
		return element
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupsSelf:
	func _init():
		var service
		
		_groups_self = PBField.new("groups_self", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _groups_self
		service.func_ref = funcref(self, "add_groups_self")
		data[_groups_self.tag] = service
		
	var data = {}
	
	var _groups_self
	func get_groups_self():
		return _groups_self.value
	func clear_groups_self():
		_groups_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_groups_self():
		var element = TGroupsSelf.GroupSelf.new()
		_groups_self.value.append(element)
		return element
	
	class GroupSelf:
		func _init():
			var service
			
			_group = PBField.new("group", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
			service = PBServiceField.new()
			service.field = _group
			service.func_ref = funcref(self, "new_group")
			data[_group.tag] = service
			
			_state = PBField.new("state", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _state
			data[_state.tag] = service
			
		var data = {}
		
		var _group
		func get_group():
			return _group.value
		func clear_group():
			_group.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		func new_group():
			_group.value = Group.new()
			return _group.value
		
		var _state
		func get_state():
			return _state.value
		func clear_state():
			_state.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_state(value):
			_state.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class GroupUser:
	func _init():
		var service
		
		_user = PBField.new("user", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _user
		service.func_ref = funcref(self, "new_user")
		data[_user.tag] = service
		
		_state = PBField.new("state", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _state
		data[_state.tag] = service
		
	var data = {}
	
	var _user
	func get_user():
		return _user.value
	func clear_user():
		_user.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_user():
		_user.value = User.new()
		return _user.value
	
	var _state
	func get_state():
		return _state.value
	func clear_state():
		_state.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_state(value):
		_state.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupUsersList:
	func _init():
		var service
		
		_group_id = PBField.new("group_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _group_id
		data[_group_id.tag] = service
		
	var data = {}
	
	var _group_id
	func get_group_id():
		return _group_id.value
	func clear_group_id():
		_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_group_id(value):
		_group_id.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupUsers:
	func _init():
		var service
		
		_users = PBField.new("users", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _users
		service.func_ref = funcref(self, "add_users")
		data[_users.tag] = service
		
	var data = {}
	
	var _users
	func get_users():
		return _users.value
	func clear_users():
		_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_users():
		var element = GroupUser.new()
		_users.value.append(element)
		return element
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupsJoin:
	func _init():
		var service
		
		_group_ids = PBField.new("group_ids", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _group_ids
		data[_group_ids.tag] = service
		
	var data = {}
	
	var _group_ids
	func get_group_ids():
		return _group_ids.value
	func clear_group_ids():
		_group_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func add_group_ids(value):
		_group_ids.value.append(value)
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupsLeave:
	func _init():
		var service
		
		_group_ids = PBField.new("group_ids", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _group_ids
		data[_group_ids.tag] = service
		
	var data = {}
	
	var _group_ids
	func get_group_ids():
		return _group_ids.value
	func clear_group_ids():
		_group_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func add_group_ids(value):
		_group_ids.value.append(value)
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupUsersAdd:
	func _init():
		var service
		
		_group_users = PBField.new("group_users", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _group_users
		service.func_ref = funcref(self, "add_group_users")
		data[_group_users.tag] = service
		
	var data = {}
	
	var _group_users
	func get_group_users():
		return _group_users.value
	func clear_group_users():
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_group_users():
		var element = TGroupUsersAdd.GroupUserAdd.new()
		_group_users.value.append(element)
		return element
	
	class GroupUserAdd:
		func _init():
			var service
			
			_group_id = PBField.new("group_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _group_id
			data[_group_id.tag] = service
			
			_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _user_id
			data[_user_id.tag] = service
			
		var data = {}
		
		var _group_id
		func get_group_id():
			return _group_id.value
		func clear_group_id():
			_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_group_id(value):
			_group_id.value = value
		
		var _user_id
		func get_user_id():
			return _user_id.value
		func clear_user_id():
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_user_id(value):
			_user_id.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupUsersKick:
	func _init():
		var service
		
		_group_users = PBField.new("group_users", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _group_users
		service.func_ref = funcref(self, "add_group_users")
		data[_group_users.tag] = service
		
	var data = {}
	
	var _group_users
	func get_group_users():
		return _group_users.value
	func clear_group_users():
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_group_users():
		var element = TGroupUsersKick.GroupUserKick.new()
		_group_users.value.append(element)
		return element
	
	class GroupUserKick:
		func _init():
			var service
			
			_group_id = PBField.new("group_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _group_id
			data[_group_id.tag] = service
			
			_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _user_id
			data[_user_id.tag] = service
			
		var data = {}
		
		var _group_id
		func get_group_id():
			return _group_id.value
		func clear_group_id():
			_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_group_id(value):
			_group_id.value = value
		
		var _user_id
		func get_user_id():
			return _user_id.value
		func clear_user_id():
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_user_id(value):
			_user_id.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TGroupUsersPromote:
	func _init():
		var service
		
		_group_users = PBField.new("group_users", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _group_users
		service.func_ref = funcref(self, "add_group_users")
		data[_group_users.tag] = service
		
	var data = {}
	
	var _group_users
	func get_group_users():
		return _group_users.value
	func clear_group_users():
		_group_users.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_group_users():
		var element = TGroupUsersPromote.GroupUserPromote.new()
		_group_users.value.append(element)
		return element
	
	class GroupUserPromote:
		func _init():
			var service
			
			_group_id = PBField.new("group_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _group_id
			data[_group_id.tag] = service
			
			_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _user_id
			data[_user_id.tag] = service
			
		var data = {}
		
		var _group_id
		func get_group_id():
			return _group_id.value
		func clear_group_id():
			_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_group_id(value):
			_group_id.value = value
		
		var _user_id
		func get_user_id():
			return _user_id.value
		func clear_user_id():
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_user_id(value):
			_user_id.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TopicId:
	func _init():
		var service
		
		_dm = PBField.new("dm", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _dm
		data[_dm.tag] = service
		
		_room = PBField.new("room", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _room
		data[_room.tag] = service
		
		_group_id = PBField.new("group_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _group_id
		data[_group_id.tag] = service
		
	var data = {}
	
	var _dm
	func has_dm():
		if data[1].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_dm():
		return _dm.value
	func clear_dm():
		_dm.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_dm(value):
		_room.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_dm.value = value
	
	var _room
	func has_room():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_room():
		return _room.value
	func clear_room():
		_room.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_room(value):
		_dm.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_room.value = value
	
	var _group_id
	func has_group_id():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_group_id():
		return _group_id.value
	func clear_group_id():
		_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_group_id(value):
		_dm.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_room.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_group_id.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class UserPresence:
	func _init():
		var service
		
		_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _user_id
		data[_user_id.tag] = service
		
		_session_id = PBField.new("session_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _session_id
		data[_session_id.tag] = service
		
		_handle = PBField.new("handle", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _handle
		data[_handle.tag] = service
		
	var data = {}
	
	var _user_id
	func get_user_id():
		return _user_id.value
	func clear_user_id():
		_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_user_id(value):
		_user_id.value = value
	
	var _session_id
	func get_session_id():
		return _session_id.value
	func clear_session_id():
		_session_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_session_id(value):
		_session_id.value = value
	
	var _handle
	func get_handle():
		return _handle.value
	func clear_handle():
		_handle.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_handle(value):
		_handle.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TTopicsJoin:
	func _init():
		var service
		
		_joins = PBField.new("joins", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _joins
		service.func_ref = funcref(self, "add_joins")
		data[_joins.tag] = service
		
	var data = {}
	
	var _joins
	func get_joins():
		return _joins.value
	func clear_joins():
		_joins.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_joins():
		var element = TTopicsJoin.TopicJoin.new()
		_joins.value.append(element)
		return element
	
	class TopicJoin:
		func _init():
			var service
			
			_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _user_id
			data[_user_id.tag] = service
			
			_room = PBField.new("room", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _room
			data[_room.tag] = service
			
			_group_id = PBField.new("group_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _group_id
			data[_group_id.tag] = service
			
		var data = {}
		
		var _user_id
		func has_user_id():
			if data[1].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_user_id():
			return _user_id.value
		func clear_user_id():
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_user_id(value):
			_room.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_user_id.value = value
		
		var _room
		func has_room():
			if data[2].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_room():
			return _room.value
		func clear_room():
			_room.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_room(value):
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_room.value = value
		
		var _group_id
		func has_group_id():
			if data[3].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_group_id():
			return _group_id.value
		func clear_group_id():
			_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_group_id(value):
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_room.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_group_id.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TTopics:
	func _init():
		var service
		
		_topics = PBField.new("topics", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _topics
		service.func_ref = funcref(self, "add_topics")
		data[_topics.tag] = service
		
	var data = {}
	
	var _topics
	func get_topics():
		return _topics.value
	func clear_topics():
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_topics():
		var element = TTopics.Topic.new()
		_topics.value.append(element)
		return element
	
	class Topic:
		func _init():
			var service
			
			_topic = PBField.new("topic", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
			service = PBServiceField.new()
			service.field = _topic
			service.func_ref = funcref(self, "new_topic")
			data[_topic.tag] = service
			
			_presences = PBField.new("presences", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 2, true, [])
			service = PBServiceField.new()
			service.field = _presences
			service.func_ref = funcref(self, "add_presences")
			data[_presences.tag] = service
			
			_self = PBField.new("self", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
			service = PBServiceField.new()
			service.field = _self
			service.func_ref = funcref(self, "new_self")
			data[_self.tag] = service
			
		var data = {}
		
		var _topic
		func get_topic():
			return _topic.value
		func clear_topic():
			_topic.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		func new_topic():
			_topic.value = TopicId.new()
			return _topic.value
		
		var _presences
		func get_presences():
			return _presences.value
		func clear_presences():
			_presences.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		func add_presences():
			var element = UserPresence.new()
			_presences.value.append(element)
			return element
		
		var _self
		func get_self():
			return _self.value
		func clear_self():
			_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		func new_self():
			_self.value = UserPresence.new()
			return _self.value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TTopicsLeave:
	func _init():
		var service
		
		_topics = PBField.new("topics", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _topics
		service.func_ref = funcref(self, "add_topics")
		data[_topics.tag] = service
		
	var data = {}
	
	var _topics
	func get_topics():
		return _topics.value
	func clear_topics():
		_topics.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_topics():
		var element = TopicId.new()
		_topics.value.append(element)
		return element
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TTopicMessageSend:
	func _init():
		var service
		
		_topic = PBField.new("topic", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topic
		service.func_ref = funcref(self, "new_topic")
		data[_topic.tag] = service
		
		_data = PBField.new("data", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _data
		data[_data.tag] = service
		
	var data = {}
	
	var _topic
	func get_topic():
		return _topic.value
	func clear_topic():
		_topic.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topic():
		_topic.value = TopicId.new()
		return _topic.value
	
	var _data
	func get_data():
		return _data.value
	func clear_data():
		_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_data(value):
		_data.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TTopicMessageAck:
	func _init():
		var service
		
		_message_id = PBField.new("message_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _message_id
		data[_message_id.tag] = service
		
		_created_at = PBField.new("created_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _created_at
		data[_created_at.tag] = service
		
		_expires_at = PBField.new("expires_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _expires_at
		data[_expires_at.tag] = service
		
		_handle = PBField.new("handle", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _handle
		data[_handle.tag] = service
		
	var data = {}
	
	var _message_id
	func get_message_id():
		return _message_id.value
	func clear_message_id():
		_message_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_message_id(value):
		_message_id.value = value
	
	var _created_at
	func get_created_at():
		return _created_at.value
	func clear_created_at():
		_created_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_created_at(value):
		_created_at.value = value
	
	var _expires_at
	func get_expires_at():
		return _expires_at.value
	func clear_expires_at():
		_expires_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_expires_at(value):
		_expires_at.value = value
	
	var _handle
	func get_handle():
		return _handle.value
	func clear_handle():
		_handle.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_handle(value):
		_handle.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TopicMessage:
	func _init():
		var service
		
		_topic = PBField.new("topic", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topic
		service.func_ref = funcref(self, "new_topic")
		data[_topic.tag] = service
		
		_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _user_id
		data[_user_id.tag] = service
		
		_message_id = PBField.new("message_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _message_id
		data[_message_id.tag] = service
		
		_created_at = PBField.new("created_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _created_at
		data[_created_at.tag] = service
		
		_expires_at = PBField.new("expires_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _expires_at
		data[_expires_at.tag] = service
		
		_handle = PBField.new("handle", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _handle
		data[_handle.tag] = service
		
		_type = PBField.new("type", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _type
		data[_type.tag] = service
		
		_data = PBField.new("data", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _data
		data[_data.tag] = service
		
	var data = {}
	
	var _topic
	func get_topic():
		return _topic.value
	func clear_topic():
		_topic.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topic():
		_topic.value = TopicId.new()
		return _topic.value
	
	var _user_id
	func get_user_id():
		return _user_id.value
	func clear_user_id():
		_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_user_id(value):
		_user_id.value = value
	
	var _message_id
	func get_message_id():
		return _message_id.value
	func clear_message_id():
		_message_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_message_id(value):
		_message_id.value = value
	
	var _created_at
	func get_created_at():
		return _created_at.value
	func clear_created_at():
		_created_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_created_at(value):
		_created_at.value = value
	
	var _expires_at
	func get_expires_at():
		return _expires_at.value
	func clear_expires_at():
		_expires_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_expires_at(value):
		_expires_at.value = value
	
	var _handle
	func get_handle():
		return _handle.value
	func clear_handle():
		_handle.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_handle(value):
		_handle.value = value
	
	var _type
	func get_type():
		return _type.value
	func clear_type():
		_type.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_type(value):
		_type.value = value
	
	var _data
	func get_data():
		return _data.value
	func clear_data():
		_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_data(value):
		_data.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TTopicMessagesList:
	func _init():
		var service
		
		_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _user_id
		data[_user_id.tag] = service
		
		_room = PBField.new("room", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _room
		data[_room.tag] = service
		
		_group_id = PBField.new("group_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _group_id
		data[_group_id.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
		_forward = PBField.new("forward", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _forward
		data[_forward.tag] = service
		
		_limit = PBField.new("limit", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _limit
		data[_limit.tag] = service
		
	var data = {}
	
	var _user_id
	func has_user_id():
		if data[1].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_user_id():
		return _user_id.value
	func clear_user_id():
		_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_user_id(value):
		_room.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_user_id.value = value
	
	var _room
	func has_room():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_room():
		return _room.value
	func clear_room():
		_room.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_room(value):
		_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_room.value = value
	
	var _group_id
	func has_group_id():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_group_id():
		return _group_id.value
	func clear_group_id():
		_group_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_group_id(value):
		_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_room.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_group_id.value = value
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	var _forward
	func get_forward():
		return _forward.value
	func clear_forward():
		_forward.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_forward(value):
		_forward.value = value
	
	var _limit
	func get_limit():
		return _limit.value
	func clear_limit():
		_limit.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_limit(value):
		_limit.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TTopicMessages:
	func _init():
		var service
		
		_messages = PBField.new("messages", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _messages
		service.func_ref = funcref(self, "add_messages")
		data[_messages.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
	var data = {}
	
	var _messages
	func get_messages():
		return _messages.value
	func clear_messages():
		_messages.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_messages():
		var element = TopicMessage.new()
		_messages.value.append(element)
		return element
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TopicPresence:
	func _init():
		var service
		
		_topic = PBField.new("topic", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _topic
		service.func_ref = funcref(self, "new_topic")
		data[_topic.tag] = service
		
		_joins = PBField.new("joins", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 2, true, [])
		service = PBServiceField.new()
		service.field = _joins
		service.func_ref = funcref(self, "add_joins")
		data[_joins.tag] = service
		
		_leaves = PBField.new("leaves", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 3, true, [])
		service = PBServiceField.new()
		service.field = _leaves
		service.func_ref = funcref(self, "add_leaves")
		data[_leaves.tag] = service
		
	var data = {}
	
	var _topic
	func get_topic():
		return _topic.value
	func clear_topic():
		_topic.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_topic():
		_topic.value = TopicId.new()
		return _topic.value
	
	var _joins
	func get_joins():
		return _joins.value
	func clear_joins():
		_joins.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_joins():
		var element = UserPresence.new()
		_joins.value.append(element)
		return element
	
	var _leaves
	func get_leaves():
		return _leaves.value
	func clear_leaves():
		_leaves.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_leaves():
		var element = UserPresence.new()
		_leaves.value.append(element)
		return element
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class PropertyPair:
	func _init():
		var service
		
		_key = PBField.new("key", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _key
		data[_key.tag] = service
		
		_stringSet = PBField.new("stringSet", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _stringSet
		service.func_ref = funcref(self, "new_stringSet")
		data[_stringSet.tag] = service
		
		_boolValue = PBField.new("boolValue", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _boolValue
		data[_boolValue.tag] = service
		
		_intValue = PBField.new("intValue", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _intValue
		data[_intValue.tag] = service
		
	var data = {}
	
	var _key
	func get_key():
		return _key.value
	func clear_key():
		_key.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_key(value):
		_key.value = value
	
	var _stringSet
	func has_stringSet():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_stringSet():
		return _stringSet.value
	func clear_stringSet():
		_stringSet.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_stringSet():
		_boolValue.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
		_intValue.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		_stringSet.value = PropertyPair.StringSet.new()
		return _stringSet.value
	
	var _boolValue
	func has_boolValue():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_boolValue():
		return _boolValue.value
	func clear_boolValue():
		_boolValue.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_boolValue(value):
		_stringSet.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_intValue.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		_boolValue.value = value
	
	var _intValue
	func has_intValue():
		if data[4].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_intValue():
		return _intValue.value
	func clear_intValue():
		_intValue.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_intValue(value):
		_stringSet.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_boolValue.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
		_intValue.value = value
	
	class StringSet:
		func _init():
			var service
			
			_values = PBField.new("values", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
			service = PBServiceField.new()
			service.field = _values
			data[_values.tag] = service
			
		var data = {}
		
		var _values
		func get_values():
			return _values.value
		func clear_values():
			_values.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func add_values(value):
			_values.value.append(value)
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class MatchmakeFilter:
	func _init():
		var service
		
		_name = PBField.new("name", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _name
		data[_name.tag] = service
		
		_term = PBField.new("term", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _term
		service.func_ref = funcref(self, "new_term")
		data[_term.tag] = service
		
		_range = PBField.new("range", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _range
		service.func_ref = funcref(self, "new_range")
		data[_range.tag] = service
		
		_check = PBField.new("check", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _check
		data[_check.tag] = service
		
	var data = {}
	
	var _name
	func get_name():
		return _name.value
	func clear_name():
		_name.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_name(value):
		_name.value = value
	
	var _term
	func has_term():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_term():
		return _term.value
	func clear_term():
		_term.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_term():
		_range.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_check.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
		_term.value = MatchmakeFilter.TermFilter.new()
		return _term.value
	
	var _range
	func has_range():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_range():
		return _range.value
	func clear_range():
		_range.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_range():
		_term.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_check.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
		_range.value = MatchmakeFilter.RangeFilter.new()
		return _range.value
	
	var _check
	func has_check():
		if data[4].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_check():
		return _check.value
	func clear_check():
		_check.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_check(value):
		_term.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_range.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_check.value = value
	
	class TermFilter:
		func _init():
			var service
			
			_terms = PBField.new("terms", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
			service = PBServiceField.new()
			service.field = _terms
			data[_terms.tag] = service
			
			_matchAllTerms = PBField.new("matchAllTerms", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
			service = PBServiceField.new()
			service.field = _matchAllTerms
			data[_matchAllTerms.tag] = service
			
		var data = {}
		
		var _terms
		func get_terms():
			return _terms.value
		func clear_terms():
			_terms.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func add_terms(value):
			_terms.value.append(value)
		
		var _matchAllTerms
		func get_matchAllTerms():
			return _matchAllTerms.value
		func clear_matchAllTerms():
			_matchAllTerms.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
		func set_matchAllTerms(value):
			_matchAllTerms.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	class RangeFilter:
		func _init():
			var service
			
			_lower_bound = PBField.new("lower_bound", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _lower_bound
			data[_lower_bound.tag] = service
			
			_upper_bound = PBField.new("upper_bound", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _upper_bound
			data[_upper_bound.tag] = service
			
		var data = {}
		
		var _lower_bound
		func get_lower_bound():
			return _lower_bound.value
		func clear_lower_bound():
			_lower_bound.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_lower_bound(value):
			_lower_bound.value = value
		
		var _upper_bound
		func get_upper_bound():
			return _upper_bound.value
		func clear_upper_bound():
			_upper_bound.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_upper_bound(value):
			_upper_bound.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TMatchmakeAdd:
	func _init():
		var service
		
		_required_count = PBField.new("required_count", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _required_count
		data[_required_count.tag] = service
		
		_filters = PBField.new("filters", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 2, true, [])
		service = PBServiceField.new()
		service.field = _filters
		service.func_ref = funcref(self, "add_filters")
		data[_filters.tag] = service
		
		_properties = PBField.new("properties", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 3, true, [])
		service = PBServiceField.new()
		service.field = _properties
		service.func_ref = funcref(self, "add_properties")
		data[_properties.tag] = service
		
	var data = {}
	
	var _required_count
	func get_required_count():
		return _required_count.value
	func clear_required_count():
		_required_count.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_required_count(value):
		_required_count.value = value
	
	var _filters
	func get_filters():
		return _filters.value
	func clear_filters():
		_filters.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_filters():
		var element = MatchmakeFilter.new()
		_filters.value.append(element)
		return element
	
	var _properties
	func get_properties():
		return _properties.value
	func clear_properties():
		_properties.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_properties():
		var element = PropertyPair.new()
		_properties.value.append(element)
		return element
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TMatchmakeTicket:
	func _init():
		var service
		
		_ticket = PBField.new("ticket", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _ticket
		data[_ticket.tag] = service
		
	var data = {}
	
	var _ticket
	func get_ticket():
		return _ticket.value
	func clear_ticket():
		_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_ticket(value):
		_ticket.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TMatchmakeRemove:
	func _init():
		var service
		
		_ticket = PBField.new("ticket", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _ticket
		data[_ticket.tag] = service
		
	var data = {}
	
	var _ticket
	func get_ticket():
		return _ticket.value
	func clear_ticket():
		_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_ticket(value):
		_ticket.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class MatchmakeMatched:
	func _init():
		var service
		
		_ticket = PBField.new("ticket", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _ticket
		data[_ticket.tag] = service
		
		_token = PBField.new("token", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _token
		data[_token.tag] = service
		
		_presences = PBField.new("presences", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 3, true, [])
		service = PBServiceField.new()
		service.field = _presences
		service.func_ref = funcref(self, "add_presences")
		data[_presences.tag] = service
		
		_self = PBField.new("self", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _self
		service.func_ref = funcref(self, "new_self")
		data[_self.tag] = service
		
		_properties = PBField.new("properties", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 5, true, [])
		service = PBServiceField.new()
		service.field = _properties
		service.func_ref = funcref(self, "add_properties")
		data[_properties.tag] = service
		
	var data = {}
	
	var _ticket
	func get_ticket():
		return _ticket.value
	func clear_ticket():
		_ticket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_ticket(value):
		_ticket.value = value
	
	var _token
	func get_token():
		return _token.value
	func clear_token():
		_token.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_token(value):
		_token.value = value
	
	var _presences
	func get_presences():
		return _presences.value
	func clear_presences():
		_presences.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_presences():
		var element = UserPresence.new()
		_presences.value.append(element)
		return element
	
	var _self
	func get_self():
		return _self.value
	func clear_self():
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_self():
		_self.value = UserPresence.new()
		return _self.value
	
	var _properties
	func get_properties():
		return _properties.value
	func clear_properties():
		_properties.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_properties():
		var element = MatchmakeMatched.UserProperty.new()
		_properties.value.append(element)
		return element
	
	class UserProperty:
		func _init():
			var service
			
			_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _user_id
			data[_user_id.tag] = service
			
			_properties = PBField.new("properties", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 2, true, [])
			service = PBServiceField.new()
			service.field = _properties
			service.func_ref = funcref(self, "add_properties")
			data[_properties.tag] = service
			
			_filters = PBField.new("filters", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 3, true, [])
			service = PBServiceField.new()
			service.field = _filters
			service.func_ref = funcref(self, "add_filters")
			data[_filters.tag] = service
			
		var data = {}
		
		var _user_id
		func get_user_id():
			return _user_id.value
		func clear_user_id():
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_user_id(value):
			_user_id.value = value
		
		var _properties
		func get_properties():
			return _properties.value
		func clear_properties():
			_properties.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		func add_properties():
			var element = PropertyPair.new()
			_properties.value.append(element)
			return element
		
		var _filters
		func get_filters():
			return _filters.value
		func clear_filters():
			_filters.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		func add_filters():
			var element = MatchmakeFilter.new()
			_filters.value.append(element)
			return element
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class Match:
	func _init():
		var service
		
		_match_id = PBField.new("match_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _match_id
		data[_match_id.tag] = service
		
		_presences = PBField.new("presences", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 2, true, [])
		service = PBServiceField.new()
		service.field = _presences
		service.func_ref = funcref(self, "add_presences")
		data[_presences.tag] = service
		
		_self = PBField.new("self", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _self
		service.func_ref = funcref(self, "new_self")
		data[_self.tag] = service
		
	var data = {}
	
	var _match_id
	func get_match_id():
		return _match_id.value
	func clear_match_id():
		_match_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_match_id(value):
		_match_id.value = value
	
	var _presences
	func get_presences():
		return _presences.value
	func clear_presences():
		_presences.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_presences():
		var element = UserPresence.new()
		_presences.value.append(element)
		return element
	
	var _self
	func get_self():
		return _self.value
	func clear_self():
		_self.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_self():
		_self.value = UserPresence.new()
		return _self.value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class MatchPresence:
	func _init():
		var service
		
		_match_id = PBField.new("match_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _match_id
		data[_match_id.tag] = service
		
		_joins = PBField.new("joins", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 2, true, [])
		service = PBServiceField.new()
		service.field = _joins
		service.func_ref = funcref(self, "add_joins")
		data[_joins.tag] = service
		
		_leaves = PBField.new("leaves", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 3, true, [])
		service = PBServiceField.new()
		service.field = _leaves
		service.func_ref = funcref(self, "add_leaves")
		data[_leaves.tag] = service
		
	var data = {}
	
	var _match_id
	func get_match_id():
		return _match_id.value
	func clear_match_id():
		_match_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_match_id(value):
		_match_id.value = value
	
	var _joins
	func get_joins():
		return _joins.value
	func clear_joins():
		_joins.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_joins():
		var element = UserPresence.new()
		_joins.value.append(element)
		return element
	
	var _leaves
	func get_leaves():
		return _leaves.value
	func clear_leaves():
		_leaves.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_leaves():
		var element = UserPresence.new()
		_leaves.value.append(element)
		return element
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TMatchCreate:
	func _init():
		var service
		
	var data = {}
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TMatch:
	func _init():
		var service
		
		_match = PBField.new("match", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _match
		service.func_ref = funcref(self, "new_match")
		data[_match.tag] = service
		
	var data = {}
	
	var _match
	func get_match():
		return _match.value
	func clear_match():
		_match.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_match():
		_match.value = Match.new()
		return _match.value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TMatchesJoin:
	func _init():
		var service
		
		_matches = PBField.new("matches", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _matches
		service.func_ref = funcref(self, "add_matches")
		data[_matches.tag] = service
		
	var data = {}
	
	var _matches
	func get_matches():
		return _matches.value
	func clear_matches():
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_matches():
		var element = TMatchesJoin.MatchJoin.new()
		_matches.value.append(element)
		return element
	
	class MatchJoin:
		func _init():
			var service
			
			_match_id = PBField.new("match_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _match_id
			data[_match_id.tag] = service
			
			_token = PBField.new("token", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _token
			data[_token.tag] = service
			
		var data = {}
		
		var _match_id
		func has_match_id():
			if data[1].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_match_id():
			return _match_id.value
		func clear_match_id():
			_match_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_match_id(value):
			_token.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_match_id.value = value
		
		var _token
		func has_token():
			if data[2].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_token():
			return _token.value
		func clear_token():
			_token.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_token(value):
			_match_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			_token.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TMatches:
	func _init():
		var service
		
		_matches = PBField.new("matches", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _matches
		service.func_ref = funcref(self, "add_matches")
		data[_matches.tag] = service
		
	var data = {}
	
	var _matches
	func get_matches():
		return _matches.value
	func clear_matches():
		_matches.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_matches():
		var element = Match.new()
		_matches.value.append(element)
		return element
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class MatchDataSend:
	func _init():
		var service
		
		_match_id = PBField.new("match_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _match_id
		data[_match_id.tag] = service
		
		_op_code = PBField.new("op_code", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _op_code
		data[_op_code.tag] = service
		
		_data = PBField.new("data", PB_DATA_TYPE.BYTES, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BYTES])
		service = PBServiceField.new()
		service.field = _data
		data[_data.tag] = service
		
		_presences = PBField.new("presences", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 4, true, [])
		service = PBServiceField.new()
		service.field = _presences
		service.func_ref = funcref(self, "add_presences")
		data[_presences.tag] = service
		
	var data = {}
	
	var _match_id
	func get_match_id():
		return _match_id.value
	func clear_match_id():
		_match_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_match_id(value):
		_match_id.value = value
	
	var _op_code
	func get_op_code():
		return _op_code.value
	func clear_op_code():
		_op_code.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_op_code(value):
		_op_code.value = value
	
	var _data
	func get_data():
		return _data.value
	func clear_data():
		_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BYTES]
	func set_data(value):
		_data.value = value
	
	var _presences
	func get_presences():
		return _presences.value
	func clear_presences():
		_presences.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_presences():
		var element = UserPresence.new()
		_presences.value.append(element)
		return element
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class MatchData:
	func _init():
		var service
		
		_match_id = PBField.new("match_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _match_id
		data[_match_id.tag] = service
		
		_presence = PBField.new("presence", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _presence
		service.func_ref = funcref(self, "new_presence")
		data[_presence.tag] = service
		
		_op_code = PBField.new("op_code", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _op_code
		data[_op_code.tag] = service
		
		_data = PBField.new("data", PB_DATA_TYPE.BYTES, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BYTES])
		service = PBServiceField.new()
		service.field = _data
		data[_data.tag] = service
		
	var data = {}
	
	var _match_id
	func get_match_id():
		return _match_id.value
	func clear_match_id():
		_match_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_match_id(value):
		_match_id.value = value
	
	var _presence
	func get_presence():
		return _presence.value
	func clear_presence():
		_presence.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_presence():
		_presence.value = UserPresence.new()
		return _presence.value
	
	var _op_code
	func get_op_code():
		return _op_code.value
	func clear_op_code():
		_op_code.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_op_code(value):
		_op_code.value = value
	
	var _data
	func get_data():
		return _data.value
	func clear_data():
		_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BYTES]
	func set_data(value):
		_data.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TMatchesLeave:
	func _init():
		var service
		
		_match_ids = PBField.new("match_ids", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _match_ids
		data[_match_ids.tag] = service
		
	var data = {}
	
	var _match_ids
	func get_match_ids():
		return _match_ids.value
	func clear_match_ids():
		_match_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func add_match_ids(value):
		_match_ids.value.append(value)
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
enum StoragePermissionRead {
	NO_READ = 0,
	OWNER_READ = 1,
	PUBLIC_READ = 2
}

enum StoragePermissionWrite {
	NO_WRITE = 0,
	OWNER_WRITE = 1
}

class TStorageList:
	func _init():
		var service
		
		_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _user_id
		data[_user_id.tag] = service
		
		_bucket = PBField.new("bucket", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _bucket
		data[_bucket.tag] = service
		
		_collection = PBField.new("collection", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _collection
		data[_collection.tag] = service
		
		_limit = PBField.new("limit", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _limit
		data[_limit.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
	var data = {}
	
	var _user_id
	func get_user_id():
		return _user_id.value
	func clear_user_id():
		_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_user_id(value):
		_user_id.value = value
	
	var _bucket
	func get_bucket():
		return _bucket.value
	func clear_bucket():
		_bucket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_bucket(value):
		_bucket.value = value
	
	var _collection
	func get_collection():
		return _collection.value
	func clear_collection():
		_collection.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_collection(value):
		_collection.value = value
	
	var _limit
	func get_limit():
		return _limit.value
	func clear_limit():
		_limit.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_limit(value):
		_limit.value = value
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TStorageFetch:
	func _init():
		var service
		
		_keys = PBField.new("keys", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _keys
		service.func_ref = funcref(self, "add_keys")
		data[_keys.tag] = service
		
	var data = {}
	
	var _keys
	func get_keys():
		return _keys.value
	func clear_keys():
		_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_keys():
		var element = TStorageFetch.StorageKey.new()
		_keys.value.append(element)
		return element
	
	class StorageKey:
		func _init():
			var service
			
			_bucket = PBField.new("bucket", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _bucket
			data[_bucket.tag] = service
			
			_collection = PBField.new("collection", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _collection
			data[_collection.tag] = service
			
			_record = PBField.new("record", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _record
			data[_record.tag] = service
			
			_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _user_id
			data[_user_id.tag] = service
			
		var data = {}
		
		var _bucket
		func get_bucket():
			return _bucket.value
		func clear_bucket():
			_bucket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_bucket(value):
			_bucket.value = value
		
		var _collection
		func get_collection():
			return _collection.value
		func clear_collection():
			_collection.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_collection(value):
			_collection.value = value
		
		var _record
		func get_record():
			return _record.value
		func clear_record():
			_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_record(value):
			_record.value = value
		
		var _user_id
		func get_user_id():
			return _user_id.value
		func clear_user_id():
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_user_id(value):
			_user_id.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TStorageData:
	func _init():
		var service
		
		_data = PBField.new("data", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _data
		service.func_ref = funcref(self, "add_data")
		data[_data.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
	var data = {}
	
	var _data
	func get_data():
		return _data.value
	func clear_data():
		_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_data():
		var element = TStorageData.StorageData.new()
		_data.value.append(element)
		return element
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	class StorageData:
		func _init():
			var service
			
			_bucket = PBField.new("bucket", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _bucket
			data[_bucket.tag] = service
			
			_collection = PBField.new("collection", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _collection
			data[_collection.tag] = service
			
			_record = PBField.new("record", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _record
			data[_record.tag] = service
			
			_user_id = PBField.new("user_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _user_id
			data[_user_id.tag] = service
			
			_value = PBField.new("value", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _value
			data[_value.tag] = service
			
			_version = PBField.new("version", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _version
			data[_version.tag] = service
			
			_permission_read = PBField.new("permission_read", PB_DATA_TYPE.INT32, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT32])
			service = PBServiceField.new()
			service.field = _permission_read
			data[_permission_read.tag] = service
			
			_permission_write = PBField.new("permission_write", PB_DATA_TYPE.INT32, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT32])
			service = PBServiceField.new()
			service.field = _permission_write
			data[_permission_write.tag] = service
			
			_created_at = PBField.new("created_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 9, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _created_at
			data[_created_at.tag] = service
			
			_updated_at = PBField.new("updated_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 10, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _updated_at
			data[_updated_at.tag] = service
			
			_expires_at = PBField.new("expires_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 11, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _expires_at
			data[_expires_at.tag] = service
			
		var data = {}
		
		var _bucket
		func get_bucket():
			return _bucket.value
		func clear_bucket():
			_bucket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_bucket(value):
			_bucket.value = value
		
		var _collection
		func get_collection():
			return _collection.value
		func clear_collection():
			_collection.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_collection(value):
			_collection.value = value
		
		var _record
		func get_record():
			return _record.value
		func clear_record():
			_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_record(value):
			_record.value = value
		
		var _user_id
		func get_user_id():
			return _user_id.value
		func clear_user_id():
			_user_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_user_id(value):
			_user_id.value = value
		
		var _value
		func get_value():
			return _value.value
		func clear_value():
			_value.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_value(value):
			_value.value = value
		
		var _version
		func get_version():
			return _version.value
		func clear_version():
			_version.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_version(value):
			_version.value = value
		
		var _permission_read
		func get_permission_read():
			return _permission_read.value
		func clear_permission_read():
			_permission_read.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT32]
		func set_permission_read(value):
			_permission_read.value = value
		
		var _permission_write
		func get_permission_write():
			return _permission_write.value
		func clear_permission_write():
			_permission_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT32]
		func set_permission_write(value):
			_permission_write.value = value
		
		var _created_at
		func get_created_at():
			return _created_at.value
		func clear_created_at():
			_created_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_created_at(value):
			_created_at.value = value
		
		var _updated_at
		func get_updated_at():
			return _updated_at.value
		func clear_updated_at():
			_updated_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_updated_at(value):
			_updated_at.value = value
		
		var _expires_at
		func get_expires_at():
			return _expires_at.value
		func clear_expires_at():
			_expires_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_expires_at(value):
			_expires_at.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TStorageWrite:
	func _init():
		var service
		
		_data = PBField.new("data", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 3, true, [])
		service = PBServiceField.new()
		service.field = _data
		service.func_ref = funcref(self, "add_data")
		data[_data.tag] = service
		
	var data = {}
	
	var _data
	func get_data():
		return _data.value
	func clear_data():
		_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_data():
		var element = TStorageWrite.StorageData.new()
		_data.value.append(element)
		return element
	
	class StorageData:
		func _init():
			var service
			
			_bucket = PBField.new("bucket", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _bucket
			data[_bucket.tag] = service
			
			_collection = PBField.new("collection", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _collection
			data[_collection.tag] = service
			
			_record = PBField.new("record", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _record
			data[_record.tag] = service
			
			_value = PBField.new("value", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _value
			data[_value.tag] = service
			
			_version = PBField.new("version", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _version
			data[_version.tag] = service
			
			_permission_read = PBField.new("permission_read", PB_DATA_TYPE.INT32, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT32])
			service = PBServiceField.new()
			service.field = _permission_read
			data[_permission_read.tag] = service
			
			_permission_write = PBField.new("permission_write", PB_DATA_TYPE.INT32, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT32])
			service = PBServiceField.new()
			service.field = _permission_write
			data[_permission_write.tag] = service
			
		var data = {}
		
		var _bucket
		func get_bucket():
			return _bucket.value
		func clear_bucket():
			_bucket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_bucket(value):
			_bucket.value = value
		
		var _collection
		func get_collection():
			return _collection.value
		func clear_collection():
			_collection.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_collection(value):
			_collection.value = value
		
		var _record
		func get_record():
			return _record.value
		func clear_record():
			_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_record(value):
			_record.value = value
		
		var _value
		func get_value():
			return _value.value
		func clear_value():
			_value.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_value(value):
			_value.value = value
		
		var _version
		func get_version():
			return _version.value
		func clear_version():
			_version.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_version(value):
			_version.value = value
		
		var _permission_read
		func get_permission_read():
			return _permission_read.value
		func clear_permission_read():
			_permission_read.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT32]
		func set_permission_read(value):
			_permission_read.value = value
		
		var _permission_write
		func get_permission_write():
			return _permission_write.value
		func clear_permission_write():
			_permission_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT32]
		func set_permission_write(value):
			_permission_write.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TStorageUpdate:
	func _init():
		var service
		
		_updates = PBField.new("updates", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _updates
		service.func_ref = funcref(self, "add_updates")
		data[_updates.tag] = service
		
	var data = {}
	
	var _updates
	func get_updates():
		return _updates.value
	func clear_updates():
		_updates.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_updates():
		var element = TStorageUpdate.StorageUpdate.new()
		_updates.value.append(element)
		return element
	
	class StorageUpdate:
		func _init():
			var service
			
			_key = PBField.new("key", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
			service = PBServiceField.new()
			service.field = _key
			service.func_ref = funcref(self, "new_key")
			data[_key.tag] = service
			
			_permission_read = PBField.new("permission_read", PB_DATA_TYPE.INT32, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT32])
			service = PBServiceField.new()
			service.field = _permission_read
			data[_permission_read.tag] = service
			
			_permission_write = PBField.new("permission_write", PB_DATA_TYPE.INT32, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT32])
			service = PBServiceField.new()
			service.field = _permission_write
			data[_permission_write.tag] = service
			
			_ops = PBField.new("ops", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 4, true, [])
			service = PBServiceField.new()
			service.field = _ops
			service.func_ref = funcref(self, "add_ops")
			data[_ops.tag] = service
			
		var data = {}
		
		var _key
		func get_key():
			return _key.value
		func clear_key():
			_key.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		func new_key():
			_key.value = TStorageUpdate.StorageUpdate.StorageKey.new()
			return _key.value
		
		var _permission_read
		func get_permission_read():
			return _permission_read.value
		func clear_permission_read():
			_permission_read.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT32]
		func set_permission_read(value):
			_permission_read.value = value
		
		var _permission_write
		func get_permission_write():
			return _permission_write.value
		func clear_permission_write():
			_permission_write.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT32]
		func set_permission_write(value):
			_permission_write.value = value
		
		var _ops
		func get_ops():
			return _ops.value
		func clear_ops():
			_ops.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		func add_ops():
			var element = TStorageUpdate.StorageUpdate.UpdateOp.new()
			_ops.value.append(element)
			return element
		
		class UpdateOp:
			func _init():
				var service
				
				_op = PBField.new("op", PB_DATA_TYPE.INT32, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT32])
				service = PBServiceField.new()
				service.field = _op
				data[_op.tag] = service
				
				_path = PBField.new("path", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
				service = PBServiceField.new()
				service.field = _path
				data[_path.tag] = service
				
				_value = PBField.new("value", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
				service = PBServiceField.new()
				service.field = _value
				data[_value.tag] = service
				
				_from = PBField.new("from", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
				service = PBServiceField.new()
				service.field = _from
				data[_from.tag] = service
				
				_conditional = PBField.new("conditional", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
				service = PBServiceField.new()
				service.field = _conditional
				data[_conditional.tag] = service
				
				_assert = PBField.new("assert", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
				service = PBServiceField.new()
				service.field = _assert
				data[_assert.tag] = service
				
				_ops = PBField.new("ops", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 7, true, [])
				service = PBServiceField.new()
				service.field = _ops
				service.func_ref = funcref(self, "add_ops")
				data[_ops.tag] = service
				
			var data = {}
			
			var _op
			func get_op():
				return _op.value
			func clear_op():
				_op.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT32]
			func set_op(value):
				_op.value = value
			
			var _path
			func get_path():
				return _path.value
			func clear_path():
				_path.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			func set_path(value):
				_path.value = value
			
			var _value
			func get_value():
				return _value.value
			func clear_value():
				_value.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			func set_value(value):
				_value.value = value
			
			var _from
			func get_from():
				return _from.value
			func clear_from():
				_from.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			func set_from(value):
				_from.value = value
			
			var _conditional
			func get_conditional():
				return _conditional.value
			func clear_conditional():
				_conditional.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
			func set_conditional(value):
				_conditional.value = value
			
			var _assert
			func get_assert():
				return _assert.value
			func clear_assert():
				_assert.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			func set_assert(value):
				_assert.value = value
			
			var _ops
			func get_ops():
				return _ops.value
			func clear_ops():
				_ops.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
			func add_ops():
				var element = TStorageUpdate.StorageUpdate.UpdateOp.new()
				_ops.value.append(element)
				return element
			
			enum UpdateOpCode {
				ADD = 0,
				APPEND = 1,
				COPY = 2,
				INCR = 3,
				INIT = 4,
				MERGE = 5,
				MOVE = 6,
				PATCH = 7,
				REMOVE = 8,
				REPLACE = 9,
				TEST = 10,
				COMPARE = 11
			}
			
			func to_string():
				return PBPacker.message_to_string(data)
				
			func to_bytes():
				return PBPacker.pack_message(data)
				
			func from_bytes(bytes, offset = 0, limit = -1):
				var cur_limit = bytes.size()
				if limit != -1:
					cur_limit = limit
				var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
				if result == cur_limit:
					if PBPacker.check_required(data):
						if limit == -1:
							return PB_ERR.NO_ERRORS
					else:
						return PB_ERR.REQUIRED_FIELDS
				elif limit == -1 && result > 0:
					return PB_ERR.PARSE_INCOMPLETE
				return result
			
		class StorageKey:
			func _init():
				var service
				
				_bucket = PBField.new("bucket", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
				service = PBServiceField.new()
				service.field = _bucket
				data[_bucket.tag] = service
				
				_collection = PBField.new("collection", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
				service = PBServiceField.new()
				service.field = _collection
				data[_collection.tag] = service
				
				_record = PBField.new("record", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
				service = PBServiceField.new()
				service.field = _record
				data[_record.tag] = service
				
				_version = PBField.new("version", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
				service = PBServiceField.new()
				service.field = _version
				data[_version.tag] = service
				
			var data = {}
			
			var _bucket
			func get_bucket():
				return _bucket.value
			func clear_bucket():
				_bucket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			func set_bucket(value):
				_bucket.value = value
			
			var _collection
			func get_collection():
				return _collection.value
			func clear_collection():
				_collection.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			func set_collection(value):
				_collection.value = value
			
			var _record
			func get_record():
				return _record.value
			func clear_record():
				_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			func set_record(value):
				_record.value = value
			
			var _version
			func get_version():
				return _version.value
			func clear_version():
				_version.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
			func set_version(value):
				_version.value = value
			
			func to_string():
				return PBPacker.message_to_string(data)
				
			func to_bytes():
				return PBPacker.pack_message(data)
				
			func from_bytes(bytes, offset = 0, limit = -1):
				var cur_limit = bytes.size()
				if limit != -1:
					cur_limit = limit
				var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
				if result == cur_limit:
					if PBPacker.check_required(data):
						if limit == -1:
							return PB_ERR.NO_ERRORS
					else:
						return PB_ERR.REQUIRED_FIELDS
				elif limit == -1 && result > 0:
					return PB_ERR.PARSE_INCOMPLETE
				return result
			
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TStorageKeys:
	func _init():
		var service
		
		_keys = PBField.new("keys", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _keys
		service.func_ref = funcref(self, "add_keys")
		data[_keys.tag] = service
		
	var data = {}
	
	var _keys
	func get_keys():
		return _keys.value
	func clear_keys():
		_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_keys():
		var element = TStorageKeys.StorageKey.new()
		_keys.value.append(element)
		return element
	
	class StorageKey:
		func _init():
			var service
			
			_bucket = PBField.new("bucket", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _bucket
			data[_bucket.tag] = service
			
			_collection = PBField.new("collection", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _collection
			data[_collection.tag] = service
			
			_record = PBField.new("record", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _record
			data[_record.tag] = service
			
			_version = PBField.new("version", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _version
			data[_version.tag] = service
			
		var data = {}
		
		var _bucket
		func get_bucket():
			return _bucket.value
		func clear_bucket():
			_bucket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_bucket(value):
			_bucket.value = value
		
		var _collection
		func get_collection():
			return _collection.value
		func clear_collection():
			_collection.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_collection(value):
			_collection.value = value
		
		var _record
		func get_record():
			return _record.value
		func clear_record():
			_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_record(value):
			_record.value = value
		
		var _version
		func get_version():
			return _version.value
		func clear_version():
			_version.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_version(value):
			_version.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TStorageRemove:
	func _init():
		var service
		
		_keys = PBField.new("keys", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _keys
		service.func_ref = funcref(self, "add_keys")
		data[_keys.tag] = service
		
	var data = {}
	
	var _keys
	func get_keys():
		return _keys.value
	func clear_keys():
		_keys.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_keys():
		var element = TStorageRemove.StorageKey.new()
		_keys.value.append(element)
		return element
	
	class StorageKey:
		func _init():
			var service
			
			_bucket = PBField.new("bucket", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _bucket
			data[_bucket.tag] = service
			
			_collection = PBField.new("collection", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _collection
			data[_collection.tag] = service
			
			_record = PBField.new("record", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _record
			data[_record.tag] = service
			
			_version = PBField.new("version", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _version
			data[_version.tag] = service
			
		var data = {}
		
		var _bucket
		func get_bucket():
			return _bucket.value
		func clear_bucket():
			_bucket.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_bucket(value):
			_bucket.value = value
		
		var _collection
		func get_collection():
			return _collection.value
		func clear_collection():
			_collection.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_collection(value):
			_collection.value = value
		
		var _record
		func get_record():
			return _record.value
		func clear_record():
			_record.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_record(value):
			_record.value = value
		
		var _version
		func get_version():
			return _version.value
		func clear_version():
			_version.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_version(value):
			_version.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class Leaderboard:
	func _init():
		var service
		
		_id = PBField.new("id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _id
		data[_id.tag] = service
		
		_authoritative = PBField.new("authoritative", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _authoritative
		data[_authoritative.tag] = service
		
		_sort = PBField.new("sort", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _sort
		data[_sort.tag] = service
		
		_count = PBField.new("count", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _count
		data[_count.tag] = service
		
		_reset_schedule = PBField.new("reset_schedule", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _reset_schedule
		data[_reset_schedule.tag] = service
		
		_metadata = PBField.new("metadata", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _metadata
		data[_metadata.tag] = service
		
	var data = {}
	
	var _id
	func get_id():
		return _id.value
	func clear_id():
		_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_id(value):
		_id.value = value
	
	var _authoritative
	func get_authoritative():
		return _authoritative.value
	func clear_authoritative():
		_authoritative.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_authoritative(value):
		_authoritative.value = value
	
	var _sort
	func get_sort():
		return _sort.value
	func clear_sort():
		_sort.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_sort(value):
		_sort.value = value
	
	var _count
	func get_count():
		return _count.value
	func clear_count():
		_count.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_count(value):
		_count.value = value
	
	var _reset_schedule
	func get_reset_schedule():
		return _reset_schedule.value
	func clear_reset_schedule():
		_reset_schedule.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_reset_schedule(value):
		_reset_schedule.value = value
	
	var _metadata
	func get_metadata():
		return _metadata.value
	func clear_metadata():
		_metadata.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_metadata(value):
		_metadata.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class LeaderboardRecord:
	func _init():
		var service
		
		_leaderboard_id = PBField.new("leaderboard_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _leaderboard_id
		data[_leaderboard_id.tag] = service
		
		_owner_id = PBField.new("owner_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _owner_id
		data[_owner_id.tag] = service
		
		_handle = PBField.new("handle", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _handle
		data[_handle.tag] = service
		
		_lang = PBField.new("lang", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _lang
		data[_lang.tag] = service
		
		_location = PBField.new("location", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _location
		data[_location.tag] = service
		
		_timezone = PBField.new("timezone", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _timezone
		data[_timezone.tag] = service
		
		_rank = PBField.new("rank", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _rank
		data[_rank.tag] = service
		
		_score = PBField.new("score", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _score
		data[_score.tag] = service
		
		_num_score = PBField.new("num_score", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 9, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _num_score
		data[_num_score.tag] = service
		
		_metadata = PBField.new("metadata", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 10, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _metadata
		data[_metadata.tag] = service
		
		_ranked_at = PBField.new("ranked_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 11, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _ranked_at
		data[_ranked_at.tag] = service
		
		_updated_at = PBField.new("updated_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 12, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _updated_at
		data[_updated_at.tag] = service
		
		_expires_at = PBField.new("expires_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 13, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _expires_at
		data[_expires_at.tag] = service
		
	var data = {}
	
	var _leaderboard_id
	func get_leaderboard_id():
		return _leaderboard_id.value
	func clear_leaderboard_id():
		_leaderboard_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_leaderboard_id(value):
		_leaderboard_id.value = value
	
	var _owner_id
	func get_owner_id():
		return _owner_id.value
	func clear_owner_id():
		_owner_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_owner_id(value):
		_owner_id.value = value
	
	var _handle
	func get_handle():
		return _handle.value
	func clear_handle():
		_handle.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_handle(value):
		_handle.value = value
	
	var _lang
	func get_lang():
		return _lang.value
	func clear_lang():
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_lang(value):
		_lang.value = value
	
	var _location
	func get_location():
		return _location.value
	func clear_location():
		_location.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_location(value):
		_location.value = value
	
	var _timezone
	func get_timezone():
		return _timezone.value
	func clear_timezone():
		_timezone.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_timezone(value):
		_timezone.value = value
	
	var _rank
	func get_rank():
		return _rank.value
	func clear_rank():
		_rank.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_rank(value):
		_rank.value = value
	
	var _score
	func get_score():
		return _score.value
	func clear_score():
		_score.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_score(value):
		_score.value = value
	
	var _num_score
	func get_num_score():
		return _num_score.value
	func clear_num_score():
		_num_score.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_num_score(value):
		_num_score.value = value
	
	var _metadata
	func get_metadata():
		return _metadata.value
	func clear_metadata():
		_metadata.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_metadata(value):
		_metadata.value = value
	
	var _ranked_at
	func get_ranked_at():
		return _ranked_at.value
	func clear_ranked_at():
		_ranked_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_ranked_at(value):
		_ranked_at.value = value
	
	var _updated_at
	func get_updated_at():
		return _updated_at.value
	func clear_updated_at():
		_updated_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_updated_at(value):
		_updated_at.value = value
	
	var _expires_at
	func get_expires_at():
		return _expires_at.value
	func clear_expires_at():
		_expires_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_expires_at(value):
		_expires_at.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TLeaderboardsList:
	func _init():
		var service
		
		_limit = PBField.new("limit", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _limit
		data[_limit.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
		_filter_leaderboard_id = PBField.new("filter_leaderboard_id", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 3, true, [])
		service = PBServiceField.new()
		service.field = _filter_leaderboard_id
		data[_filter_leaderboard_id.tag] = service
		
	var data = {}
	
	var _limit
	func get_limit():
		return _limit.value
	func clear_limit():
		_limit.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_limit(value):
		_limit.value = value
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	var _filter_leaderboard_id
	func get_filter_leaderboard_id():
		return _filter_leaderboard_id.value
	func clear_filter_leaderboard_id():
		_filter_leaderboard_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func add_filter_leaderboard_id(value):
		_filter_leaderboard_id.value.append(value)
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TLeaderboards:
	func _init():
		var service
		
		_leaderboards = PBField.new("leaderboards", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _leaderboards
		service.func_ref = funcref(self, "add_leaderboards")
		data[_leaderboards.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
	var data = {}
	
	var _leaderboards
	func get_leaderboards():
		return _leaderboards.value
	func clear_leaderboards():
		_leaderboards.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_leaderboards():
		var element = Leaderboard.new()
		_leaderboards.value.append(element)
		return element
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TLeaderboardRecordsWrite:
	func _init():
		var service
		
		_records = PBField.new("records", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _records
		service.func_ref = funcref(self, "add_records")
		data[_records.tag] = service
		
	var data = {}
	
	var _records
	func get_records():
		return _records.value
	func clear_records():
		_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_records():
		var element = TLeaderboardRecordsWrite.LeaderboardRecordWrite.new()
		_records.value.append(element)
		return element
	
	class LeaderboardRecordWrite:
		func _init():
			var service
			
			_leaderboard_id = PBField.new("leaderboard_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _leaderboard_id
			data[_leaderboard_id.tag] = service
			
			_incr = PBField.new("incr", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _incr
			data[_incr.tag] = service
			
			_decr = PBField.new("decr", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _decr
			data[_decr.tag] = service
			
			_set = PBField.new("set", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _set
			data[_set.tag] = service
			
			_best = PBField.new("best", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
			service = PBServiceField.new()
			service.field = _best
			data[_best.tag] = service
			
			_location = PBField.new("location", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _location
			data[_location.tag] = service
			
			_timezone = PBField.new("timezone", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _timezone
			data[_timezone.tag] = service
			
			_metadata = PBField.new("metadata", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _metadata
			data[_metadata.tag] = service
			
		var data = {}
		
		var _leaderboard_id
		func get_leaderboard_id():
			return _leaderboard_id.value
		func clear_leaderboard_id():
			_leaderboard_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_leaderboard_id(value):
			_leaderboard_id.value = value
		
		var _incr
		func has_incr():
			if data[2].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_incr():
			return _incr.value
		func clear_incr():
			_incr.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_incr(value):
			_decr.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_set.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_best.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_incr.value = value
		
		var _decr
		func has_decr():
			if data[3].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_decr():
			return _decr.value
		func clear_decr():
			_decr.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_decr(value):
			_incr.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_set.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_best.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_decr.value = value
		
		var _set
		func has_set():
			if data[4].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_set():
			return _set.value
		func clear_set():
			_set.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_set(value):
			_incr.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_decr.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_best.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_set.value = value
		
		var _best
		func has_best():
			if data[5].state == PB_SERVICE_STATE.FILLED:
				return true
			return false
		func get_best():
			return _best.value
		func clear_best():
			_best.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
		func set_best(value):
			_incr.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_decr.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_set.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
			_best.value = value
		
		var _location
		func get_location():
			return _location.value
		func clear_location():
			_location.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_location(value):
			_location.value = value
		
		var _timezone
		func get_timezone():
			return _timezone.value
		func clear_timezone():
			_timezone.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_timezone(value):
			_timezone.value = value
		
		var _metadata
		func get_metadata():
			return _metadata.value
		func clear_metadata():
			_metadata.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_metadata(value):
			_metadata.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TLeaderboardRecordsFetch:
	func _init():
		var service
		
		_leaderboard_ids = PBField.new("leaderboard_ids", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _leaderboard_ids
		data[_leaderboard_ids.tag] = service
		
		_limit = PBField.new("limit", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _limit
		data[_limit.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
	var data = {}
	
	var _leaderboard_ids
	func get_leaderboard_ids():
		return _leaderboard_ids.value
	func clear_leaderboard_ids():
		_leaderboard_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func add_leaderboard_ids(value):
		_leaderboard_ids.value.append(value)
	
	var _limit
	func get_limit():
		return _limit.value
	func clear_limit():
		_limit.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_limit(value):
		_limit.value = value
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TLeaderboardRecordsList:
	func _init():
		var service
		
		_leaderboard_id = PBField.new("leaderboard_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _leaderboard_id
		data[_leaderboard_id.tag] = service
		
		_owner_id = PBField.new("owner_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _owner_id
		data[_owner_id.tag] = service
		
		_owner_ids = PBField.new("owner_ids", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _owner_ids
		service.func_ref = funcref(self, "new_owner_ids")
		data[_owner_ids.tag] = service
		
		_lang = PBField.new("lang", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _lang
		data[_lang.tag] = service
		
		_location = PBField.new("location", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _location
		data[_location.tag] = service
		
		_timezone = PBField.new("timezone", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _timezone
		data[_timezone.tag] = service
		
		_limit = PBField.new("limit", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _limit
		data[_limit.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
	var data = {}
	
	var _leaderboard_id
	func get_leaderboard_id():
		return _leaderboard_id.value
	func clear_leaderboard_id():
		_leaderboard_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_leaderboard_id(value):
		_leaderboard_id.value = value
	
	var _owner_id
	func has_owner_id():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_owner_id():
		return _owner_id.value
	func clear_owner_id():
		_owner_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_owner_id(value):
		_owner_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_location.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_timezone.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_owner_id.value = value
	
	var _owner_ids
	func has_owner_ids():
		if data[3].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_owner_ids():
		return _owner_ids.value
	func clear_owner_ids():
		_owner_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_owner_ids():
		_owner_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_location.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_timezone.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_owner_ids.value = TLeaderboardRecordsList.Owners.new()
		return _owner_ids.value
	
	var _lang
	func has_lang():
		if data[4].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_lang():
		return _lang.value
	func clear_lang():
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_lang(value):
		_owner_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_owner_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_location.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_timezone.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_lang.value = value
	
	var _location
	func has_location():
		if data[5].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_location():
		return _location.value
	func clear_location():
		_location.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_location(value):
		_owner_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_owner_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_timezone.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_location.value = value
	
	var _timezone
	func has_timezone():
		if data[6].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_timezone():
		return _timezone.value
	func clear_timezone():
		_timezone.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_timezone(value):
		_owner_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_owner_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_lang.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_location.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		_timezone.value = value
	
	var _limit
	func get_limit():
		return _limit.value
	func clear_limit():
		_limit.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_limit(value):
		_limit.value = value
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	class Owners:
		func _init():
			var service
			
			_owner_ids = PBField.new("owner_ids", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
			service = PBServiceField.new()
			service.field = _owner_ids
			data[_owner_ids.tag] = service
			
		var data = {}
		
		var _owner_ids
		func get_owner_ids():
			return _owner_ids.value
		func clear_owner_ids():
			_owner_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func add_owner_ids(value):
			_owner_ids.value.append(value)
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TLeaderboardRecords:
	func _init():
		var service
		
		_records = PBField.new("records", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _records
		service.func_ref = funcref(self, "add_records")
		data[_records.tag] = service
		
		_cursor = PBField.new("cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _cursor
		data[_cursor.tag] = service
		
	var data = {}
	
	var _records
	func get_records():
		return _records.value
	func clear_records():
		_records.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_records():
		var element = LeaderboardRecord.new()
		_records.value.append(element)
		return element
	
	var _cursor
	func get_cursor():
		return _cursor.value
	func clear_cursor():
		_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_cursor(value):
		_cursor.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TRpc:
	func _init():
		var service
		
		_id = PBField.new("id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _id
		data[_id.tag] = service
		
		_payload = PBField.new("payload", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _payload
		data[_payload.tag] = service
		
	var data = {}
	
	var _id
	func get_id():
		return _id.value
	func clear_id():
		_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_id(value):
		_id.value = value
	
	var _payload
	func get_payload():
		return _payload.value
	func clear_payload():
		_payload.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_payload(value):
		_payload.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TPurchaseValidation:
	func _init():
		var service
		
		_apple_purchase = PBField.new("apple_purchase", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _apple_purchase
		service.func_ref = funcref(self, "new_apple_purchase")
		data[_apple_purchase.tag] = service
		
		_google_purchase = PBField.new("google_purchase", PB_DATA_TYPE.MESSAGE, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE])
		service = PBServiceField.new()
		service.field = _google_purchase
		service.func_ref = funcref(self, "new_google_purchase")
		data[_google_purchase.tag] = service
		
	var data = {}
	
	var _apple_purchase
	func has_apple_purchase():
		if data[1].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_apple_purchase():
		return _apple_purchase.value
	func clear_apple_purchase():
		_apple_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_apple_purchase():
		_google_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_apple_purchase.value = TPurchaseValidation.ApplePurchase.new()
		return _apple_purchase.value
	
	var _google_purchase
	func has_google_purchase():
		if data[2].state == PB_SERVICE_STATE.FILLED:
			return true
		return false
	func get_google_purchase():
		return _google_purchase.value
	func clear_google_purchase():
		_google_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func new_google_purchase():
		_apple_purchase.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
		_google_purchase.value = TPurchaseValidation.GooglePurchase.new()
		return _google_purchase.value
	
	class ApplePurchase:
		func _init():
			var service
			
			_product_id = PBField.new("product_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _product_id
			data[_product_id.tag] = service
			
			_receipt_data = PBField.new("receipt_data", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _receipt_data
			data[_receipt_data.tag] = service
			
		var data = {}
		
		var _product_id
		func get_product_id():
			return _product_id.value
		func clear_product_id():
			_product_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_product_id(value):
			_product_id.value = value
		
		var _receipt_data
		func get_receipt_data():
			return _receipt_data.value
		func clear_receipt_data():
			_receipt_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_receipt_data(value):
			_receipt_data.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	class GooglePurchase:
		func _init():
			var service
			
			_product_id = PBField.new("product_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _product_id
			data[_product_id.tag] = service
			
			_product_type = PBField.new("product_type", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _product_type
			data[_product_type.tag] = service
			
			_purchase_token = PBField.new("purchase_token", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
			service = PBServiceField.new()
			service.field = _purchase_token
			data[_purchase_token.tag] = service
			
		var data = {}
		
		var _product_id
		func get_product_id():
			return _product_id.value
		func clear_product_id():
			_product_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_product_id(value):
			_product_id.value = value
		
		var _product_type
		func get_product_type():
			return _product_type.value
		func clear_product_type():
			_product_type.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_product_type(value):
			_product_type.value = value
		
		var _purchase_token
		func get_purchase_token():
			return _purchase_token.value
		func clear_purchase_token():
			_purchase_token.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
		func set_purchase_token(value):
			_purchase_token.value = value
		
		func to_string():
			return PBPacker.message_to_string(data)
			
		func to_bytes():
			return PBPacker.pack_message(data)
			
		func from_bytes(bytes, offset = 0, limit = -1):
			var cur_limit = bytes.size()
			if limit != -1:
				cur_limit = limit
			var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
			if result == cur_limit:
				if PBPacker.check_required(data):
					if limit == -1:
						return PB_ERR.NO_ERRORS
				else:
					return PB_ERR.REQUIRED_FIELDS
			elif limit == -1 && result > 0:
				return PB_ERR.PARSE_INCOMPLETE
			return result
		
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TPurchaseRecord:
	func _init():
		var service
		
		_success = PBField.new("success", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _success
		data[_success.tag] = service
		
		_seen_before = PBField.new("seen_before", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _seen_before
		data[_seen_before.tag] = service
		
		_purchase_provider_reachable = PBField.new("purchase_provider_reachable", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _purchase_provider_reachable
		data[_purchase_provider_reachable.tag] = service
		
		_msg = PBField.new("msg", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _msg
		data[_msg.tag] = service
		
		_data = PBField.new("data", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _data
		data[_data.tag] = service
		
	var data = {}
	
	var _success
	func get_success():
		return _success.value
	func clear_success():
		_success.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_success(value):
		_success.value = value
	
	var _seen_before
	func get_seen_before():
		return _seen_before.value
	func clear_seen_before():
		_seen_before.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_seen_before(value):
		_seen_before.value = value
	
	var _purchase_provider_reachable
	func get_purchase_provider_reachable():
		return _purchase_provider_reachable.value
	func clear_purchase_provider_reachable():
		_purchase_provider_reachable.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_purchase_provider_reachable(value):
		_purchase_provider_reachable.value = value
	
	var _msg
	func get_msg():
		return _msg.value
	func clear_msg():
		_msg.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_msg(value):
		_msg.value = value
	
	var _data
	func get_data():
		return _data.value
	func clear_data():
		_data.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_data(value):
		_data.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class Notification:
	func _init():
		var service
		
		_id = PBField.new("id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _id
		data[_id.tag] = service
		
		_subject = PBField.new("subject", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _subject
		data[_subject.tag] = service
		
		_content = PBField.new("content", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 3, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _content
		data[_content.tag] = service
		
		_code = PBField.new("code", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 4, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _code
		data[_code.tag] = service
		
		_sender_id = PBField.new("sender_id", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 5, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _sender_id
		data[_sender_id.tag] = service
		
		_created_at = PBField.new("created_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 6, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _created_at
		data[_created_at.tag] = service
		
		_expires_at = PBField.new("expires_at", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 7, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _expires_at
		data[_expires_at.tag] = service
		
		_persistent = PBField.new("persistent", PB_DATA_TYPE.BOOL, PB_RULE.OPTIONAL, 8, true, DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL])
		service = PBServiceField.new()
		service.field = _persistent
		data[_persistent.tag] = service
		
	var data = {}
	
	var _id
	func get_id():
		return _id.value
	func clear_id():
		_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_id(value):
		_id.value = value
	
	var _subject
	func get_subject():
		return _subject.value
	func clear_subject():
		_subject.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_subject(value):
		_subject.value = value
	
	var _content
	func get_content():
		return _content.value
	func clear_content():
		_content.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_content(value):
		_content.value = value
	
	var _code
	func get_code():
		return _code.value
	func clear_code():
		_code.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_code(value):
		_code.value = value
	
	var _sender_id
	func get_sender_id():
		return _sender_id.value
	func clear_sender_id():
		_sender_id.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_sender_id(value):
		_sender_id.value = value
	
	var _created_at
	func get_created_at():
		return _created_at.value
	func clear_created_at():
		_created_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_created_at(value):
		_created_at.value = value
	
	var _expires_at
	func get_expires_at():
		return _expires_at.value
	func clear_expires_at():
		_expires_at.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_expires_at(value):
		_expires_at.value = value
	
	var _persistent
	func get_persistent():
		return _persistent.value
	func clear_persistent():
		_persistent.value = DEFAULT_VALUES_3[PB_DATA_TYPE.BOOL]
	func set_persistent(value):
		_persistent.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class Notifications:
	func _init():
		var service
		
		_notifications = PBField.new("notifications", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _notifications
		service.func_ref = funcref(self, "add_notifications")
		data[_notifications.tag] = service
		
	var data = {}
	
	var _notifications
	func get_notifications():
		return _notifications.value
	func clear_notifications():
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_notifications():
		var element = Notification.new()
		_notifications.value.append(element)
		return element
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TNotificationsList:
	func _init():
		var service
		
		_limit = PBField.new("limit", PB_DATA_TYPE.INT64, PB_RULE.OPTIONAL, 1, true, DEFAULT_VALUES_3[PB_DATA_TYPE.INT64])
		service = PBServiceField.new()
		service.field = _limit
		data[_limit.tag] = service
		
		_resumable_cursor = PBField.new("resumable_cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _resumable_cursor
		data[_resumable_cursor.tag] = service
		
	var data = {}
	
	var _limit
	func get_limit():
		return _limit.value
	func clear_limit():
		_limit.value = DEFAULT_VALUES_3[PB_DATA_TYPE.INT64]
	func set_limit(value):
		_limit.value = value
	
	var _resumable_cursor
	func get_resumable_cursor():
		return _resumable_cursor.value
	func clear_resumable_cursor():
		_resumable_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_resumable_cursor(value):
		_resumable_cursor.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TNotifications:
	func _init():
		var service
		
		_notifications = PBField.new("notifications", PB_DATA_TYPE.MESSAGE, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _notifications
		service.func_ref = funcref(self, "add_notifications")
		data[_notifications.tag] = service
		
		_resumable_cursor = PBField.new("resumable_cursor", PB_DATA_TYPE.STRING, PB_RULE.OPTIONAL, 2, true, DEFAULT_VALUES_3[PB_DATA_TYPE.STRING])
		service = PBServiceField.new()
		service.field = _resumable_cursor
		data[_resumable_cursor.tag] = service
		
	var data = {}
	
	var _notifications
	func get_notifications():
		return _notifications.value
	func clear_notifications():
		_notifications.value = DEFAULT_VALUES_3[PB_DATA_TYPE.MESSAGE]
	func add_notifications():
		var element = Notification.new()
		_notifications.value.append(element)
		return element
	
	var _resumable_cursor
	func get_resumable_cursor():
		return _resumable_cursor.value
	func clear_resumable_cursor():
		_resumable_cursor.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func set_resumable_cursor(value):
		_resumable_cursor.value = value
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
class TNotificationsRemove:
	func _init():
		var service
		
		_notification_ids = PBField.new("notification_ids", PB_DATA_TYPE.STRING, PB_RULE.REPEATED, 1, true, [])
		service = PBServiceField.new()
		service.field = _notification_ids
		data[_notification_ids.tag] = service
		
	var data = {}
	
	var _notification_ids
	func get_notification_ids():
		return _notification_ids.value
	func clear_notification_ids():
		_notification_ids.value = DEFAULT_VALUES_3[PB_DATA_TYPE.STRING]
	func add_notification_ids(value):
		_notification_ids.value.append(value)
	
	func to_string():
		return PBPacker.message_to_string(data)
		
	func to_bytes():
		return PBPacker.pack_message(data)
		
	func from_bytes(bytes, offset = 0, limit = -1):
		var cur_limit = bytes.size()
		if limit != -1:
			cur_limit = limit
		var result = PBPacker.unpack_message(data, bytes, offset, cur_limit)
		if result == cur_limit:
			if PBPacker.check_required(data):
				if limit == -1:
					return PB_ERR.NO_ERRORS
			else:
				return PB_ERR.REQUIRED_FIELDS
		elif limit == -1 && result > 0:
			return PB_ERR.PARSE_INCOMPLETE
		return result
	
################ USER DATA END #################
