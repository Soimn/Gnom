#pragma once

#include "types.h"

#define internal static
#define global static
#define local_persist static

#define ARRAY_COUNT(array) (sizeof((array)) / sizeof((array)[0]))

#define BYTES(N) (N)
#define KILOBYTES(N) (BYTES(N) * 1024ULL)
#define MEGABYTES(N) (KILOBYTES(N) * 1024ULL)
#define GIGABYTES(N) (MEGABYTES(N) * 1024ULL)
#define TERRABYTES(N) (GIGABYTES(N) * 1024ULL)

#define OFFSETOF(obj, var) (UMM) &(((obj*)0)->var)

#define INVALID_CODE_PATH Assert(!"Invalid code path")
#define INVALID_DEFAULT_CASE default: INVALID_CODE_PATH; break
#define NOT_IMPLEMENTED Assert(!"Not implemented");

#define CONST_STRING(string) {(U8*)string, sizeof(string) - 1}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define U8_MAX  (U8)  0xFF
#define U16_MAX (U16) 0xFFFF
#define U32_MAX (U32) 0xFFFFFFFF
#define U64_MAX (U64) 0xFFFFFFFFFFFFFFFF

#define U8_MIN  (U8)  0
#define U16_MIN (U16) 0
#define U32_MIN (U32) 0
#define U64_MIN (U64) 0

#define I8_MAX  (I8)  (U8_MAX  >> 1)
#define I16_MAX (I16) (U16_MAX >> 1)
#define I32_MAX (I32) (U32_MAX >> 1)
#define I64_MAX (I64) (U64_MAX >> 1)

#define I8_MIN  (I8)  (~I8_MAX)
#define I16_MIN (I16) (~I16_MAX)
#define I32_MIN (I32) (~I32_MAX)
#define I64_MIN (I64) (~I64_MAX)

// TODO(soimn): Should these be floats or doubles?
#define F32_MIN -3.402823466e+38
#define F32_MAX  3.402823466e+38
#define F32_UNSIGNED_MIN 1.175494351e-38

#define Flag8(type)  U8
#define Flag16(type) U16
#define Flag32(type) U32
#define Flag64(type) U64

#define Enum8(type)  U8
#define Enum16(type) U16
#define Enum32(type) U32
#define Enum64(type) U64

/// 
/// ERROR HANDLING FUNCTIIONALITY
/// 

[[noreturn]]
inline void
AssertionFailed(const char* file, const char* function, U32 line, const char* condition_string, String message, ...);
[[noreturn]]
inline void
AssertionFailed(const char* file, const char* function, U32 line, const char* condition_string, const char* message, ...);

[[noreturn]]
inline void
AssertionFailed(const char* file, const char* fucntion, U32 line, const char* condition_string);

inline void
Report(Enum8(REPORT_SEVERITY) severity, String message, ...);

inline void
Report(Enum8(REPORT_SEVERITY) severity, const char* message, ...);

#ifndef DISABLE_ASSERT
#define Assert(condition, ...) ((condition) ? 1 : AssertionFailed(__FILE__, __FUNCTION__, __LINE__, #condition, ##__VA_ARGS__), 0)
#else
#define Assert(condition, ...) (condition)
#endif

inline void
Flush(struct String_Stream* stream);

global struct String_Stream* PrintStream;
global struct String_Stream* ErrorStream;