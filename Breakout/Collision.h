#pragma once

#include <cmath>
#include <algorithm>
#include "Shape.h"

enum class CollisionEdge
{
	None = 0,
	Top,
	Bottom,
	Left,
	Right,
};

bool IsCircleRectColliding(const Circle& circle, const Rect& rect);
CollisionEdge GetCircleRectCollisionEdge(const Circle& circle, const Rect& rect);
