#ifndef GD_CORE_H
#define GD_CORE_H

#include "core/class_db.h"
#include "core/ustring.h"
#include "core/os/os.h"

#define safe_delete(x) (delete x,x = nullptr)

#ifdef DEBUG_ENABLED
# define DEBUG_PRINT(m_text) print_line(m_text);
#else
# define DEBUG_PRINT(m_text)
#endif


#ifdef DEBUG_ENABLED

// Bind constant with custom name
#define BIND_ENUM_CONSTANT_CUSTOM(m_constant, m_name) \
	ClassDB::bind_integer_constant(get_class_static(), __constant_get_enum_name(m_constant, m_name), m_name, ((int)(m_constant)));

#else

// Bind constant with custom name
#define BIND_ENUM_CONSTANT_CUSTOM(m_constant, m_name) \
	ClassDB::bind_integer_constant(get_class_static(), StringName(), m_name, ((int)(m_constant)));

#endif // DEBUG_ENABLED


static inline void trace(int line, const char *file, const String &text) {
	OS::get_singleton()->print("%s", text.utf8().get_data());
}
#define TRACE(text, ...) trace(__LINE__, __FILE__, vformat(text, __VA_ARGS__))


#endif // GD_CORE_H
