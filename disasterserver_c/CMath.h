#ifndef CMATH_H
#define CMATH_H
#include <math.h>

typedef struct
{
	float x;
	float y;
} Vector2;

float	lerp(float a, float b, float f);
float	sign(float x);

float	vector2_dist(Vector2* a, Vector2* b);
Vector2	vector2_dir(Vector2* a, Vector2* b);

#endif
