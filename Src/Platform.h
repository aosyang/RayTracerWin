//=============================================================================
// Platform.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#if __APPLE__

#define PLATFORM_OSX 1
typedef unsigned int UINT32;
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

#elif _WIN32

#define PLATFORM_WIN32 1

#endif
