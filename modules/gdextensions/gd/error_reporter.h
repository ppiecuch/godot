
//
//    Copyright (c) 2020-2021 Benjamin 'drwhut' Beddows
//

#ifndef ERROR_REPORTER_H
#define ERROR_REPORTER_H

#include "core/reference.h"

class ErrorReporter : public Reference {
    GDCLASS(ErrorReporter, Reference);

public:
    ErrorReporter();
    ~ErrorReporter();

    void deinit();

protected:
    static void _bind_methods();
    static void _error_handler(void *p_self, const char *p_func, const char *p_file, int p_line, const char *p_error, const char *p_errorexp, ErrorHandlerType p_type);

    ErrorHandlerList eh;
};

#endif // ERROR_REPORTER_H
