#pragma once

#include "common.h"
#include "memory.h"

// TODO(soimn): Remove this dependency
#include <stdarg.h>

inline bool
IsAlpha(char c)
{
    return ((c >= 'A' && c <= 'Z') && (c >= 'a' && c <= 'z'));
}

inline bool
IsNumeric(char c)
{
    return (c >= '0' && c <= '9');
}

inline bool
IsWhitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\v');
}

inline bool
IsEndOfLine(char c)
{
    return (c == '\n');
}

inline bool
IsSpacing(char c)
{
    return (IsWhitespace(c) || IsEndOfLine(c));
}


inline char
ToLower(char c)
{
    char result = c;
    
    if (result >= 'A' && result <= 'Z')
    {
        result -= 'A';
        result += 'a';
    }
    
    return result;
}

inline char
ToUpper(char c)
{
    char result = c;
    
    if (result >= 'a' && result <= 'z')
    {
        result -= 'a';
        result += 'A';
    }
    
    return result;
}

/// 
/// 
/// 

inline void
Advance(String* string, UMM amount)
{
    if (string->size >= amount)
    {
        string->data += amount;
        string->size -= amount;
    }
    
    else
    {
        string->data = 0;
        string->size = 0;
    }
}

inline UMM
StringLength(const char* cstring)
{
    UMM length = 0;
    for (char* scan = (char*)cstring; *scan; ++scan, ++length);
    
    return length;
}

inline bool
StringCompare(String s0, String s1)
{
    while (s0.size && s1.size && s0.data[0] == s1.data[0])
    {
        Advance(&s0, 1);
        Advance(&s1, 1);
    }
    
    return (s0.size == 0 && s0.size == s1.size);
}

/// 
/// 
/// 

struct String_Stream
{
    Bucket_Array bucket_array;
};

inline void
Append(String_Stream* stream, char c)
{
    if (stream)
    {
        *(char*)PushElement(&stream->bucket_array) = c;
    }
}

inline void
Append(String_Stream* stream, String string)
{
    if (stream)
    {
        for (; string.size; Advance(&string, 1))
        {
            *(char*)PushElement(&stream->bucket_array) = (char)*string.data;
        }
    }
}

struct String_Stream_Interval
{
    Bucket_Array_Block* first_block;
    UMM block_size;
    UMM index;
    UMM size;
};

inline bool
StringCompare(String_Stream_Interval interval, String string)
{
    Bucket_Array_Block* current_block = interval.first_block;
    U8* current = (U8*)(current + 1) + interval.index % interval.block_size;
    
    while (interval.size && string.size && *current == string.data[0])
    {
        Advance(&string, 1);
        
        ++interval.index;
        --interval.size;
        
        if (interval.index % interval.block_size == 0)
        {
            current_block = current_block->next;
        }
        
        current = (U8*)(current + 1) + interval.index % interval.block_size;
    }
    
    return (interval.size == 0 && interval.size == string.size);
}

inline bool
StringCompare(String string, String_Stream_Interval interval)
{
    return StringCompare(interval, string);
}

inline void
Append(String_Stream* stream, String_Stream_Interval interval)
{
    Bucket_Array_Block* block = interval.first_block;
    
    for (UMM i = interval.index % interval.block_size; i < MIN(interval.size, interval.block_size); ++i)
    {
        Append(stream, *((char*)(block + 1) + i));
    }
    
    UMM remaining_size = interval.size - MIN(interval.size, interval.block_size);
    
    if (remaining_size)
    {
        block = block->next;
        
        U32 num_blocks = (U32)(remaining_size / interval.block_size) + 1;
        for (U32 i = 0; i < num_blocks; ++i)
        {
            for (U32 j = 0; j < MIN(remaining_size, interval.block_size); ++j)
            {
                Append(stream, *((char*)(block + 1) + j));
            }
            
            remaining_size -= interval.block_size;
            block = block->next;
        }
    }
}

/// 
/// 
/// 

