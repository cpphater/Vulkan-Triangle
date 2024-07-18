#pragma once

//==============================================================================
// 
// You guess it it's basic types!
// 
//==============================================================================

#include <stdint.h>

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8 ;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8 ;

//
// Sometimes you need to work with simple bytes:
//

typedef u8 byte;

//
//  For struct padding:
//

typedef byte slop_t;

//
// For String Literals (like printf format string)
//

typedef const char*const literal_t;

