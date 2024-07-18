#pragma once

//==============================================================================
// 
// Debug related functions
// 
//==============================================================================

#include "01_commons.h"


#if DEBUG
#   include <stdarg.h>

static char dbg_print_buf[BUFFER_SIZE_DBGPRINT];

static
void dbg_print(literal_t fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    {
        VA_FMT(dbg_print_buf, fmt, va);

        OutputDebugStringA(dbg_print_buf);
    }
    va_end(va);
}
#endif // DEBUG

