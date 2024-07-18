#pragma once

//==============================================================================
// 
// Common stuff (every module knows these)
// 
//==============================================================================

//
// Defines that dictate the compilation
//

#include "02_compilation.h"

//
// Common stuff
//

#include "02_basic_types.h"
#include "02_common_macros.h"
#include "02_common_constants.h"

//
// Third party includes
//

#pragma warning(push, 3)
#   include <Windows.h>
#   include <vulkan/vulkan.h>
#   include <vulkan/vk_enum_string_helper.h>
#pragma warning(pop)

