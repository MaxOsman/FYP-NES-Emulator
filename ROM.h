#pragma once
#include "Types.h"
#include <string>


class ROM
{
public:
	ROM();
	~ROM();

	void LoadROM();

	byte Read(word addr);

	Tile* GetCHR() { return m_pCHR; }
	bool GetMirroring() { return m_isVerticalMirroring; }

private:
	byte* m_pPRG = new byte[0x8000];
	Tile* m_pCHR = new Tile[512];

	bool m_isVerticalMirroring;
};