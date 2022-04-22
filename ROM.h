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

	byte* GetCHR() { return m_pCHR; }
	bool GetMirroring() { return m_isVerticalMirroring; }

private:
	byte* m_pPRG = new byte[0x8000];
	byte* m_pCHR = new byte[0x2000];

	bool m_isVerticalMirroring;
};