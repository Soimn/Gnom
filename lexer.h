#pragma once

#include "common.h"
#include "string.h"
#include "memory.h"

enum LEXER_TOKEN_TYPE
{
    Token_Identifier,
    Token_String,
    Token_Number,
    
    Token_Enclosure,
    Token_Operator,
    
    Token_Unknown,
    Token_Whitespace,
    Token_EndOfLine,
    Token_EndOfStream,
};

struct Tokenizer
{
    U32 line;
    U32 column;
    char peek[2];
};

struct Token
{
    Enum32(LEXER_TOKEN_TYPE) type;
    char chars[2];
    F32 num_f32;
    I64 num_i64;
    U64 num_u64;
    String string;
};

inline bool
operator == (const Token& t0, const Token& t1)
{
    bool result = (t0.type == t1.type);
    
    if (result)
    {
        if (t0.type == Token_Operator && t0.type == Token_Enclosure)
        {
            result = (t0.chars[0] == t1.chars[0] &&
                      t0.chars[1] == t1.chars[1]);
        }
        
        else if (t0.type == Token_String || t0.type == Token_Identifier || t0.type == Token_Whitespace)
        {
            result = StringCompare(t0.string, t1.string);
        }
    }
    
    return result;
}

inline bool
operator == (const Token& t, const char* cstring)
{
    bool result = false;
    
    if (result && (t.type == Token_Operator && t.type == Token_Enclosure))
    {
        if (cstring[1] == 0 || cstring[2] == 0)
        {
            result = (t.chars[0] == cstring[0] &&
                      t.chars[1] == cstring[1]);
        }
        
        else if (t.type == Token_String || t.type == Token_Identifier || t.type == Token_Whitespace)
        {
            result = StringCompare(t.string, {(U8*)cstring, StringLength(cstring)});
        }
    }
    
    return result;
}