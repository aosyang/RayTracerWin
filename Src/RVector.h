//=============================================================================
// RVector.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "MathHelper.h"
#include <math.h>

class RVec2;
class RVec3;
class RVec4;

class RVec2
{
public:
	float x, y;
	
	RVec2()
	{}

	RVec2(float _x, float _y)
		: x(_x), y(_y)
	{}

	RVec2(const float* v)
		: x(v[0]), y(v[1])
	{}

	RVec2(const RVec2& rhs)
		: x(rhs.x), y(rhs.y)
	{}

	RVec2& operator=(const RVec2& rhs)
	{
		x = rhs.x; y = rhs.y;
		return *this;
	}

	RVec2 operator-() const											{ return RVec2(-x, -y); }

	RVec2 operator+(const RVec2& rhs) const							{ return RVec2(x + rhs.x, y + rhs.y); }
	RVec2 operator-(const RVec2& rhs) const							{ return RVec2(x - rhs.x, y - rhs.y); }

	RVec2 operator*(float val) const								{ return RVec2(x * val, y * val); }
	RVec2 operator/(float val) const								{ return RVec2(x / val, y / val); }

	RVec2& operator+=(const RVec2& rhs)								{ x += rhs.x; y += rhs.y; return *this; }
	RVec2& operator-=(const RVec2& rhs)								{ x -= rhs.x; y -= rhs.y; return *this; }
	RVec2& operator*=(float val)									{ x *= val; y *= val; return *this; }
	RVec2& operator/=(float val)									{ x /= val; y /= val; return *this; }

	// Get length of vector
	float Magnitude() const
	{
		return sqrtf(x * x + y * y);
	}

	// Make unit vector
	void Normalize()
	{
		float mag = Magnitude();
		if (!FLT_EQUAL_ZERO(mag))
		{
			x /= mag;
			y /= mag;
		}
	}

	// Dot product
	float Dot(const RVec2& rhs) const
	{
		return x * rhs.x + y * rhs.y;
	}

	// Cross product
	float Cross(const RVec2& rhs) const
	{
		return x * rhs.y - y * rhs.x;
	}

	static RVec2 Zero()
	{
		return RVec2(0.0f, 0.0f);
	}

	static RVec2 Lerp(const RVec2& a, const RVec2& b, float t)
	{
		return RVec2(Math::Lerp(a.x, b.x, t),
					 Math::Lerp(a.y, b.y, t));
	}
};

class RVec3
{
public:
	float x, y, z;

	RVec3()
		: x(0.0f), y(0.0f), z(0.0f)
	{}

	RVec3(float _x, float _y, float _z)
		: x(_x), y(_y), z(_z)
	{}

	RVec3(const float* v)
		: x(v[0]), y(v[1]), z(v[2])
	{}

	RVec3(const RVec3& rhs)
		: x(rhs.x), y(rhs.y), z(rhs.z)
	{}

	RVec3& operator=(const RVec3& rhs)
	{
		x = rhs.x; y = rhs.y; z = rhs.z;
		return *this;
	}

	RVec3 operator-() const											{ return RVec3(-x, -y, -z); }

	RVec3 operator+(const RVec3& rhs) const							{ return RVec3(x + rhs.x, y + rhs.y, z + rhs.z); }
	RVec3 operator-(const RVec3& rhs) const							{ return RVec3(x - rhs.x, y - rhs.y, z - rhs.z); }

	RVec3 operator*(float val) const								{ return RVec3(x * val, y * val, z * val); }
	RVec3 operator/(float val) const								{ return RVec3(x / val, y / val, z / val); }

	RVec3 operator*(const RVec3& rhs) const							{ return RVec3(x * rhs.x, y * rhs.y, z * rhs.z); }

