#pragma once

#include "common.h"

// TODO(soimn): Add tags to memory arenas to allow for tracking and 
//              more detailed error messages

struct Memory_Block
{
	Memory_Block* prev;
	Memory_Block* next;
	U8* push_ptr;
	U64 space;
};

struct Memory_Arena
{
	Memory_Block* current_block;
	UMM block_size;
	U16 block_count;
};

inline Memory_Block*
AllocateMemoryBlock(UMM block_size);

inline void
FreeMemoryBlock(Memory_Block* block);

inline U8*
Align(void* ptr, U8 alignment)
{
	return ((U8*) ptr + (U8)(((~((UMM) ptr)) + 1) & (U8)(alignment - 1)));
}

inline U8
AlignOffset(void* ptr, U8 alignment)
{
	return (U8)((U8*) Align(ptr, alignment) - (U8*) ptr);
}

inline UMM
RoundSize(UMM size, U8 alignment)
{
    return size + AlignOffset((U8*)size, alignment);
}

inline void
Copy(void* source, void* dest, UMM size)
{
    Assert(size != 0);
    
    U8* bsource = (U8*) source;
    U8* bdest   = (U8*) dest;
    
    while (bsource < (U8*) source + size)
    {
        *(bdest++) = *(bsource++);
    }
}

#define CopyStruct(source, dest) Copy((void*) (source), (void*) (dest), sizeof(*(source)))
#define CopyArray(source, dest, count) Copy((void*) (source), (void*) (dest), sizeof(*(source)) * (count))

inline void
ZeroSize(void* ptr, UMM size)
{
	U8* bptr = (U8*) ptr;
    
	while (bptr < (U8*) ptr + size) *(bptr++) = 0;
}

#define ZeroStruct(type) (*(type) = {})
#define ZeroArray(type, count) ZeroSize(type, sizeof((type)[0]) * (count))

/// /////////////////////////////////////////////
/// /////////////////////////////////////////////
/// /////////////////////////////////////////////

inline void
ClearArena(Memory_Arena* arena)
{
    Memory_Block* block = arena->current_block;
    
    while (block && block->next)
    {
        block = block->next;
    }
    
    while (block)
    {
        Memory_Block* temp_ptr = block->prev;
        
        FreeMemoryBlock(block);
        
        block = temp_ptr;
    }
    
    arena->current_block = 0;
    arena->block_count   = 0;
}

inline void
ResetArena(Memory_Arena* arena)
{
    Memory_Block* block = arena->current_block;
    
    while (block && block->next)
    {
        block = block->next;
    }
    
    while (block)
    {
        U8* new_push_ptr = Align(block + 1, 8);
        block->space    += block->push_ptr - new_push_ptr;
        block->push_ptr  = new_push_ptr;
        
        block = block->prev;
    }
}

inline void*
PushSize(Memory_Arena* arena, UMM size, U8 alignment = 1)
{
    void* result = 0;
    
    Assert(size != 0);
    Assert(alignment == 1 || alignment == 2 || alignment == 4 || alignment == 8);
    
    UMM total_size = (arena->current_block ? size + AlignOffset(arena->current_block->push_ptr, alignment) : 0);
    
    if (!arena->current_block || arena->current_block->space < total_size)
    {
        if (arena->current_block && arena->current_block->next)
        {
            if (arena->current_block->next->space >= size)
            {
                arena->current_block = arena->current_block->next;
            }
            
            else
            {
                Memory_Block* new_block = AllocateMemoryBlock(size + alignment - 1);
                
                Memory_Block* next = arena->current_block->next;
                
                arena->current_block->next = new_block;
                new_block->prev            = arena->current_block;
                
                new_block->next = next;
                next->prev = new_block;
                
                arena->current_block = new_block;
                ++arena->block_count;
            }
        }
        
        else
        {
            UMM block_size = MAX(size + alignment - 1, arena->block_size);
            Memory_Block* new_block = AllocateMemoryBlock(block_size);
            
            if (arena->current_block)
            {
                arena->current_block->next = new_block;
                new_block->prev            = arena->current_block;
            }
            
            arena->current_block = new_block;
            ++arena->block_count;
        }
        
        total_size = size + AlignOffset(arena->current_block->push_ptr, alignment);
    }
    
    
    result = Align(arena->current_block->push_ptr, alignment);
    
    arena->current_block->push_ptr += total_size;
    arena->current_block->space    -= total_size;
    
    return result;
}

