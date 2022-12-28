#include "core/ustring.h"

String string_format(const char *p_format, ...) {
	va_list list;

	va_start(list, p_format);
	String res = string_format(p_format, list);
	va_end(list);

	return res;
}

#ifdef DEBUG_ENABLED
void print_debug() {
}
#else
void print_debug() {
}
#endif
