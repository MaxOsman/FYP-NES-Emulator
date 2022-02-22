#include "NES.h"


NES::NES()
{
	m_pROM = new ROM();
	m_pROM->LoadROM();

	m_pCPU = new CPU(this);
	m_pRAM = new RAM();
	m_pDebug = new Debug();

	Update();
}

NES::~NES()
{
	delete m_pCPU;
	delete m_pRAM;
	delete m_pDebug;
}

void NES::Update()
{
	while (!m_pCPU->Update())
	{
		if(m_pCPU->m_cycle == 0)
			m_pDebug->OutputLine(m_pROM, m_pCPU);
	}
}

byte NES::Read(word addr)
{
	if (addr < 0x2000)
	{
		// RAM
		return m_pRAM->Read(addr);
	}
	else if (addr < 0x4000)
	{
		// PPU registers
		return 0xff;
	}
	else if (addr < 0x4018)
	{
		// APU + controller registers
	}
	else
	{
		// ROM
		return m_pROM->Read(addr);
	}
}

void NES::Write(byte value, word addr)
{
	if (addr < 0x2000)
	{
		// RAM
		return m_pRAM->Write(value, addr);
	}
	else if (addr < 0x4000)
	{
		// PPU registers
	}
	else if (addr < 0x4018)
	{
		// APU + controller registers
	}
}