#define PushStruct(arena, type) (type*) PushSize(arena, sizeof(type), alignof(type))
#define PushArray(arena, type, count) (type*) PushSize(arena, (count) * RoundSize(sizeof(type), alignof(type)), alignof(type))

/// /////////////////////////////////////////////
/// /////////////////////////////////////////////
/// /////////////////////////////////////////////

struct Bucket_Array_Block
{
    Bucket_Array_Block* next;
    Bucket_Array_Block* prev;
    U32 offset;
    U32 space;
};

struct Bucket_Array
{
    Memory_Arena* arena;
    
    Bucket_Array_Block* first_block;
    Bucket_Array_Block* current_block;
    
    U32 num_elements;
    U32 element_size;
    U32 block_size;
    U32 block_count;
};

inline Bucket_Array
BucketArray(Memory_Arena* arena, UMM element_size, U32 block_size)
{
    Assert(element_size <= U32_MAX);
    
    Bucket_Array result = {};
    result.arena        = arena;
    result.element_size = (U32)element_size;
    result.block_size   = block_size;
    
    return result;
}

#define BUCKET_ARRAY(arena, type, block_size) BucketArray(arena, RoundSize(sizeof(type), alignof(type)), block_size)

inline void*
ElementAt(Bucket_Array* array, UMM index)
{
    void* result = 0;
    
    UMM array_size = 0;
    
    if (array->block_count)
    {
        array_size = (array->block_count - 1) * array->block_size + array->current_block->offset;
    }
    
    if (index < array_size)
    {
        Bucket_Array_Block* scan = 0;
        
        UMM block_index  = index / array->block_size;
        U32 block_offset = index % array->block_size;
        
        if (block_index == array->block_count - 1)
        {
            scan = array->current_block;
        }
        
        else if (block_index <= array->block_count / 2)
        {
            scan = array->first_block;
            
            for (U32 i = 0; i < block_index; ++i)
            {
                scan = scan->next;
            }
        }
        
        else
        {
            scan = array->current_block;
            
            for (U32 i = 0; i < array->block_count - (block_index + 1), scan; ++i)
            {
                scan = scan->prev;
            }
        }
        
        if (scan)
        {
            result = (U8*)(scan + 1) + array->element_size * block_offset;
        }
    }
    
    return result;
}

inline void*
PushElement(Bucket_Array* array)
{
    void* result = 0;
    
    if (!array->first_block || !array->current_block->space)
    {
        Assert(array->block_count < U32_MAX);
        
        UMM block_size = sizeof(Bucket_Array_Block) + array->element_size * array->block_size;
        Bucket_Array_Block* new_block = (Bucket_Array_Block*)PushSize(array->arena, block_size, alignof(Bucket_Array_Block));
        *new_block = {};
        
        if (array->first_block)
        {
            array->current_block->next = new_block;
            new_block->prev = array->current_block;
        }
        
        else
        {
            array->first_block = new_block;
        }
        
        array->current_block = new_block;
        ++array->block_count;
    }
    
    result = (U8*)(array->current_block + 1) + array->element_size * array->current_block->offset;
    ++array->current_block->offset;
    --array->current_block->space;
    ++array->num_elements;
    
    return result;
}

struct Bucket_Array_Iterator
{
    Bucket_Array_Block* current_block;
    UMM current_index;
    void* current;
    U32 element_size;
    U32 block_size;
    U32 num_elements;
};

