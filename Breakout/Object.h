#pragma once

#include <chrono>
#include <SDL2/SDL.h>
#include "Property.h"
#include "Shape.h"

struct Paddle
{
	Rect shape;
	Velocity velocity;
};

struct Ball
{
	Circle shape;
	Velocity velocity;
	Motion motion;
	std::chrono::time_point<std::chrono::steady_clock> outTime;
	bool shouldRespawn;
	std::chrono::milliseconds respawnDelay;
};

struct Brick
{
	Rect shape;
	Color color;
	bool broken;
};
