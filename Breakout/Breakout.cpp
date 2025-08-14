#include <SDL2/SDL.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>

#pragma region types
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
	int width;
	int height;
};

struct Color
{
	int r;
	int g;
	int b;
};

struct Brick
{
	SDL_Rect rect;
	Color color;
	bool broken;
};

enum class CollisionEdge
{
	None = 0,
	Top, 
	Bottom, 
	Left, 
	Right, 
};

#pragma endregion
#pragma region functions
static void DrawCircle(SDL_Renderer* renderer, Position center, int radius)
{
	const int diameter = (radius * 2);

	int x = (radius - 1);
	int y = 0;
	int tx = 1;
	int ty = 1;
	int error = (tx - diameter);

	while (x >= y)
	{
		SDL_RenderDrawLine(renderer, (int)center.x - x, (int)center.y - y, (int)center.x + x, (int)center.y - y);
		SDL_RenderDrawLine(renderer, (int)center.x - x, (int)center.y + y, (int)center.x + x, (int)center.y + y);
		SDL_RenderDrawLine(renderer, (int)center.x - y, (int)center.y - x, (int)center.x + y, (int)center.y - x);
		SDL_RenderDrawLine(renderer, (int)center.x - y, (int)center.y + x, (int)center.x + y, (int)center.y + x);

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

static bool IsCircleRectIntersecting(Position circleCenter, int circleRadius, const SDL_Rect& rect)
{
	Position closestPoint{};
	closestPoint.x = std::max((float)rect.x, std::min(circleCenter.x, (float)(rect.x + rect.w)));
	closestPoint.y = std::max((float)rect.y, std::min(circleCenter.y, (float)(rect.y + rect.h)));

	float dx = circleCenter.x - closestPoint.x;
	float dy = circleCenter.y - closestPoint.y;

	return (dx * dx + dy * dy) <= (circleRadius * circleRadius);
}

static CollisionEdge GetCircleRectIntersectionEdge(Position circleCenter, int circleRadius, const SDL_Rect& rect)
{
	Position closestPoint{};
	closestPoint.x = std::max((float)rect.x, std::min(circleCenter.x, (float)rect.x + rect.w));
	closestPoint.y = std::max((float)rect.y, std::min(circleCenter.y, (float)rect.y + rect.h));

	float dx = circleCenter.x - closestPoint.x;
	float dy = circleCenter.y - closestPoint.y;

	if (dx * dx + dy * dy > circleRadius * circleRadius)
	{
		return CollisionEdge::None;
	}

	Position rectCenter{};
	rectCenter.x = rect.x + rect.w / 2.0f;
	rectCenter.y = rect.y + rect.h / 2.0f;

	float dxCenter = circleCenter.x - rectCenter.x;
	float dyCenter = circleCenter.y - rectCenter.y;

	float verticalFactor = (rect.w / 2.0f + circleRadius) * dyCenter;
	float horizontalFactor = (rect.h / 2.0f + circleRadius) * dxCenter;

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

#pragma endregion

int main(int argc, char** args)
{
#pragma region decl
	// window
	const Size windowSize = { 800, 600 };
	SDL_Window* window = nullptr;

	// renderer
	SDL_Renderer* renderer = nullptr;

	// for game loop
	SDL_Event e{};
	bool running = true;

	// paddle
	Size paddleSize = { 150, 10 };
	Position paddlePos = { (windowSize.width - paddleSize.width) / 2.0f, (float)((windowSize.height - 40) - paddleSize.height)};
	int paddleVelocity = 15;
	SDL_Rect paddleShape{};

	// ball
	int ballRadius = 8;
	Position ballCenter = { paddlePos.x + paddleSize.width / 2, paddlePos.y - ballRadius * 2};
	float ballVelocity = 7;
	Motion ballMotion = { -ballVelocity, -ballVelocity };
	using clock = std::chrono::steady_clock;
	std::chrono::time_point<clock> ballOutTime;
	bool ballWaiting = false;
	auto respawnDelay = std::chrono::milliseconds(2000);
	
	// for paddle-ball hit processing
	float hitFactor;
	float bounceAngleAcute;
	float bounceAngle;
	float bounceAngleBoundary = (float)M_PI / 18.0f;

	// bricks
	Color rainbow[10] = {
		{255, 0, 0},
		{255, 127, 0},
		{255, 255, 0},
		{127, 255, 0},
		{0, 255, 0},
		{0, 255, 127},
		{0, 255, 255},
		{0, 127, 255},
		{0, 0, 255},
		{127, 0, 255}
	};
	Brick bricks[200]{};
	for (int i = 0; i < sizeof(bricks) / sizeof(Brick); i++)
	{
		Brick& brick = bricks[i];

		brick.rect.w = 40;
		brick.rect.h = 15;

		brick.rect.x = (800 - brick.rect.w * 20) / 2 + brick.rect.w * (i % 20);
		brick.rect.y = 100 + brick.rect.h * (i / 20);

		int row = i / 20;
		brick.color = rainbow[row];
		brick.broken = false;
	}
#pragma endregion
#pragma region init
	// init SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	// create window and renderer
	window = SDL_CreateWindow("Breakout", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowSize.width, windowSize.height, SDL_WINDOW_SHOWN);
	if (!window)
	{
		std::cout << "Error creating window: " << SDL_GetError() << std::endl;
		return 1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
	{
		std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
		return 1;
	}
#pragma endregion

	// game loop
	while (running)
	{
#pragma region input
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				running = false;
			}
		}

		const unsigned char* keyboardState = SDL_GetKeyboardState(nullptr);
#pragma endregion
#pragma region update
		// paddle movement
		if (keyboardState[SDL_SCANCODE_LEFT])
		{
			paddlePos.x -= paddleVelocity;
		}
		if (keyboardState[SDL_SCANCODE_RIGHT])
		{
			paddlePos.x += paddleVelocity;
		}

		// paddle boundary (leftwall, rightwall)
		if (paddlePos.x < 0)
		{
			paddlePos.x = 0;
		}
		if (paddlePos.x > windowSize.width - paddleSize.width)
		{
			paddlePos.x = (float)(windowSize.width - paddleSize.width);
		}

		// ball bounce (leftwall, rightwall, topwall)
		if (ballCenter.x - ballRadius < 0)
		{
			ballCenter.x = (float)ballRadius;
			ballMotion.dx *= -1;
		}
		if (ballCenter.x + ballRadius > windowSize.width)
		{
			ballCenter.x = (float)(windowSize.width - ballRadius);
			ballMotion.dx *= -1;
		}
		if (ballCenter.y - ballRadius < 0)
		{
			ballCenter.y = (float)ballRadius;
			ballMotion.dy *= -1;
		}

		// ball out (bottomwall)
		if (!ballWaiting && ballCenter.y > windowSize.height + ballRadius) 
		{
			ballMotion = { 0, 0 };
			ballWaiting = true;
			ballOutTime = clock::now();
		}
		if (ballWaiting) 
		{
			if (clock::now() - ballOutTime >= respawnDelay) 
			{
				ballCenter = { paddlePos.x + paddleSize.width / 2, paddlePos.y - ballRadius * 2 };
				ballMotion = { -ballVelocity, -ballVelocity };

				bool allBroken = true;
				for (Brick& brick : bricks)
				{
					allBroken = allBroken && brick.broken;
				}

				if (allBroken)
				{
					for (Brick& brick : bricks)
					{
						brick.broken = false;
					}
				}

				ballWaiting = false;
			}
		}

		// paddle-ball collision detection
		if ((ballCenter.y + ballRadius >= paddlePos.y) &&
			(ballCenter.y < paddlePos.y) &&
			(paddlePos.x - ballRadius <= ballCenter.x) &&
			(ballCenter.x <= paddlePos.x + paddleSize.width + ballRadius))
		{
			hitFactor = (ballCenter.x - (paddlePos.x + paddleSize.width / 2.0f)) / (paddleSize.width / 2.0f + ballRadius);
			bounceAngleAcute = bounceAngleBoundary + ((float)M_PI / 2.0f - bounceAngleBoundary) * (1.0f - std::abs(hitFactor));

			if (hitFactor < 0)
			{
				bounceAngle = (float)M_PI - bounceAngleAcute;
			}
			else
			{
				bounceAngle = bounceAngleAcute;
			}

			ballMotion.dx = ballVelocity * std::cos(bounceAngle);
			ballMotion.dy = ballVelocity * -std::sin(bounceAngle);
			
			ballCenter.y = paddlePos.y - ballRadius - 1;
		}
		
		// ball-brick collision detection
		for (Brick& brick : bricks)
		{
			if (!brick.broken && IsCircleRectIntersecting(ballCenter, ballRadius, brick.rect))
			{
				brick.broken = true;
				
				switch (GetCircleRectIntersectionEdge(ballCenter, ballRadius, brick.rect))
				{
				case CollisionEdge::Top:
					ballMotion.dy *= -1;
					break;
				case CollisionEdge::Bottom:
					ballMotion.dy *= -1;
					break;
				case CollisionEdge::Left:
					ballMotion.dx *= -1;
					break;
				case CollisionEdge::Right:
					ballMotion.dx *= -1;
					break;
				default:
					__debugbreak();
					break;
				}

				break;
			}
		}

		// move ball
		ballCenter.x += ballMotion.dx;
		ballCenter.y += ballMotion.dy;
#pragma endregion
#pragma region render
		SDL_SetRenderDrawColor(renderer, 26, 16, 46, 255);
		SDL_RenderClear(renderer);

		paddleShape = { (int)paddlePos.x, (int)paddlePos.y, paddleSize.width, paddleSize.height };
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &paddleShape);

		SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
		DrawCircle(renderer, ballCenter, ballRadius);

		for (Brick& brick : bricks)
		{
			if (!brick.broken)
			{
				SDL_SetRenderDrawColor(renderer, brick.color.r, brick.color.g, brick.color.b, 255);
				SDL_RenderFillRect(renderer, &(brick.rect));
			}
		}

		SDL_SetRenderDrawColor(renderer, 255, 16, 46, 255);

		SDL_RenderPresent(renderer);
#pragma endregion
	}

#pragma region free
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
#pragma endregion

	return 0;
}