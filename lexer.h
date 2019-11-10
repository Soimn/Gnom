#pragma once

#include "common.h"
#include "string.h"
#include "memory.h"

enum LEXER_TOKEN_TYPE
{
    Token_Identifier,
    Token_Character, // TODO(soimn): GetTokenRaw unicode character constant support
    Token_String,
    Token_INT,
    Token_F32,
    Token_F64,
    
    Token_Plus,               // +
    Token_Inc,                // ++
    Token_PlusEquals,         // +=
    Token_Minus,              // -
    Token_Dec,                // --
    Token_MinusEquals,        // -=
    Token_Divide,             // /
    Token_DivideEquals,       // /=
    Token_Asterisk,           // * (multiply or dereference)
    Token_MultiplyEquals,     // *=
    Token_Modulo,             // %
    Token_ModuloEquals,       // %=
    Token_Equals,             // =
    Token_EqualTo,            // ==
    Token_LogicalNot,         // !
    Token_NotEqual,           // !=
    Token_GreaterThan,        // >
    Token_GreaterThanOrEqual, // >=
    Token_RightShift,         // >>
    Token_RightShiftEquals,   // >>=
    Token_LessThan,           // <
    Token_LessThanOrEqual,    // <=
    Token_LeftShift,          // <<
    Token_LeftShiftEquals,    // <<=
    Token_Ampersand,          // & (bitwise and or reference)
    Token_LogicalAnd,         // &&
    Token_AndEquals,          // &=
    Token_Or,                 // |
    Token_LogicalOr,          // ||
    Token_OrEquals,           // |=
    Token_Not,                // ~
    Token_XOR,                // ^
    Token_QuestionMark,       // ?
    Token_Colon,              // :
    Token_Dot,                // .
    Token_Elipsis,            // ...
    Token_Comma,              // ,
    Token_Semicolon,          // ;
    
    Token_OpenParen,          // (
    Token_CloseParen,         // )
    Token_OpenBrace,          // {
    Token_CloseBrace,         // }
    Token_OpenBracket,        // [
    Token_CloseBracket,       // ]
    
    
    Token_Unknown,
    Token_Comment,
    Token_Whitespace,
    Token_EndOfLine,
    Token_EndOfStream,
    
    Token_Error,
};

struct Lexer
{
    String_Stream_Interval file;
    U32 line;
    U32 column;
    char peek[2];
    Bucket_Array_Iterator iterator;
};

struct Token
{
    Enum32(LEXER_TOKEN_TYPE) type;
    
    union
    {
        // NOTE(soimn): The number is always positive, since minus tokens are handled before the parsing of a 
        //              number
        U64 num_u64;
        F32 num_f32;
        F64 num_f64;
        
        String_Stream_Interval string;
        
        U32 whitespace_length;
        
        struct
        {
            U8 bytes[4];
            U8 width;
        } character;
    };
    
};

inline void
Refill(Lexer* lexer)
{
    char* peek_0 = (char*)PeekForward(&lexer->iterator, 0);
    char* peek_1 = (char*)PeekForward(&lexer->iterator, 1);
    
    lexer->peek[0] = (peek_0 != 0 ? *peek_0 : 0);
    lexer->peek[1] = (peek_1 != 0 ? *peek_1 : 0);
}

inline void
Advance(Lexer* lexer, U32 amount)
{
    for (U32 i = 0; (i < amount) && lexer->iterator.current; ++i)
    {
        ++lexer->column;
        
        // TODO(soimn): Watch out for file and line tags
        
        if (IsEndOfLine(lexer->peek[0]))
        {
            ++lexer->line;
            lexer->column = 0;
        }
        
        Advance(&lexer->iterator);
        Refill(lexer);
    }
}

inline Lexer
LexStringStream(String_Stream stream)
{
    Lexer lexer = {};
    
    lexer.iterator = Iterate(&stream.bucket_array);
    Refill(&lexer);
    
    return lexer;
}

