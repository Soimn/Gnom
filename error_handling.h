#pragma once

#include "common.h"
#include "string.h"
#include "memory.h"

// TODO(soimn): Implement a proper abort
inline void
Abort()
{
    *(volatile int*)0 = 0;
}

[[noreturn]]
inline void
AssertionFailed(const char* file, const char* function, U32 line, const char* condition_string, String message, ...)
{
    va_list arg_list;
    va_start(arg_list, message);
    Print(ErrorStream, "*********** [ASSERTION FAILED] ***********\n");
    Print(ErrorStream, "The assertion '%s' failed.\n", condition_string);
    Print(ErrorStream, "File: %s,\nLine: %u,Function: %s\n\n", file, line, function);
    Print(ErrorStream, message, arg_list);
    Flush(ErrorStream);
    va_end(arg_list);
    
    Abort();
}

[[noreturn]]
inline void
AssertionFailed(const char* file, const char* function, U32 line, const char* condition_string, const char* message, ...)
{
    va_list arg_list;
    va_start(arg_list, message);
    Print(ErrorStream, "*********** [ASSERTION FAILED] ***********\n");
    Print(ErrorStream, "The assertion '%s' failed.\n", condition_string);
    Print(ErrorStream, "File: %s,\nLine: %u,\nFunction: %s\n\n", file, line, function);
    Print(ErrorStream, {(U8*)message, StringLength(message)}, arg_list);
    Flush(ErrorStream);
    va_end(arg_list);
    
    Abort();
}

[[noreturn]]
inline void
AssertionFailed(const char* file, const char* function, U32 line, const char* condition_string)
{
    Print(ErrorStream, "*********** [ASSERTION FAILED] ***********\n");
    Print(ErrorStream, "The assertion '%s' failed.\n", condition_string);
    Print(ErrorStream, "File: %s,\nLine: %u,Function: %s\n", file, line, function);
    Flush(ErrorStream);
    
    Abort();
}

enum REPORT_SEVERITY
{
    Warning,
    Error,
    Fatal,
};

inline void
Report(Enum8(REPORT_SEVERITY) severity, String message, ...)
{
    va_list arg_list;
    va_start(arg_list, message);
    
    String severity_string = CONST_STRING("");
    
    switch (severity)
    {
        case Warning:
        severity_string = CONST_STRING("WARNING");
        break;
        
        case Error:
        severity_string = CONST_STRING("ERROR");
        break;
        
        case Fatal:
        severity_string = CONST_STRING("FATAL");
        break;
        
        INVALID_DEFAULT_CASE;
    }
    
    Print(ErrorStream, "[%S] ", severity_string);
    Print(ErrorStream, message, arg_list);
    Append(ErrorStream, '\n');
    
    if (severity != Warning)
    {
        Flush(ErrorStream);
    }
    
    va_end(arg_list);
}

inline void
Report(Enum8(REPORT_SEVERITY) severity, const char* message, ...)
{
    va_list arg_list;
    va_start(arg_list, message);
    
    String severity_string = CONST_STRING("");
    
    switch (severity)
    {
        case Warning:
        severity_string = CONST_STRING("WARNING");
        break;
        
        case Error:
        severity_string = CONST_STRING("ERROR");
        break;
        
        case Fatal:
        severity_string = CONST_STRING("FATAL");
        break;
        
        INVALID_DEFAULT_CASE;
    }
    
    Print(ErrorStream, "[%S] ", severity_string);
    Print(ErrorStream, {(U8*)message, StringLength(message)}, arg_list);
    Append(ErrorStream, '\n');
    
    if (severity != Warning)
    {
        Flush(ErrorStream);
    }
    
    va_end(arg_list);
}