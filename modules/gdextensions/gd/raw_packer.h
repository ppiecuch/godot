#ifndef RAW_PACKER_H
#define RAW_PACKER_H

#include "core/error_list.h"
#include "core/error_macros.h"
#include "core/io/marshalls.h"
#include "core/reference.h"
#include "core/ustring.h"
#include "core/variant.h"

class RawPacker : public Reference {
	GDCLASS(RawPacker,Reference);

	Error encode(const String& fmt, const Array& array, uint8_t *buf, int &len);
	Error decode(const String& fmt, Array& array, const uint8_t *buf, int size);
	
	bool is_digit(char c);
	
protected:
	static void _bind_methods();

public:
	PoolByteArray pack(const String& fmt, const Array& array);
	Array unpack(const String& fmt, const PoolByteArray& array);
};

#endif // RAW_PACKER_H
