//=============================================================================
// ColorBuffer.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Platform.h"
#include "RVector.h"
#include <math.h>

template <typename T>
inline const T& Min(const T& a, const T& b) { return a < b ? a : b; }

template <typename T>
inline const T& Max(const T& a, const T& b) { return a > b ? a : b; }

typedef UINT32 Pixel;

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


inline UINT32 MakeUint32Color(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a)
{
#if (PLATFORM_OSX)
	return (_r << 24 | _g << 16 | _b << 8 | _a);
#else
	return (_a << 24 | _r << 16 | _g << 8 | _b);
#endif
}

inline UINT32 MakePixelColor(RVec3 color)
{
	int r = int(Min(Max(color.x, 0.0f), 1.0f) * 255);
	int g = int(Min(Max(color.y, 0.0f), 1.0f) * 255);
	int b = int(Min(Max(color.z, 0.0f), 1.0f) * 255);
	return MakeUint32Color(r, g, b, 255);
}
