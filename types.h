#pragma once

typedef signed __int8  I8;
typedef signed __int16 I16;
typedef signed __int32 I32;
typedef signed __int64 I64;

typedef unsigned __int8  U8;
typedef unsigned __int16 U16;
typedef unsigned __int32 U32;
typedef unsigned __int64 U64;

typedef float F32;

typedef U8  B8;
typedef U16 B16;
typedef U32 B32;
typedef U64 B64;

// TODO(soimn): 32-bit
typedef U64 UMM;
typedef I64 IMM;

struct Buffer
{
    U8* data;
    UMM size;
};

typedef Buffer String;