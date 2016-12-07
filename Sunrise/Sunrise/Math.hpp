#pragma once
#include <math.h>

namespace sr
{
    // Determines whether the two floating-point values f1 and f2 are
    // close enough together that they can be considered equal.
	bool closeEnough(float f1, float f2)
    {
		const float EPSILON = 1e-6f;
        return fabsf((f1 - f2) / ((f2 == 0.0f) ? 1.0f : f2)) < EPSILON;
    }
}