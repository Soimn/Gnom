#pragma once

#include "common.h"
#include "string.h"
#include "memory.h"

/// --- FUNCTIONS ---
// Assert : assertion of a value, used only for internal checking, 
//          terminates in debug mode and prints an error message 
//          regardless of mode
//
// Report : prints an error message based on severity

[[noreturn]]
inline void
AssertionFailed(const char* file, const char* function, U32 line, const char* condition_string, String message, ...)
{
    // Print error message
    *(volatile int*)0 = 0;
}

[[noreturn]]
inline void
AssertionFailed(const char* file, const char* function, U32 line, const char* condition_string, const char* message, ...)
{
    // Print error message
    *(volatile int*)0 = 0;
}

[[noreturn]]
inline void
AssertionFailed(const char* file, const char* fucntion, U32 line, const char* condition_string)
{
    // Print error message
    *(volatile int*)0 = 0;
}

enum REPORT_SEVERITY
{
    Report_Warning,
    Report_Error,
    Report_Fatal,
};

inline void
Report(Enum8(REPORT_SEVERITY) severity, String message, ...)
{
}

inline void
Report(Enum8(REPORT_SEVERITY) severity, const char* message, ...)
{
}