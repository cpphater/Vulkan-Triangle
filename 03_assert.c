#pragma once

//==============================================================================
// 
// Assertion functions
// 
//==============================================================================

#include "01_commons.h"

static
void assert_callback(literal_t file, s32 line, literal_t expr)
{
    static char buf[BUFFER_SIZE_ASSERT];
    {
        FMT(buf, "[ASSERT] (%s:%d) %s\n", file,line, expr);
    }

    int button_pressed = MessageBoxA(NULL, buf, "ASSERTION", MB_OKCANCEL);
    switch (button_pressed)
    {
        case IDCANCEL:
        {
            ExitProcess(EXIT_FAILURE);
        }
        break;

        case IDOK:
        {
            BREAK();
        }
        break;
    }
}

