
#include "fastmath.h"

#include <numbers>

FastMath g_Math;

// This should match EQ's sin/cos LUTs
// Some values didn't match on linux, but they were the "same" float :P
FastMath::FastMath()
{
	int ci = 0;
	int si = 128;
	float res;
	do {
		res = std::cos(static_cast<float>(ci) * std::numbers::pi * 2 / 512);
		lut_cos[ci] = res;
		if (si == 512)
			si = 0;
		lut_sin[si] = res;
		++ci;
		++si;
	} while (ci < 512);

	lut_sin[0] = 0.0f;
	lut_sin[128] = 1.0f;
	lut_sin[256] = 0.0f;
	lut_sin[384] = -1.0f;

	lut_cos[0] = 1.0f;
	lut_cos[128] = 0.0f;
	lut_cos[384] = 0.0f;
}

