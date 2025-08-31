#pragma once

#include <SDL2/SDL.h>
#include "Property.h"

struct Rect
{
	Size s;
	Position p;
};

struct Circle
{
	Length r;
	Position c;
};

void DrawCircle(SDL_Renderer* renderer, const Circle& circle);
