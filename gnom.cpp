#include "common.h"
#include "error_handling.h"
#include "memory.h"
#include "lexer.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef near
#undef far

global Memory_Arena  OutputStreamArena = {};
global String_Stream ErrorStreamObject = {};
global String_Stream PrintStreamObject = {};

inline Memory_Block*
AllocateMemoryBlock(UMM block_size)
{
    Memory_Block* new_block = 0;
    
    UMM total_size = (alignof(Memory_Block) - 1) + sizeof(Memory_Block) + block_size;
    
    void* memory = VirtualAlloc(0, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    
    Assert(memory, "Failed to allocate memory block");
    
    new_block = (Memory_Block*) Align(memory, alignof(Memory_Block));
    
    *new_block = {};
    new_block->push_ptr = Align(new_block + 1, 8);
    new_block->space    = total_size - (new_block->push_ptr - (U8*) new_block);
    
    return new_block;
}

inline void
FreeMemoryBlock(Memory_Block* block)
{
    VirtualFree((void*) block, block->space + (block->push_ptr - (U8*) block), MEM_RELEASE | MEM_DECOMMIT);
}

inline void
Flush(String_Stream* stream)
{
    // NOTE(soimn): This acquires a handle to stderr which is not 
    //              buffered
    HANDLE output_handle = GetStdHandle((U32)-12);
    
    Bucket_Array* bucket_array = &stream->bucket_array;
    for (Bucket_Array_Block* scan = bucket_array->first_block; scan; scan = scan->next)
    {
        void* start = scan + 1;
        U32 size = (scan == bucket_array->current_block ? bucket_array->current_block->offset : bucket_array->block_size);
        
        if (size)
        {
            DWORD chars_written;
            WriteConsole(output_handle, start, size, &chars_written, 0);
        }
    }
    
    ResetArray(&stream->bucket_array);
}

int
main(int argc, const char** argv)
{
    ErrorStreamObject.bucket_array = BUCKET_ARRAY(&OutputStreamArena, char, 512);
    ErrorStream = &ErrorStreamObject;
    
    PrintStreamObject.bucket_array = BUCKET_ARRAY(&OutputStreamArena, char, 512);
    PrintStream = &PrintStreamObject;
    
    Print(ErrorStream, "Hello %s!", "World");
    Flush(ErrorStream);
    
    return 0;
}