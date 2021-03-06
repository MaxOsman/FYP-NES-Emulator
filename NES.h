#pragma once
#include "CPU.h"
#include "RAM.h"
#include "ROM.h"
#include "Debug.h"
#include "PPU.h"
#include "Controller.h"
#include <sstream>


class NES
{
public:
	NES(SDL_Renderer* renderer);
	~NES();

	void Update(SDL_Renderer* renderer, float deltaTime);
	void Render(SDL_Renderer* renderer);

	byte Read(word addr);
	void Write(byte value, word addr);

	void DrawText(SDL_Renderer* renderer, Vec2D position, const char* text);

private:
	CPU* m_pCPU;
	RAM* m_pRAM;
	ROM* m_pROM;
	PPU* m_pPPU;
	Controller* m_pCtrl;

	Debug* m_pDebug;

	float m_timer;
	SDL_Texture* m_pTextTexture;
	SDL_Surface* m_pSurface;
	TTF_Font* m_pFont;
	std::ostringstream oss;
	int m_prevOutputCycles = 0;
	float m_tempDeltaTime = 0;

	unsigned long long int m_totalCycles;

	float m_prevTimer;
	float m_storedTimer;
};