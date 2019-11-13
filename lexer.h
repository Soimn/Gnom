#pragma once

#include "common.h"
#include "string.h"
#include "memory.h"

enum LEXER_TOKEN_TYPE
{
    Token_Struct,
    Token_Union,
    Token_Enum,
    Token_Typedef,
    Token_If,
    Token_Else,
    Token_Do,
    Token_While,
    Token_For,
    
    Token_Identifier,
    // TODO(soimn): GetTokenRaw unicode character constant support
    Token_Character,
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
    Token_NotEquals,          // ~=
    Token_XOR,                // ^
    Token_XOREquals,          // ^=
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
    File_ID file;
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
GetTokenRaw(Lexer* lexer, bool eat_all_whitespace_and_comments)
{
    Token token = {};
    
    if (eat_all_whitespace_and_comments)
    {
        for (;;)
        {
            if (IsSpacing(lexer->peek[0]))
            {
                Advance(lexer, 1);
            }
            
            else if (lexer->peek[0] == '/' && lexer->peek[1] == '/')
            {
                while (lexer->peek[0] != 0 && !IsEndOfLine(lexer->peek[0]))
                {
                    Advance(lexer, 1);
                }
                
                Advance(lexer, 1);
            }
            
            else if (lexer->peek[0] == '/' && lexer->peek[1] == '*')
            {
                while (lexer->peek[0] != 0 && !(lexer->peek[0] == '*' && lexer->peek[1] == '/'))
                {
                    Advance(lexer, 1);
                }
                
                Advance(lexer, 2);
            }
            
            else break;
        }
    }
    
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
                
                if (StringCompare(token.string, CONST_STRING("struct")))
                {
                    token.type = Token_Struct;
                }
                
                else if (StringCompare(token.string, CONST_STRING("union")))
                {
                    token.type = Token_Union;
                }
                
                else if (StringCompare(token.string, CONST_STRING("enum")))
                {
                    token.type = Token_Enum;
                }
                
                else if (StringCompare(token.string, CONST_STRING("typedef")))
                {
                    token.type = Token_Typedef;
                }
                
                else if (StringCompare(token.string, CONST_STRING("if")))
                {
                    token.type = Token_If;
                }
                
                else if (StringCompare(token.string, CONST_STRING("else")))
                {
                    token.type = Token_Else;
                }
                
                else if (StringCompare(token.string, CONST_STRING("do")))
                {
                    token.type = Token_Do;
                }
                
                else if (StringCompare(token.string, CONST_STRING("while")))
                {
                    token.type = Token_While;
                }
                
                else if (StringCompare(token.string, CONST_STRING("for")))
                {
                    token.type = Token_For;
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
                                // IMPORTANT TODO(soimn): This method of parsing floats is not precise enough to be acceptable. Find another 
                                //                        way of parsing floats.
                                I32 decimal_exponent = ((point_place != -1 ? point_place : length) - length) + exponent;
                                
                                F64 adjuster = 1.0f;
                                for (U32 i = 0; i < (U32)(decimal_exponent < 0 ? -decimal_exponent : decimal_exponent); ++i)
                                {
                                    adjuster *= 10.0f;
                                }
                                
                                F64 num = (F64)acc / adjuster;
                                
                                if (ended_in_float_specifier)
                                {
                                    if (num > F32_MIN && num < F32_MAX)
                                    {
                                        token.type    = Token_F32;
                                        token.num_f32 = num;
                                    }
                                    
                                    else
                                    {
                                        //// ERROR: Floating point literal too large to be represented by type 'float'
                                        token.type = Token_Error;
                                    }
                                }
                                
                                else
                                {
                                    token.type    = Token_F64;
                                    token.num_f64 = num;
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
PeekTokenRaw(Lexer* lexer, bool eat_all_whitespace_and_comments = false)
{
    Lexer temp_lexer = *lexer;
    
    return GetTokenRaw(&temp_lexer, eat_all_whitespace_and_comments);
}

inline Token
GetToken(Lexer* lexer)
{
    Token result = {};
    
    result     = GetTokenRaw(lexer, true);
    Token peek = PeekTokenRaw(lexer, true);
    
    Token old_result = result;
    
    switch (result.type)
    {
        case Token_Plus:
        {
            if (peek.type == Token_Plus)
            {
                result.type = Token_Inc;
            }
            
            else if (peek.type == Token_Equals)
            {
                result.type = Token_PlusEquals;
            }
        } break;
        
        case Token_Minus:
        {
            if (peek.type == Token_Minus)
            {
                result.type = Token_Dec;
            }
            
            else if (peek.type == Token_Equals)
            {
                result.type = Token_MinusEquals;
            }
        } break;
        
        case Token_Divide:
        {
            if (peek.type == Token_Equals)
            {
                result.type = Token_DivideEquals;
            }
        } break;
        
        case Token_Asterisk:
        {
            if (peek.type == Token_Equals)
            {
                result.type = Token_MultiplyEquals;
            }
            
            else
            {
                // TODO(soimn): Should dereference and multiply be differentiated by the 
                //              lexer or the parser?
            }
        } break;
        
        case Token_Modulo:
        {
            if (peek.type == Token_Equals)
            {
                result.type = Token_ModuloEquals;
            }
        } break;
        
        case Token_Equals:
        {
            if (peek.type == Token_Equals)
            {
                result.type = Token_EqualTo;
            }
        } break;
        
        case Token_LogicalNot:
        {
            if (peek.type == Token_Equals)
            {
                result.type = Token_NotEqual;
            }
        } break;
        
        case Token_GreaterThan:
        {
            if (peek.type == Token_Equals)
            {
                result.type = Token_GreaterThanOrEqual;
            }
            
            else if (peek.type == Token_GreaterThan)
            {
                result.type = Token_RightShift;
            }
        } break;
        
        case Token_LessThan:
        {
            if (peek.type == Token_Equals)
            {
                result.type = Token_LessThanOrEqual;
            }
            
            else if (peek.type == Token_LessThan)
            {
                result.type = Token_LeftShift;
            }
        } break;
        
        case Token_Ampersand:
        {
            if (peek.type == Token_Ampersand)
            {
                result.type = Token_LogicalAnd;
            }
            
            else if (peek.type == Token_Equals)
            {
                result.type = Token_AndEquals;
            }
        } break;
        
        case Token_Or:
        {
            if (peek.type == Token_Or)
            {
                result.type = Token_LogicalOr;
            }
            
            else if (peek.type == Token_Equals)
            {
                result.type = Token_OrEquals;
            }
        } break;
        
        case Token_Not:
        {
            if (peek.type == Token_Equals)
            {
                result.type = Token_NotEquals;
            }
        } break;
        
        case Token_XOR:
        {
            if (peek.type == Token_Equals)
            {
                result.type = Token_XOREquals;
            }
        } break;
    }
    
    if (result.type != old_result.type)
    {
        GetTokenRaw(lexer, true);
        
        if (result.type == Token_RightShift &&
            PeekTokenRaw(lexer, true).type == Token_Equals)
        {
            result.type = Token_RightShiftEquals;
        }
        
        else if (result.type == Token_LeftShift &&
                 PeekTokenRaw(lexer, true).type == Token_Equals)
        {
            result.type = Token_LeftShiftEquals;
        }
    }
    
    else if (result.type == Token_Dot && peek.type == Token_Dot)
    {
        GetTokenRaw(lexer, true);
        
        if (PeekTokenRaw(lexer, true).type == Token_Dot)
        {
            GetTokenRaw(lexer, true);
            result.type = Token_Elipsis;
        }
        
        else
        {
            // TODO(soimn): How to handle incomplete elipsis tokens?
            //// ERROR: ..
            result.type = Token_Error;
        }
    }
    
    return result;
}

inline Token
PeekToken(Lexer* lexer)
{
    Lexer temp_lexer = *lexer;
    
    return GetToken(&temp_lexer);
}

inline bool
RequireToken(Lexer* lexer, Enum32(LEXER_TOKEN_TYPE) type, bool allways_remove = false)
{
    bool result = false;
    
    result = (PeekToken(lexer).type == type);
    
    if (result || allways_remove)
    {
        GetToken(lexer);
    }
    
    return result;
}