#include <CMath.h>

float lerp(float a, float b, float f)
{
	return a * (1.0f - f) + (b * f);
}

float sign(float x)
{
	return ((x) > 0) ? 1.f : (((x) < 0) ? -1.f : 0.f);
}

float vector2_dist(Vector2* a, Vector2* b)
{
	return sqrtf(powf(b->x - a->x, 2) + powf(b->y - a->y, 2));
}

Vector2 vector2_dir(Vector2* a, Vector2* b)
{
	return (Vector2) { sign(a->x - b->x), sign(a->y - b->y) };
}
