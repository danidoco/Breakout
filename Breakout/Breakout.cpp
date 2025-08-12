#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

#undef ENABLE_BALL_TRACE

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

int main(int argc, char** args)
{
	const Size windowSize = { 800, 600 };

	Size paddleSize = { 200, 10 };
	Position paddlePos = { (windowSize.width - paddleSize.width) / 2.0f, (float)((windowSize.height - 40) - paddleSize.height)};
	int paddleVelocity = 15;
	SDL_Rect paddleShape{};

	int ballRadius = 10;
	Position ballCenter = { paddlePos.x + paddleSize.width / 2, paddlePos.y - ballRadius * 2};
	float ballVelocity = 7;
	Motion ballMotion = { -ballVelocity, -ballVelocity };
	using clock = std::chrono::steady_clock;
	std::chrono::time_point<clock> ballOutTime;
	bool ballWaiting = false;
	auto respawnDelay = std::chrono::milliseconds(2000);
	float hitFactor;
	float bounceAngleAcute;
	float bounceAngle;
	float bounceAngleBoundary = (float)M_PI / 18.0f;

#ifdef ENABLE_BALL_TRACE
	std::vector<Position> trace;
#endif

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Breakout", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowSize.width, windowSize.height, SDL_WINDOW_SHOWN);
	if (!window)
	{
		std::cout << "Error creating window: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
	{
		std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
		return 1;
	}

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
		
		if (keyboardState[SDL_SCANCODE_LEFT])
		{
			paddlePos.x -= paddleVelocity;
		}

		if (keyboardState[SDL_SCANCODE_RIGHT])
		{
			paddlePos.x += paddleVelocity;
		}

		if (paddlePos.x < 0)
		{
			paddlePos.x = 0;
		}

		if (paddlePos.x > windowSize.width - paddleSize.width)
		{
			paddlePos.x = (float)(windowSize.width - paddleSize.width);
		}

		if (ballCenter.x - ballRadius < 0)
		{
			ballMotion.dx *= -1;
		}

		if (ballCenter.y - ballRadius < 0)
		{
			ballMotion.dy *= -1;
		}

		if (ballCenter.x + ballRadius > windowSize.width)
		{
			ballMotion.dx *= -1;
		}

		// out
		// TODO: improve behavior when out
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
#ifdef ENABLE_BALL_TRACE
				trace.clear();
#endif
				ballCenter = { paddlePos.x + paddleSize.width / 2, paddlePos.y - ballRadius * 2 };
				ballMotion = { -ballVelocity, -ballVelocity };

				ballWaiting = false;
			}
		}


		// ball-paddle collision check
		// TODO: Check collision on all sides of the paddle
		if (
			(ballCenter.y + ballRadius >= paddlePos.y) && 
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


		ballCenter.x += ballMotion.dx;
		ballCenter.y += ballMotion.dy;

		SDL_SetRenderDrawColor(renderer, 26, 16, 46, 255);
		SDL_RenderClear(renderer);

		paddleShape = { (int)paddlePos.x, (int)paddlePos.y, paddleSize.width, paddleSize.height };
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &paddleShape);

		SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
		DrawCircle(renderer, ballCenter, ballRadius);

		SDL_SetRenderDrawColor(renderer, 255, 16, 46, 255);

#ifdef ENABLE_BALL_TRACE
		trace.push_back(ballCenter);

		for (size_t i = 1; i < trace.size(); i++)
		{
			Position prevDot = trace.at(i - 1);
			Position currentDot = trace.at(i);

			SDL_SetRenderDrawColor(renderer, 255, 16, 46, 255);
			SDL_RenderDrawLine(renderer, prevDot.x, prevDot.y, currentDot.x, currentDot.y);
		}

		if (trace.size() > 50)
		{
			trace.erase(trace.begin());
		}
		
#endif
		SDL_RenderPresent(renderer);
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}