inline Token
GetTokenRaw(Lexer* lexer)
{
    Token token = {};
    
    char c = lexer->peek[0];
    Advance(lexer, 1);
    
    switch (c)
    {
        case 0: token.type = Token_EndOfStream; break;
        case '\n': token.type = Token_EndOfLine; break;
        
        case '+': token.type = Token_Plus; break;
        case '-': token.type = Token_Minus; break;
        //case '/': token.type = Token_Divide; break;
        case '*': token.type = Token_Asterisk; break;
        case '%': token.type = Token_Modulo; break;
        case '=': token.type = Token_Equals; break;
        case '!': token.type = Token_LogicalNot; break;
        case '>': token.type = Token_GreaterThan; break;
        case '<': token.type = Token_LessThan; break;
        case '&': token.type = Token_Ampersand; break;
        case '|': token.type = Token_Or; break;
        case '~': token.type = Token_Not; break;
        case '^': token.type = Token_XOR; break;
        case '?': token.type = Token_QuestionMark; break;
        case ':': token.type = Token_Colon; break;
        //case '.': token.type = Token_Dot; break;
        case ',': token.type = Token_Comma; break;
        case ';': token.type = Token_Semicolon; break;
        case '(': token.type = Token_OpenParen; break;
        case ')': token.type = Token_CloseParen; break;
        case '{': token.type = Token_OpenBrace; break;
        case '}': token.type = Token_CloseBrace; break;
        case '[': token.type = Token_OpenBracket; break;
        case ']': token.type = Token_CloseBracket; break;
        
        default:
        {
            if (IsWhitespace(c))
            {
                token.type = Token_Whitespace;
                token.whitespace_length = 1;
                
                while (lexer->peek[0] != 0 && IsWhitespace(lexer->peek[0]))
                {
                    ++token.whitespace_length;
                    Advance(lexer, 1);
                } 
            }
            
            else if (c == '/')
            {
                if (lexer->peek[0] == '*')
                {
                    Advance(lexer, 1);
                    
                    token.type = Token_Comment;
                    token.string.first_block = lexer->iterator.current_block;
                    token.string.index       = lexer->iterator.current_index;
                    token.string.block_size  = lexer->iterator.block_size;
                    
                    while (lexer->peek[0] != 0 && !(lexer->peek[0] == '*' && lexer->peek[1] == '/'))
                    {
                        ++token.string.size;
                        Advance(lexer, 1);
                    }
                    
                    Advance(lexer, 2);
                }
                
                else if (lexer->peek[0] == '/')
                {
                    Advance(lexer, 1);
                    
                    token.type = Token_Comment;
                    token.string.first_block = lexer->iterator.current_block;
                    token.string.index       = lexer->iterator.current_index;
                    token.string.block_size  = lexer->iterator.block_size;
                    
                    while (lexer->peek[0] != 0 && !IsEndOfLine(lexer->peek[0]))
                    {
                        ++token.string.size;
                        Advance(lexer, 1);
                    }
                    
                    Advance(lexer, 1);
                }
                
                else
                {
                    token.type = Token_Divide;
                }
            }
            
            else if (IsAlpha(c) || c == '_')
            {
                token.type = Token_Identifier;
                
                token.string.first_block = lexer->iterator.current_block;
                token.string.index       = lexer->iterator.current_index;
                token.string.block_size  = lexer->iterator.block_size;
                token.string.size        = 1;
                
                while (lexer->peek[0] != 0 && IsAlpha(lexer->peek[0]) || IsNumeric(lexer->peek[0]) || lexer->peek[0] == '_')
                {
                    ++token.string.size;
                    Advance(lexer, 1);
                }
            }
            
            else if (c == '"')
            {
                token.type = Token_String;
                
                token.string.first_block = lexer->iterator.current_block;
                token.string.index       = lexer->iterator.current_index;
                token.string.block_size  = lexer->iterator.block_size;
                
                while (lexer->peek[0] != 0 && lexer->peek[0] != '"')
                {
                    ++token.string.size;
                    Advance(lexer, 1);
                }
                
                if (lexer->peek[0] == 0)
                {
                    //// ERROR: Reached end of stream before closing '"'
                    token.type = Token_Error;
                }
                
                else
                {
                    // NOTE(soimn): Skip terminating '"'
                    Advance(lexer, 1);
                }
            }
            
            else if (c == '\'')
            {
                token.type = Token_Character;
                
                // TODO(soimn): This does not work with unicode
                if (lexer->peek[0] != '\'' && lexer->peek[1] == '\'')
                {
                    token.character.width = 1;
                    token.character.bytes[0] = (U8)lexer->peek[0];
                    Advance(lexer, 2);
                }
                
                else
                {
                    token.type = Token_Error;
                    
                    if (lexer->peek[0] == '\'')
                    {
                        //// ERROR: Empty character constant
                        token.type = Token_Error;
                    }
                    
                    else
                    {
                        //// ERROR: Missing terminating ' character
                        token.type = Token_Error;
                    }
                }
            }
            
            else if (IsNumeric(c) || c == '.')
            {
                if (c == '.' && !IsNumeric(lexer->peek[0]))
                {
                    token.type = Token_Dot;
                }
                
                else
                {
                    bool is_hex    = false;
                    bool is_octal  = false;
                    bool is_binary = false;
                    
                    if (c == '0')
                    {
                        is_hex    = (lexer->peek[0] == 'x' || lexer->peek[0] == 'X');
                        is_octal  = IsNumeric(lexer->peek[0]);
                        is_binary = (lexer->peek[0] == 'b' || lexer->peek[0] == 'B');
                    }
                    
                    if (is_hex || is_octal || is_binary)
                    {
                        Advance(lexer, 1);
                        token.type = Token_INT;
                        
                        U64 base = (U64)(is_hex ? 16 : (is_octal ? 8 : 2));
                        
                        U64 last_num = 0;
                        for (;;)
                        {
                            if (token.num_u64 < last_num)
                            {
                                //// ERROR: Integer literal is too large to be represented in any integer type
                                token.type = Token_Error;
                            }
                            
                            if (IsNumeric(lexer->peek[0]))
                            {
                                U8 digit = (U8)(lexer->peek[0] - '0');
                                
                                last_num = token.num_u64;
                                
                                token.num_u64 *= base;
                                token.num_u64 += digit;
                                
                                if (is_binary && digit > 1)
                                {
                                    //// ERROR: Invalid digit '%u' in binary constant
                                    token.type = Token_Error;
                                }
                                
                                else if (is_octal && digit > 7)
                                {
                                    //// ERROR: Invalid digit '%u' in octal constant
                                    token.type = Token_Error;
                                }
                            }
                            
                            else if (is_hex && (ToUpper(lexer->peek[0]) >= 'A' && ToUpper(lexer->peek[0]) <= 'F'))
                            {
                                last_num = token.num_u64;
                                
                                token.num_u64 *= 16;
                                token.num_u64 += (ToUpper(lexer->peek[0]) - 'A') + 10;
                            }
                            
                            else
                            {
                                break;
                            }
                            
                            Advance(lexer, 1);
                        }
                    }
                    
                    else
                    {
                        U64 acc         = (U64)(IsNumeric(c) ? c - '0' : 0);
                        I32 point_place = (c == '.' ? 0 : -1);
                        I16 exponent    = 0;
                        
                        bool ended_in_float_specifier = false;
                        bool has_exponent             = false;
                        bool detected_overflow        = false;
                        
                        U64 last_acc = 0;
                        I32 length   = (IsNumeric(c) ? 1 : 0);
                        for (;; ++length)
                        {
                            if (!detected_overflow && IsNumeric(lexer->peek[0]))
                            {
                                acc *= 10;
                                acc += lexer->peek[0] - '0';
                                
                                if (acc < last_acc)
                                {
                                    // NOTE(soimn): Continue parsing the number as if all is good
                                    detected_overflow = true;
                                    acc = last_acc;
                                }
                            }
                            
                            else if (!IsNumeric(lexer->peek[0]))
                            {
                                if (lexer->peek[0] == '.')
                                {
                                    if (point_place == -1 && IsNumeric(lexer->peek[1]))
                                    {
                                        point_place = length;
                                    }
                                    
                                    else break;
                                }
                                
                                else if (ToLower(lexer->peek[0]) == 'f')
                                {
                                    ended_in_float_specifier = true;
                                    Advance(lexer, 1);
                                    break;
                                }
                                
                                else if (ToLower(lexer->peek[0]) == 'e')
                                {
                                    if (IsNumeric(lexer->peek[1]) || lexer->peek[1] == '+' || lexer->peek[1] == '-')
                                    {
                                        Advance(lexer, 1);
                                        
                                        char sign = lexer->peek[0];
                                        
                                        if (lexer->peek[0] == '+' || lexer->peek[0] == '-')
                                        {
                                            if (IsNumeric(lexer->peek[1]))
                                            {
                                                Advance(lexer, 1);
                                            }
                                            
                                            else
                                            {
                                                //// ERROR: Floating point number ended in exponent with no digits
                                                token.type = Token_Error;
                                                break;
                                            }
                                        }
                                        
                                        if (token.type != Token_Error)
                                        {
                                            has_exponent = true;
                                            
                                            exponent = lexer->peek[0] - '0';
                                            Advance(lexer, 1);
                                            
                                            I16 last_exponent = exponent;
                                            while (IsNumeric(lexer->peek[0]))
                                            {
                                                exponent *= 10;
                                                exponent += lexer->peek[0] - '0';
                                                Advance(lexer, 1);
                                                
                                                if (exponent < last_exponent)
                                                {
                                                    //// ERROR: Magnitude of floating-point constant too large
                                                    token.type = Token_Error;
                                                }
                                            }
                                            
                                            exponent *= (sign == '-' ? -1 : 1);
                                        }
                                    }
                                }
                                
                                else break;
                                
                            }
                            
                            Advance(lexer, 1);
                        }
                        
                        if (token.type != Token_Error)
                        {
                            if (ended_in_float_specifier || has_exponent || point_place != -1)
                            {
                                if (ended_in_float_specifier)
                                {
                                    // TODO(soimn): Convert number to float
                                }
                                
                                else
                                {
                                    // TODO(soimn): Convert number to double
                                }
                            }
                            
                            else
                            {
                                if (!detected_overflow)
                                {
                                    token.type    = Token_INT;
                                    token.num_u64 = acc;
                                }
                                
                                else
                                {
                                    //// ERROR: Integer literal is too large to be represented in any integer type
                                    token.type = Token_Error;
                                }
                            }
                        }
                    }
                }
            }
            
            else
            {
                token.type = Token_Unknown;
            }
        } break;
    }
    
    return token;
}

inline Token
PeekTokenRaw(Lexer* lexer)
{
    Lexer temp_lexer = *lexer;
    
    return GetTokenRaw(&temp_lexer);
}

inline Token
GetToken(Lexer* lexer)
{
    Token result = {};
    
    do
    {
        result = GetTokenRaw(lexer);
    } 
    while (result.type == Token_Whitespace || 
           result.type == Token_EndOfLine || 
           result.type == Token_Comment);
    
    return result;
}

inline Token
PeekToken(Lexer* lexer)
{
    Lexer temp_lexer = *lexer;
    
    return GetToken(&temp_lexer);
}