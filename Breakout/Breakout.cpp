#include <SDL2/SDL.h>
#include <iostream>

struct Position
{
	int x;
	int y;
};

struct Size
{
	int width;
	int height;
};

int main(int argc, char** args)
{
	const Size windowSize = { 800, 600 };

	Size paddleSize = { 200, 20 };
	Position paddlePos = { (windowSize.width - paddleSize.width) / 2, (windowSize.height - 20) - paddleSize.height };
	int paddleVelocity = 10;
	SDL_Rect paddleRect{};

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

		const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
		
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
			paddlePos.x = windowSize.width - paddleSize.width;
		}
		
		SDL_SetRenderDrawColor(renderer, 26, 16, 46, 255);
		SDL_RenderClear(renderer);

		paddleRect = { paddlePos.x, paddlePos.y, paddleSize.width, paddleSize.height };
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &paddleRect);

		SDL_RenderPresent(renderer);
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}