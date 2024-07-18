#pragma once

//==============================================================================
// 
// win32_errstr function that returns read-only string with error message 
// retrived from error_code (use GetLastError())
// 
//==============================================================================

#include "01_commons.h"

static
literal_t win32_errstr(DWORD error_code)
{
    static char buf[BUFFER_SIZE_WIN32_ERRSTR];

    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf,
        sizeof(buf),
        NULL
    );

    return buf;
}

