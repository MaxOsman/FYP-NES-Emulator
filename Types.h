#pragma once

typedef unsigned char byte;
typedef unsigned short word;

struct Tile
{
	// Ranging 0 to 3
	byte colours[64] = { 0x0 };
};

struct Vec2D
{
	float x;
	float y;
	Vec2D(float xx, float yy)
	{
		x = xx;
		y = yy;
	}
	Vec2D()
	{
		x = 0;
		y = 0;
	}
};