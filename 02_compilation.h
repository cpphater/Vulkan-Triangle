#pragma once

//==============================================================================
// 
// All the directives that dictate how compilation works
// 
//==============================================================================

#define VK_USE_PLATFORM_WIN32_KHR 1

#define DEBUG 1
#define ASSERTIVE DEBUG
#define VK_USING_VALIDATION DEBUG

//
// FIXME names
//

#define DISABLE_TRANSLATE_MESSAGE 1
#define DO_IF_REQUIRED_EXT_NOT_FOUND 1
#define DO_IF_REQUIRED_LAYER_NOT_FOUND 1
#define DO_I_HAVE_TO_DESTROY_VK_DBG_MESSENGER 1
#define MULTIPLE_QUEUE_FAMILIES_PER_DEVICE 1
#define NO_NEED_TO_SEPARATE_BETWEEN_VULKAN_DEVICE_AND_VULKAN_INSTANCE 1
#define PICKING_PHYSICAL_DEVICE_SUCKS_ASS 1
#define NOT_SURE_ABOUT_MAX_QUEUES_COUNT 1 

