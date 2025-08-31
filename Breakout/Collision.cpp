#include "Collision.h"

bool IsCircleRectColliding(const Circle& circle, const Rect& rect)
{
	Position closestPoint{};
	closestPoint.x = std::max(rect.p.x, std::min(circle.c.x, rect.p.x + rect.s.w));
	closestPoint.y = std::max(rect.p.y, std::min(circle.c.y, rect.p.y + rect.s.h));

	float dx = circle.c.x - closestPoint.x;
	float dy = circle.c.y - closestPoint.y;

	return (dx * dx + dy * dy) <= (circle.r * circle.r);
}

CollisionEdge GetCircleRectCollisionEdge(const Circle& circle, const Rect& rect)
{
	Position closestPoint{};
	closestPoint.x = std::max(rect.p.x, std::min(circle.c.x, rect.p.x + rect.s.w));
	closestPoint.y = std::max(rect.p.y, std::min(circle.c.y, rect.p.y + rect.s.h));

	float dx = circle.c.x - closestPoint.x;
	float dy = circle.c.y - closestPoint.y;

	if (dx * dx + dy * dy > circle.r * circle.r)
	{
		return CollisionEdge::None;
	}

	Position rectCenter{};
	rectCenter.x = rect.p.x + rect.s.w / 2.0f;
	rectCenter.y = rect.p.y + rect.s.h / 2.0f;

	float dxCenter = circle.c.x - closestPoint.x;
	float dyCenter = circle.c.y - closestPoint.y;

	float verticalFactor = (rect.s.w / 2.0f + circle.r) * dyCenter;
	float horizontalFactor = (rect.s.h / 2.0f + circle.r) * dxCenter;

	if (verticalFactor > horizontalFactor)
	{
		if (verticalFactor > -horizontalFactor)
		{
			return CollisionEdge::Bottom;
		}
		else
		{
			return CollisionEdge::Left;
		}
	}
	else
	{
		if (verticalFactor > -horizontalFactor)
		{
			return CollisionEdge::Right;
		}
		else
		{
			return CollisionEdge::Top;
		}
	}
}
