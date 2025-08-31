#include "Shape.h"

void DrawCircle(SDL_Renderer* renderer, const Circle& circle)
{
	const int diameter = circle.r * 2;

	int x = (circle.r - 1);
	int y = 0;
	int tx = 1;
	int ty = 1;
	int error = (tx - diameter);

	while (x >= y)
	{
		SDL_RenderDrawLine(renderer, (int)circle.c.x - x, (int)circle.c.y - y, (int)circle.c.x + x, (int)circle.c.y - y);
		SDL_RenderDrawLine(renderer, (int)circle.c.x - x, (int)circle.c.y + y, (int)circle.c.x + x, (int)circle.c.y + y);
		SDL_RenderDrawLine(renderer, (int)circle.c.x - y, (int)circle.c.y - x, (int)circle.c.x + y, (int)circle.c.y - x);
		SDL_RenderDrawLine(renderer, (int)circle.c.x - y, (int)circle.c.y + x, (int)circle.c.x + y, (int)circle.c.y + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}
