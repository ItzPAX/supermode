#pragma once

struct vec3
{
	float x, y, z;

	bool is_null()
	{
		return x == 0 && y == 0 && z == 0;
	}
};

struct vec2
{
	float x, y;
};