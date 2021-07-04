//
//    Copyright (c) 2020-2021 Benjamin 'drwhut' Beddows
//

#include "error_reporter.h"

ErrorReporter::ErrorReporter() {
    eh.errfunc = _error_handler;
    eh.userdata = this;
    add_error_handler(&eh);
}

ErrorReporter::~ErrorReporter() {}

void ErrorReporter::deinit() {
    remove_error_handler(&eh);
}

void ErrorReporter::_bind_methods() {
    ADD_SIGNAL(MethodInfo("error_received", PropertyInfo(Variant::STRING, "func"), PropertyInfo(Variant::STRING, "file"), PropertyInfo(Variant::INT, "line"), PropertyInfo(Variant::STRING, "error"), PropertyInfo(Variant::STRING, "errorexp")));
    ADD_SIGNAL(MethodInfo("warning_received", PropertyInfo(Variant::STRING, "func"), PropertyInfo(Variant::STRING, "file"), PropertyInfo(Variant::INT, "line"), PropertyInfo(Variant::STRING, "error"), PropertyInfo(Variant::STRING, "errorexp")));
}

void ErrorReporter::_error_handler(void *p_self, const char *p_func, const char *p_file, int p_line, const char *p_error, const char *p_errorexp, ErrorHandlerType p_type) {
    ErrorReporter *self = (ErrorReporter *)p_self;

    String func_str = String(p_func);
    String file_str = String(p_file);
    String error_str = String(p_error);
    String errorexp_str = String(p_errorexp);

    if (p_type == ERR_HANDLER_WARNING) {
        self->emit_signal("warning_received", func_str, file_str, p_line, error_str, errorexp_str);
    } else {
        self->emit_signal("error_received", func_str, file_str, p_line, error_str, errorexp_str);
    }
}
