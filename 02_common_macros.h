#pragma once

//==============================================================================
// 
// You guess it it's common macros!
// 
//==============================================================================

#include <stdio.h>

//
// FMT - snprintf shortcut
//

#define FMT(__BUF__, __FMT__, ...)            snprintf((__BUF__), sizeof((__BUF__)), __FMT__, ##__VA_ARGS__)
#define VA_FMT(__BUF__, __FMT__, __VA_LIST__) vsnprintf((__BUF__), sizeof((__BUF__)), __FMT__, __VA_LIST__)

//
// Debug print
//

#if DEBUG
#   define dbgprint(__FMT__, ...) dbg_print("[DEBUG] (%s:%d) "__FMT__"\n", __FILE__,__LINE__, ##__VA_ARGS__)
#else
#   define dbgprint(__FMT__, ...)
#endif

//
// Macro that turns any text into a string literal
//

#define STRINGIFY(__ANYTEXT__) #__ANYTEXT__

//
// Assertions
//

#if ASSERTIVE
#   define ASSERT_FN(__EQ__, __FN__) ASSERT(__EQ__ (__FN__))
#   define ASSERT(__EXPR__)\
    do\
    {\
        if (!(__EXPR__))\
        {\
            assert_callback(__FILE__,__LINE__, STRINGIFY(__EXPR__));\
        }\
    }\
    while(0)
#else
#   define ASSERT_FN(__EQ__, __FN__) __FN__
#   define ASSERT(__EXPR__)
#endif

static
void assert_callback(literal_t file, const s32 line, literal_t expr);

//
// Macro for regions of code that should be marked as required to be fixed.
//

#define FIXME(__FIXNAME__) (__FIXNAME__)

//
// Vulkan VkResult to string converter (similar to win32_errstr)
//

#define vk_errstr(__VKRESULT__) string_VkResult((__VKRESULT__))


//
// sizeof macro for arrays
//

#define sizeof_array(__ARR__) (sizeof((__ARR__))/sizeof((__ARR__[0])))

//
// static keyword disambiguation
//

#define GLOBAL static


//
// Break Instruction
//

#define BREAK DebugBreak


//
// Vulkan related asserts
//

#if ASSERTIVE
#   define VK_ASSERT_FN(__EQ__, __FN__)                                                             \
               do                                                                                   \
               {                                                                                    \
                   VkResult result = __FN__;                                                        \
                   if (!(__EQ__ result))                                                               \
                   {                                                                                \
                       FMT(g_assert_buf_, "%s: %s", STRINGIFY(__EQ__ (__FN__)), vk_errstr(result)); \
                       assert_callback(__FILE__,__LINE__, g_assert_buf_);                           \
                   }                                                                                \
               }                                                                                    \
               while(0)
#else
#   define VK_ASSERT_FN(__EQ__, __FN__) __FN__
#endif

