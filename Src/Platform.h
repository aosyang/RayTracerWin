//=============================================================================
// Platform.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#if __APPLE__
#define PLATFORM_OSX 1
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#elif _WIN32
#define PLATFORM_WIN32 1
#include <windows.h>
#else
#define PLATFORM_LINUX 1
#endif

#ifndef PLATFORM_OSX
#define PLATFORM_OSX 0
#endif

#ifndef PLATFORM_WIN32
#define PLATFORM_WIN32 0
#endif

#ifndef PLATFORM_LINUX
#define PLATFORM_LINUX 0
#endif

#ifndef UINT32
typedef unsigned int UINT32;
static_assert(sizeof(UINT32) == 4, "UINT32 must be 4 bytes");
#endif	// ifndef UINT32


#define LogBufSize 16384


// Platform specific definitions
#if (PLATFORM_WIN32)

	// Print to char buffer
	#define RPrintf(Buffer, Size, ...)		sprintf_s(Buffer, Size, __VA_ARGS__)

	// Print to output window
	#define RLog(...)						{ char LogMsg[LogBufSize]; sprintf_s(LogMsg, __VA_ARGS__); OutputDebugStringA(LogMsg); }

FORCEINLINE void DebugBreak()
{
	__debugbreak();
}

#else

	// Print to char buffer
	#define RPrintf(Buffer, Size, ...)		snprintf(Buffer, Size, __VA_ARGS__)

	// Print to output window
	#define RLog(...)						{ printf(__VA_ARGS__); }

    #define FORCEINLINE                     inline __attribute__ ((always_inline))

#include <csignal>

FORCEINLINE void DebugBreak()
{
	std::raise(SIGINT);
}

#endif
