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

private:
	byte* m_memory = new byte[0x8000];
};