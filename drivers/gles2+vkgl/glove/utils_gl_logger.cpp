/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 */

/// GloveLogger is an easy-to-use monitor interface that can be used for
/// developing, debugging and running OpenGL ES applications.

#include "utils_gl_logger.h"

#include <string.h>

GloveLogger *GloveLogger::mInstance = nullptr;
GloveLoggerImpl *GloveLogger::mLoggerImpl = nullptr;

GloveLogger::GloveLogger()
{
    mLoggerImpl->Initialize();
}

GloveLogger::~GloveLogger()
{
    mLoggerImpl->Destroy();
}

void
GloveLogger::SetLoggerImpl()
{
    if(!mLoggerImpl) {
        #ifdef VK_USE_PLATFORM_ANDROID_KHR
            mLoggerImpl = new AndroidGloveLoggerImpl();
        #else
            mLoggerImpl = new SimpleLoggerImpl();
        #endif //VK_USE_PLATFORM_ANDROID_KHR
    }
}

GloveLogger *
GloveLogger::GetInstance()
{
    SetLoggerImpl();

    if(!mInstance) {
        mInstance = new GloveLogger();
    }

    return mInstance;
}

void
GloveLogger::DestroyInstance()
{
    if(mInstance) {
        delete mInstance;
        mInstance = nullptr;
    }

    if(mLoggerImpl) {
        delete mLoggerImpl;
        mLoggerImpl = nullptr;
    }
}

int
GloveLogger::CalculateSpacesBefore(glLogLevel_e level)
{
    return ( 8 - (int)level*4 );
}

int
GloveLogger::CalculateSpacesAfter(const char *func, glLogLevel_e level)
{
    return  funcWidth - (int)strlen(func)  - 8 + (int)level*4;
}

void
GloveLogger::WriteFunEntry(glLogLevel_e level, const char* filename, const char *func, int line)
{
    char log[200];
    snprintf(log, 200, "%*s%s()%*s  [ %s: %d  Context: c1 ]", CalculateSpacesBefore(level), "", func, CalculateSpacesAfter(func, level), "", filename, line);
    mLoggerImpl->WriteLog(level, log);
}

void
GloveLogger::FunEntry(glLogLevel_e level, const char* filename, const char *func, int line)
{
    GloveLogger *glLogger = GloveLogger::GetInstance();
    glLogger->WriteFunEntry(level, filename, func, line);
}

void
GloveLogger::Log(glLogLevel_e level, const char *format, ... )
{
    char log[200];
    va_list args;
    va_start(args, format);
    vsnprintf(log, 200, format, args);
    va_end(args);
    mLoggerImpl->WriteLog(level, log);
}

void
GloveLogger::Shutdown()
{
    GloveLogger::DestroyInstance();
}