inline UMM
Print(String_Stream* stream, String message, va_list arg_list)
{
    UMM required_length = 0;
    
    while(message.size)
    {
        if (*message.data == '%')
        {
            Advance(&message, 1);
            
            if (message.size)
            {
                char c = (char)*message.data;
                Advance(&message, 1);
                
                switch (c)
                {
                    case '%':
                    {
                        Append(stream, '%');
                        ++required_length;
                    } break;
                    
                    case 'u':
                    case 'i':
                    case 'U':
                    case 'I':
                    {
                        U64 unsigned_number = 0;
                        I64 signed_number   = 0;
                        
                        bool is_signed = false;
                        
                        if (c == 'u')
                        {
                            unsigned_number = va_arg(arg_list, U32);
                        }
                        
                        else if (c == 'i')
                        {
                            signed_number = va_arg(arg_list, I32);
                            is_signed = true;
                        }
                        
                        else if (c == 'U')
                        {
                            unsigned_number = va_arg(arg_list, U64);
                        }
                        
                        else
                        {
                            signed_number = va_arg(arg_list, I64);
                            is_signed = true;
                        }
                        
                        U32 index = 0;
                        char buffer[20] = {};
                        
                        if (!unsigned_number && !signed_number)
                        {
                            index = 1;
                            buffer[0] = '0';
                        }
                        
                        else if (is_signed)
                        {
                            if (signed_number < 0)
                            {
                                Append(stream, '-');
                                signed_number *= -1;
                                
                                ++required_length;
                            }
                            
                            for (;;)
                            {
                                buffer[index] = signed_number % 10 + '0';
                                if (signed_number == 0) break;
                                
                                signed_number  /= 10;
                                
                                ++index;
                                ++required_length;
                            }
                        }
                        
                        else
                        {
                            for (;;)
                            {
                                buffer[index] = (char)(unsigned_number % 10) + '0';
                                if (unsigned_number == 0) break;
                                
                                unsigned_number /= 10;
                                
                                ++index;
                                ++required_length;
                                
                            }
                        }
                        
                        for (U32 i = index - 1; ; --i)
                        {
                            Append(stream, buffer[i]);
                            if (i == 0) break;
                        }
                    } break;
                    
                    case 'S':
                    {
                        String string = va_arg(arg_list, String);
                        Append(stream, string);
                        required_length += string.size;
                    } break;
                    
                    case 's':
                    {
                        const char* cstring = va_arg(arg_list, const char*);
                        
                        for (char* scan = (char*)cstring; *scan; ++scan)
                        {
                            Append(stream, *scan);
                            ++required_length;
                        }
                    } break;
                    
                    case 'b':
                    {
                        bool value = va_arg(arg_list, bool);
                        String true_string  = CONST_STRING("true");
                        String false_string = CONST_STRING("false");
                        
                        if (value)
                        {
                            Append(stream, true_string);
                            required_length += true_string.size;
                        }
                        
                        else
                        {
                            Append(stream, false_string);
                            required_length += false_string.size;
                        }
                    } break;
                    
                    default:
                    {
                        Append(stream, '%');
                        Append(stream, c);
                        required_length += 2;
                    } break;
                }
            }
            
            else
            {
                Append(stream, '%');
                ++required_length;
            }
        }
        
        else
        {
            Append(stream, (char)*message.data);
            Advance(&message, 1);
            ++required_length;
        }
    }
    
    return required_length;
}

inline UMM
Print(String_Stream* stream, String message, ...)
{
    UMM resulting_length = 0;
    
    va_list arg_list;
    va_start(arg_list, message);
    Print(stream, message, arg_list);
    va_end(arg_list);
    
    return resulting_length;
}

inline UMM
Print(String_Stream* stream, const char* message, ...)
{
    UMM resulting_length = 0;
    
    va_list arg_list;
    va_start(arg_list, message);
    Print(stream, {(U8*)message, StringLength(message)}, arg_list);
    va_end(arg_list);
    
    return resulting_length;
}