//=============================================================================
// ColorBuffer.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Platform.h"
#include "RVector.h"
#include <math.h>

typedef UINT32 Pixel;
typedef unsigned char BYTE;

const int bitmapWidth = 800;
const int bitmapHeight = 800;

inline void BufferIndexToCoord(int idx, int& x, int& y)
{
	x = idx % bitmapWidth;
	y = idx / bitmapWidth;
}

inline int CoordToBufferIndex(int x, int y)
{
	if (x < 0 || x >= bitmapWidth || y < 0 || y >= bitmapHeight)
		return -1;

	return y * bitmapWidth + x;
}

inline UINT32 MakeUint32Color(BYTE _r, BYTE _g, BYTE _b, BYTE _a)
{
#if (PLATFORM_OSX)
	return (_r << 24 | _g << 16 | _b << 8 | _a);
#else
	return (_a << 24 | _r << 16 | _g << 8 | _b);
#endif
}

inline BYTE GetUint32ColorRed(UINT32 Color)
{
#if (PLATFORM_OSX)
	return (BYTE)((Color >> 24) & 0xFF);
#else
	return (BYTE)((Color >> 16) & 0xFF);
#endif
}

inline BYTE GetUint32ColorGreen(UINT32 Color)
{
#if (PLATFORM_OSX)
	return (BYTE)((Color >> 16) & 0xFF);
#else
	return (BYTE)((Color >> 8) & 0xFF);
#endif
}

inline BYTE GetUint32ColorBlue(UINT32 Color)
{
#if (PLATFORM_OSX)
	return (BYTE)((Color >> 8) & 0xFF);
#else
	return (BYTE)(Color & 0xFF);
#endif
}


inline UINT32 MakePixelColor(RVec3 color)
{
    int r = int(Math::Min(Math::Max(color.x, 0.0f), 1.0f) * 255);
	int g = int(Math::Min(Math::Max(color.y, 0.0f), 1.0f) * 255);
	int b = int(Math::Min(Math::Max(color.z, 0.0f), 1.0f) * 255);
	return MakeUint32Color(r, g, b, 255);
}
