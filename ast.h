#pragma once

#include "common.h"

struct AST_Node
{
    File_ID file;
    U32 line;
    U32 column;
    
    // ...
};