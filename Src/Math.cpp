#include "Math.h"

RVec3 RandomHemisphereDir(const RVec3& dir)
{
	while (1)
	{
		float t1 = 2.0f * PI * Random();
		float t2 = acosf(1.0f - 2.0f * Random());
		RVec3 v(sinf(t1) * sinf(t2), cosf(t1) * sinf(t2), cosf(t2));

		if (v.Dot(dir) > 0.0f)
			return v;
	}
}
