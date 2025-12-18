#pragma once

class FastMath
{
private:
	float lut_cos[512];
	float lut_sin[512];

public:
	FastMath();

	inline float FastSin(float a) { return lut_sin[static_cast<int>(a) & 0x1ff]; }
	inline float FastCos(float a) { return lut_cos[static_cast<int>(a) & 0x1ff]; }

};