	RVec3& operator+=(const RVec3& rhs)								{ x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	RVec3& operator-=(const RVec3& rhs)								{ x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	RVec3& operator*=(float val)									{ x *= val; y *= val; z *= val; return *this; }
	RVec3& operator/=(float val)									{ x /= val; y /= val; z /= val; return *this; }
    
    bool operator==(const RVec3& rhs) const							{ return x == rhs.x && y == rhs.y && z == rhs.z; }
    bool operator!=(const RVec3& rhs) const							{ return x != rhs.x || y != rhs.y || z != rhs.z; }

	bool IsNonZero() const
	{
		return !FLT_EQUAL_ZERO(x) && !FLT_EQUAL_ZERO(y) && !FLT_EQUAL_ZERO(z);
	}

	float SquaredMagitude() const
	{
		return x*x + y*y + z*z;
	}

	// Get length of vector
	float Magnitude() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	// Make unit vector
	void Normalize()
	{
        *this = GetNormalizedVec3();
	}

	RVec3 GetNormalizedVec3() const
	{
		float sqr_mag = x * x + y * y + z * z;
		if (!FLT_EQUAL_ZERO(sqr_mag))
		{
            RVec3 n = *this;
            float one_over_mag = 1.0f / sqrtf(sqr_mag);
			n.x *= one_over_mag;
			n.y *= one_over_mag;
			n.z *= one_over_mag;
            
            return n;
		}
		return *this;
	}
    
    RVec3 GetNormalizedVec3_Fast() const
    {
        float sqr_mag = x * x + y * y + z * z;
        if (!FLT_EQUAL_ZERO(sqr_mag))
        {
            RVec3 n = *this;
            float one_over_mag = Math::Q_rsqrt(sqr_mag);
            n.x *= one_over_mag;
            n.y *= one_over_mag;
            n.z *= one_over_mag;
            
            return n;
        }
        return *this;
    }

	// Dot product
	static float Dot(const RVec3& a, const RVec3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	// Cross product
	static RVec3 Cross(const RVec3& a, const RVec3& b)
	{
		return RVec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
	}

	RVec3 Reflect(const RVec3& normal) const
	{
		return *this - normal * 2.0f * Dot(*this, normal);
	}

	static RVec3 Zero()
	{
		return RVec3(0.0f, 0.0f, 0.0f);
	}

	static RVec3 Lerp(const RVec3& a, const RVec3& b, float t)
	{
		return RVec3(Math::Lerp(a.x, b.x, t),
					 Math::Lerp(a.y, b.y, t),
					 Math::Lerp(a.z, b.z, t));
	}
};

class RVec4
{
public:
	float x, y, z, w;

	RVec4()
	{}

	RVec4(float _x, float _y, float _z, float _w = 1.0f)
		: x(_x), y(_y), z(_z), w(_w)
	{}

	RVec4(const float* v)
		: x(v[0]), y(v[1]), z(v[2]), w(v[3])
	{}

	RVec4(const RVec4& rhs)
		: x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
	{}

	RVec4(const RVec3 v, float _w = 1.0f)
		: x(v.x), y(v.y), z(v.z), w(_w)
	{}

	RVec4& operator=(const RVec4& rhs)
	{
		x = rhs.x; y = rhs.y; z = rhs.z; w = rhs.w;
		return *this;
	}

	RVec4 operator*(float val) const								{ return RVec4(x * val, y * val, z * val, w * val); }
	RVec4 operator/(float val) const								{ return RVec4(x / val, y / val, z / val, w / val); }

	RVec4& operator*=(float val)									{ x *= val; y *= val; z *= val; w *= val; return *this; }
	RVec4& operator/=(float val)									{ x /= val; y /= val; z /= val; w /= val; return *this; }

	float& operator[](int i)										{ return (&x)[i]; }

	RVec3 ToVec3() const
	{
		return RVec3(x, y, z);
	}

	static RVec4 Lerp(const RVec4& a, const RVec4& b, float t)
	{
		return RVec4(Math::Lerp(a.x, b.x, t),
					 Math::Lerp(a.y, b.y, t),
					 Math::Lerp(a.z, b.z, t),
					 Math::Lerp(a.w, b.w, t));
	}
};

// operator overloaded for float * RVec3
inline RVec3 operator*(float val, const RVec3& v)
{
    return RVec3(v.x * val, v.y * val, v.z * val);
}
