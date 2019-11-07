#include "common.h"
#include "error_handling.h"
#include "memory.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef near
#undef far

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

int
main(int argc, const char** argv)
{
    
}