inline Bucket_Array_Iterator
Iterate(Bucket_Array* array)
{
    Bucket_Array_Iterator iterator = {};
    
    if (array->block_count)
    {
        iterator.current_block = array->first_block;
        iterator.current_index = 0;
        iterator.current = array->first_block + 1;
        
        iterator.element_size = array->element_size;
        iterator.block_size   = array->block_size;
        iterator.num_elements = array->num_elements;
    }
    
    return iterator;
}

inline void
Advance(Bucket_Array_Iterator* iterator)
{
    iterator->current = 0;
    
    ++iterator->current_index;
    
    U32 offset = iterator->current_index % iterator->block_size;
    
    if (offset == 0)
    {
        iterator->current_block = iterator->current_block->next;
    }
    
    if (iterator->current_block)
    {
        iterator->current = (U8*)(iterator->current_block + 1) + iterator->element_size * offset;
    }
}

struct Free_List_Bucket_Array
{
    void** free_list;
    
    Memory_Arena* arena;
    
    Bucket_Array_Block* first_block;
    Bucket_Array_Block* current_block;
    
    U32 pad_;
    U32 element_size;
    U32 block_size;
    U32 block_count;
};

inline Free_List_Bucket_Array
FreeListBucketArray(Memory_Arena* arena, UMM element_size, U32 block_size)
{
    Assert(element_size <= U32_MAX);
    Assert(element_size >= sizeof(void*));
    
    Free_List_Bucket_Array result = {};
    result.arena        = arena;
    result.element_size = (U32)element_size;
    result.block_size   = block_size;
    
    return result;
}

#define FREE_LIST_BUCKET_ARRAY(arena, type, block_size) FreeListBucketArray(arena, RoundSize(sizeof(type), alignof(type)), block_size)

// NOTE(soimn): This does not check if the element is freed or not
inline void*
ElementAt(Free_List_Bucket_Array* array, IMM index)
{
    return ElementAt((Bucket_Array*)&array->arena, index);
}

inline void*
PushElement(Free_List_Bucket_Array* array)
{
    void* result = 0;
    
    if (array->free_list)
    {
        result = array->free_list;
        array->free_list = (void**)*array->free_list;
    }
    
    else
    {
        if (!array->first_block || !array->current_block->space)
        {
            Assert(array->block_count < U32_MAX);
            
            UMM block_size = sizeof(Bucket_Array_Block) + array->element_size * array->block_size;
            Bucket_Array_Block* new_block = (Bucket_Array_Block*)PushSize(array->arena, block_size, alignof(Bucket_Array_Block));
            *new_block = {};
            
            if (array->first_block)
            {
                array->current_block->next = new_block;
                new_block->prev = array->current_block;
            }
            
            else
            {
                array->first_block = new_block;
            }
            
            array->current_block = new_block;
            ++array->block_count;
        }
        
        result = (U8*)(array->current_block + 1) + array->element_size * array->current_block->offset;
        ++array->current_block->offset;
        --array->current_block->space;
    }
    
    return result;
}

inline void
RemoveElement(Free_List_Bucket_Array* array, void* element)
{
    bool is_valid = false;
    
    if (element)
    {
        U8* element_u8 = (U8*)element;
        Bucket_Array_Block* scan = array->first_block;
        
        while (scan)
        {
            U8* block_start = (U8*)(scan + 1);
            UMM offset = element_u8 - block_start;
            if (block_start <= element_u8 && element_u8 < block_start + array->block_size)
            {
                if (offset % array->element_size == 0 && (scan != array->current_block || offset < array->current_block->offset))
                {
                    is_valid = true;
                }
                
                break;
            }
            
            scan = scan->next;
        }
    }
    
    Assert(is_valid == true);
    
    if (is_valid)
    {
        *(void**)element = array->free_list;
        array->free_list = (void**)element;
    }
}