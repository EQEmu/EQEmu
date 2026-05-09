#ifndef GPOINT_H
#define GPOINT_H

#include "../zone/map.h"

class GPoint {
public:
	GPoint();
	GPoint(const GPoint &them);
	GPoint(VERTEX &v);
	GPoint(float x, float y, float z);
	
	inline void operator()(float nx, float ny, float nz) { x = nx; y = ny; z = nz; }
	
	GPoint cross(const GPoint &them) const;
	float dot3(const GPoint &them) const;
	
	const GPoint &operator+=(const GPoint &them);
	const GPoint &operator*=(const float num);
	
	float x;
	float y;
	float z;

};
GPoint operator-(const GPoint &v1, const GPoint &v2);
bool operator<(const GPoint &v1, const GPoint &v2);
bool operator>(const GPoint &v1, const GPoint &v2);

class GVector : public GPoint {
public:
	GVector();
	GVector(const GPoint &them);
	GVector(const GPoint &from, const GPoint &to);
	GVector(float x, float y, float z, float w = 1.0f);
	
	inline void operator()(float nx, float ny, float nz, float nw) { x = nx; y = ny; z = nz; W = nw; }
	float dot4(const GVector &them) const;
	float dot4(const GPoint &them) const;
	void normalize();
	float length();
	
	
	float W;
};


#endif

