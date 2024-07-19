#pragma once

//==============================================================================
// 
// All the common constants are here
// 
//==============================================================================


#pragma warning(push, 3)
#   include <Windows.h>
#   include <vulkan/vulkan.h>
#   include <vulkan/vk_enum_string_helper.h>
#pragma warning(pop)

//
// Max counts duh
//

enum max_counts {
    VK_MAX_COUNT_EXT              = 32,
    VK_MAX_COUNT_LAYERS           = 32,
    VK_MAX_COUNT_PHYSICAL_DEVICES = 8,
    VK_MAX_COUNT_QUEUE_FAMILY     = 16,
    VK_MAX_COUNT_PDEV_EXT         = 256,
    VK_MAX_COUNT_SURFACE_FORMATS  = 16,
    VK_MAX_COUNT_PRESENT_MODES    = 16,
    VK_MAX_COUNT_EXTENT_2D        = 16,
};

//
// Buffer sizes
//

enum buffer_sizes {
    BUFFER_SIZE_DBGPRINT            = 1024 * 4,
    BUFFER_SIZE_ASSERT              = 1024 * 4,
    BUFFER_SIZE_WIN32_ERRSTR        = 1024 * 4,
    BUFFER_SIZE_VK_EXT_NAMES        = (VK_MAX_EXTENSION_NAME_SIZE * VK_MAX_COUNT_EXT),
    BUFFER_SIZE_VK_PDEV_EXT_NAMES   = (VK_MAX_EXTENSION_NAME_SIZE * VK_MAX_COUNT_PDEV_EXT),
    BUFFER_SIZE_VK_LAYER_NAMES      = (VK_MAX_EXTENSION_NAME_SIZE * VK_MAX_COUNT_LAYERS),
};




#define APPNAME "vk_triangle"

