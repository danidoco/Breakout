#pragma once

using Length = unsigned short;
using Velocity = float;

struct Position
{
	float x;
	float y;
};

struct Motion
{
	float dx;
	float dy;
};

struct Size
{
	int w;
	int h;
};

struct Color
{
	int r;
	int g;
	int b;
};
