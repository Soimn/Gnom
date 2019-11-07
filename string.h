#pragma once

#include "common.h"
#include "memory.h"

// TODO(soimn): Remove this dependency
#include <stdarg.h>

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
    
    return s0.size == 0 && s0.size == s1.size;
}

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
            *(char*)PushElement(&stream->bucket_array) = *string.data;
        }
    }
}

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
                char c = *message.data;
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
                                buffer[index]  = unsigned_number % 10 + '0';
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