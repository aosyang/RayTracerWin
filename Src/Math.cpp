#include "Math.h"

namespace RMath
{

	RVec3 RandomHemisphereDirection(const RVec3& Normal)
	{
		while (1)
		{
			float t1 = 2.0f * PI * Random();
			float t2 = acosf(1.0f - 2.0f * Random());
			RVec3 v(sinf(t1) * sinf(t2), cosf(t1) * sinf(t2), cosf(t2));

			if (v.Dot(Normal) > 0.0f)
				return v;
		}
	}

}
