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

#include <windows.h>

#endif


#define LogBufSize 16384


// Platform specific definitions
#if (PLATFORM_WIN32)

	// Print to char buffer
	#define RPrintf(Buffer, Size, ...)		sprintf_s(Buffer, Size, __VA_ARGS__)

	// Print to output window
	#define RLog(...)						{ char LogMsg[LogBufSize]; sprintf_s(LogMsg, __VA_ARGS__); OutputDebugStringA(LogMsg); }

#else

	// Print to char buffer
	#define RPrintf(Buffer, Size, ...)		snprintf(Buffer, Size, __VA_ARGS__)

	// Print to output window
	#define RLog(...)						{ printf(__VA_ARGS__); }

#endif
