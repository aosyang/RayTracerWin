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

// Convert buffer array index to 2d coordinate
FORCEINLINE void BufferIndexToCoord(int idx, int& x, int& y)
{
	x = idx % bitmapWidth;
	y = idx / bitmapWidth;
}

// Convert 2d coordinate to buffer array index
FORCEINLINE int CoordToBufferIndex(int x, int y)
{
	if (x < 0 || x >= bitmapWidth || y < 0 || y >= bitmapHeight)
		return -1;

	return y * bitmapWidth + x;
}

FORCEINLINE UINT32 MakeUint32Color(BYTE _r, BYTE _g, BYTE _b, BYTE _a)
{
#if (PLATFORM_OSX)
	return (_r << 24 | _g << 16 | _b << 8 | _a);
#else
	return (_a << 24 | _r << 16 | _g << 8 | _b);
#endif
}

FORCEINLINE BYTE GetUint32ColorRed(UINT32 Color)
{
#if (PLATFORM_OSX)
	return (BYTE)((Color >> 24) & 0xFF);
#else
	return (BYTE)((Color >> 16) & 0xFF);
#endif
}

FORCEINLINE BYTE GetUint32ColorGreen(UINT32 Color)
{
#if (PLATFORM_OSX)
	return (BYTE)((Color >> 16) & 0xFF);
#else
	return (BYTE)((Color >> 8) & 0xFF);
#endif
}

FORCEINLINE BYTE GetUint32ColorBlue(UINT32 Color)
{
#if (PLATFORM_OSX)
	return (BYTE)((Color >> 8) & 0xFF);
#else
	return (BYTE)(Color & 0xFF);
#endif
}

FORCEINLINE RVec3 GammaToLinear(const RVec3& color)
{
	static const float exponent = 2.2f;
	return RVec3(
		powf(color.x, exponent),
		powf(color.y, exponent),
		powf(color.z, exponent)
	);
}

// Convert linear color to gamma space color
FORCEINLINE RVec3 LinearToGamma(const RVec3& color)
{
	static const float exponent = 1.0f / 2.2f;
	return RVec3(
		powf(color.x, exponent),
		powf(color.y, exponent),
		powf(color.z, exponent)
	);
}

// Convert signed linear color to signed gamma space color (same as LinearToGamma, but allows negative colors)
FORCEINLINE RVec3 LinearToGammaSigned(const RVec3& color)
{
	static const float exponent = 1.0f / 2.2f;
	return RVec3(
		Math::Sgn(color.x) * powf(fabs(color.x), exponent),
		Math::Sgn(color.y) * powf(fabs(color.y), exponent),
		Math::Sgn(color.z) * powf(fabs(color.z), exponent)
	);
}

// Pack rgb color to 32 bit
FORCEINLINE UINT32 MakePixelColor(const RVec3& color)
{
    int r = int(Math::Min(Math::Max(color.x, 0.0f), 1.0f) * 255);
	int g = int(Math::Min(Math::Max(color.y, 0.0f), 1.0f) * 255);
	int b = int(Math::Min(Math::Max(color.z, 0.0f), 1.0f) * 255);
	return MakeUint32Color(r, g, b, 255);
}
