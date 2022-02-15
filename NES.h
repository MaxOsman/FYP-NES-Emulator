#pragma once
#include "CPU.h"
#include "RAM.h"
#include "ROM.h"
#include "Debug.h"


class NES
{
public:
	NES();
	~NES();

	void Update();

	byte Read(word addr);
	void Write(byte value, word addr);

private:
	CPU* m_pCPU;
	RAM* m_pRAM;
	ROM* m_pROM;

	Debug* m_pDebug;
};