#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "Collision.h"
#include "Object.h"
#include "Property.h"
#include "Shape.h"

int main(int argc, char** args)
{
	const Size windowSize = { 800, 600 };
	SDL_Window* window = SDL_CreateWindow(
		"Breakout", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		windowSize.w, windowSize.h, 
		SDL_WINDOW_SHOWN);
	if (!window)
	{
		std::cout << "Error creating window: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(
		window, 
		-1, 
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
	{
		std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
		return 1;
	}

	Paddle paddle{};
	paddle.shape.s = { 150, 10 };
	paddle.shape.p = { (windowSize.w - paddle.shape.s.w) / 2.0f, (float)(windowSize.h - paddle.shape.s.h - 40) };
	paddle.velocity = 15;

	Ball ball{};
	ball.shape.r = 10;
	ball.shape.c = { paddle.shape.p.x + paddle.shape.s.w / 2.0f, paddle.shape.p.y - ball.shape.r * 2 };
	ball.velocity = 7;
	ball.motion = { -ball.velocity, -ball.velocity };
	ball.shouldRespawn = false;
	ball.respawnDelay = std::chrono::milliseconds(2000);

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

		brick.shape.s.w = 40;
		brick.shape.s.h = 15;

		brick.shape.p.x = static_cast<float>((800 - brick.shape.s.w * 20) / 2 + brick.shape.s.w * (i % 20));
		brick.shape.p.y = static_cast<float>(100 + brick.shape.s.h * (i / 20));

		int row = i / 20;
		brick.color = rainbow[row];
		brick.broken = false;
	}

	// init SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	// game loop
	SDL_Event e{};
	bool running = true;

	while (running)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				running = false;
			}
		}

		const unsigned char* keyboardState = SDL_GetKeyboardState(nullptr);

		// paddle movement
		if (keyboardState[SDL_SCANCODE_LEFT])
		{
			paddle.shape.p.x -= paddle.velocity;
		}
		if (keyboardState[SDL_SCANCODE_RIGHT])
		{
			paddle.shape.p.x += paddle.velocity;
		}

		// paddle boundary (leftwall, rightwall)
		if (paddle.shape.p.x < 0)
		{
			paddle.shape.p.x = 0;
		}
		if (paddle.shape.p.x > windowSize.w - paddle.shape.s.w)
		{
			paddle.shape.p.x = (float)(windowSize.w - paddle.shape.s.w);
		}

		// ball bounce (leftwall, rightwall, topwall)
		if (ball.shape.c.x - ball.shape.r < 0)
		{
			ball.shape.c.x = (float)ball.shape.r;
			ball.motion.dx *= -1;
		}
		if (ball.shape.c.x + ball.shape.r > windowSize.w)
		{
			ball.shape.c.x = (float)(windowSize.w - ball.shape.r);
			ball.motion.dx *= -1;
		}
		if (ball.shape.c.y - ball.shape.r < 0)
		{
			ball.shape.c.y = (float)ball.shape.r;
			ball.motion.dy *= -1;
		}

		// ball out (bottomwall)
		if (!ball.shouldRespawn && ball.shape.c.y > windowSize.h + ball.shape.r) 
		{
			ball.motion = { 0, 0 };
			ball.shouldRespawn = true;
			ball.outTime = std::chrono::steady_clock::now();
		}
		if (ball.shouldRespawn) 
		{
			if (std::chrono::steady_clock::now() - ball.outTime >= ball.respawnDelay) 
			{
				ball.shape.c = { paddle.shape.p.x + paddle.shape.s.w / 2, paddle.shape.p.y - ball.shape.r * 2 };
				ball.motion = { -ball.velocity, -ball.velocity };

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

				ball.shouldRespawn = false;
			}
		}

		// paddle-ball collision detection
		if (IsCircleRectColliding(ball.shape, paddle.shape))
		{
			hitFactor = (ball.shape.c.x - (paddle.shape.p.x + paddle.shape.s.w / 2.0f)) / (paddle.shape.s.w / 2.0f + ball.shape.r);
			bounceAngleAcute = bounceAngleBoundary + ((float)M_PI / 2.0f - bounceAngleBoundary) * (1.0f - std::abs(hitFactor));

			if (hitFactor < 0)
			{
				bounceAngle = (float)M_PI - bounceAngleAcute;
			}
			else
			{
				bounceAngle = bounceAngleAcute;
			}

			ball.motion.dx = ball.velocity * std::cos(bounceAngle);
			ball.motion.dy = ball.velocity * -std::sin(bounceAngle);
			
			ball.shape.c.y = paddle.shape.p.y - ball.shape.r - 1;
		}
		
		// ball-brick collision detection
		for (Brick& brick : bricks)
		{
			if (!brick.broken && IsCircleRectColliding(ball.shape, brick.shape))
			{
				brick.broken = true;
				
				switch (GetCircleRectCollisionEdge(ball.shape, brick.shape))
				{
				case CollisionEdge::Top:
					ball.motion.dy *= -1;
					break;
				case CollisionEdge::Bottom:
					ball.motion.dy *= -1;
					break;
				case CollisionEdge::Left:
					ball.motion.dx *= -1;
					break;
				case CollisionEdge::Right:
					ball.motion.dx *= -1;
					break;
				default:
					__debugbreak();
					break;
				}

				break;
			}
		}

		// move ball
		ball.shape.c.x += ball.motion.dx;
		ball.shape.c.y += ball.motion.dy;

		SDL_SetRenderDrawColor(renderer, 26, 16, 46, 255);
		SDL_RenderClear(renderer);

		SDL_Rect paddleRenderTarget = 
		{ 
			.x = std::lroundf(paddle.shape.p.x), 
			.y = std::lroundf(paddle.shape.p.y), 
			.w = paddle.shape.s.w, 
			.h = paddle.shape.s.h, 
		};

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &paddleRenderTarget);

		SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
		DrawCircle(renderer, ball.shape);

		for (Brick& brick : bricks)
		{
			if (!brick.broken)
			{
				SDL_Rect brickRenderTarget = 
				{ 
					.x = std::lroundf(brick.shape.p.x), 
					.y = std::lroundf(brick.shape.p.y), 
					.w = brick.shape.s.w, 
					.h = brick.shape.s.h 
				};
				SDL_SetRenderDrawColor(renderer, brick.color.r, brick.color.g, brick.color.b, 255);
				SDL_RenderFillRect(renderer, &brickRenderTarget);
			}
		}

		SDL_SetRenderDrawColor(renderer, 255, 16, 46, 255);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}