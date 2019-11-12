#pragma once

#include "common.h"
#include "error_handling.h"
#include "lexer.h"
#include "ast.h"

inline bool
IsKeyword(String_Stream_Interval string)
{
    bool result = false;
    
    String all_keywords[] = {
        CONST_STRING("if"),
        CONST_STRING("else"),
        CONST_STRING("while"),
        CONST_STRING("for"),
        CONST_STRING("do"),
        CONST_STRING("signed"),
        CONST_STRING("unsigned"),
        CONST_STRING("inline"),
        CONST_STRING("internal"),
        CONST_STRING("static"),
        CONST_STRING("enum"),
        CONST_STRING("struct"),
        CONST_STRING("union"),
    };
    
    for (U32 i = 0; i < ARRAY_COUNT(all_keywords); ++i)
    {
        if (StringCompare(all_keywords[i], string))
        {
            result = true;
            break;
        }
    }
    
    return result;
}

inline bool
ParseTypedef(Lexer* lexer)
{
    
}

inline ret_type
ParseStringStream(String_Stream stream)
{
    Lexer lexer = LexStringStream(stream);
    
    do
    {
        Token first_token = GetToken(&lexer);
        
        if (first_token.type == Token_Identifier)
        {
            if (StringCompare(first_token.string, CONST_STRING("typedef")))
            {
                ParseTypedef(&lexer);
            }
            
            Token second_token = GetToken(&lexer);
            Token third_token  = PeekToken(&lexer);
            
            if (StringCompare(first_token.string, CONST_STRING("struct")) || StringCompare(first_token.string, CONST_STRING("union")))
            {
                // TODO(soimn): Check if this is a foward decl or not
                // TODO(soimn): Check if this is a var decl or not
                // TODO(soimn): Check if valid and parse struct or union
            }
            
            else if (StringCompare(first_token.string, CONST_STRING("enum")))
            {
                // TODO(soimn): Check if valid and parse enum
            }
            
            else
            {
                // NOTE(soimn): var decl or function decl or function definition
            }
        }
        
        else
        {
            //// 
        }
        
    } while (token.type != Token_EndOfStream);
}