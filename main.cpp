#include "NES.h"


// SDL2 setup based on SDL Mario project
SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
NES* g_pNES = nullptr;
Uint32 g_oldTime;

bool InitSDL();
void CloseSDL();

int main(int argc, char* argv[])
{
	InitSDL();
	TTF_Init();
	g_pNES = new NES(g_renderer);
	g_oldTime = SDL_GetTicks();

	while (true)
	{
		Uint32 newTime = SDL_GetTicks();
		SDL_Event _event;
		SDL_PollEvent(&_event);
		printf(SDL_GetError());

		switch (_event.type)
		{
		case SDL_QUIT:
			return true;
			break;
		}

		g_pNES->Update(g_renderer, (float)(newTime - g_oldTime) / 1000.0f);

		g_oldTime = newTime;
	}

	CloseSDL();

	return 0;
}

bool InitSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		std::cout << "SDL did not initialise. Error: " << SDL_GetError();
		return false;
	}
	else
	{
		g_window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256*3, 240*3, SDL_WINDOW_SHOWN);
		g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);

		return true;
	}
}

void CloseSDL()
{
	SDL_DestroyWindow(g_window);
	SDL_DestroyRenderer(g_renderer);
	g_renderer = nullptr;
	g_window = nullptr;
	g_pNES = nullptr;
	delete g_pNES;
	TTF_Quit();
	SDL_Quit();
}