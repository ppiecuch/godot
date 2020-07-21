#ifndef GODOTERRORHANDLER_H
#define GODOTERRORHANDLER_H

#include <inttypes.h>

#include "core/error_macros.h"
#include "core/dictionary.h"
#include "core/object.h"
#include "core/reference.h"
#include "core/script_language.h"

class GodotErrorHandler : public Object
{
public:
	static GodotErrorHandler *get_singleton();
	GodotErrorHandler();
	~GodotErrorHandler();

	void handle_error(Dictionary errorObject);

protected:
	static void _bind_methods();
	static GodotErrorHandler *singleton;

struct ErrorObject {

	String source_file;
	String source_func;
	int source_line;
	String error;
	String error_descr;
	bool warning;
	Array callstack;
};

private:
	static void _err_handler(void *, const char *, const char *, int p_line, const char *, const char *, ErrorHandlerType p_type);

	ErrorHandlerList eh;

	GDCLASS(GodotErrorHandler, Object);
	OBJ_CATEGORY("GodotErrorHandler");
};
#endif // GODOTERRORHANDLER_H
