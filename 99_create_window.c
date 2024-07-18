#pragma once

#include "01_commons.h"

static
LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

static
HWND create_window(HINSTANCE instance)
{
    ASSERT(instance);

    const WNDCLASSA winclass = {
        .style         = CS_VREDRAW|CS_HREDRAW|CS_OWNDC,
        .lpfnWndProc   = &window_procedure,
        .hInstance     = instance,
        .lpszClassName = APPNAME"_winclass",
    };

    if (!RegisterClassA(&winclass))
    {
        return NULL;
    }

    return CreateWindowExA(0x0,
                           winclass.lpszClassName,
                           APPNAME,
                           WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           NULL,
                           NULL,
                           instance,
                           NULL);
}
