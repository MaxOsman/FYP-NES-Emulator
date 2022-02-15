#pragma once
#include "Types.h"

class RAM
{
public:
	RAM()
	{
		for (unsigned int i = 0; i < 0x800; ++i)
		{
			// To do - initialise to random numbers
			m_memory[i] = 0xff;
		}
	}

	~RAM()
	{
		delete[] m_memory;
	}

	byte Read(word addr)
	{
		// Limit to 11 bit address
		addr &= 0x7ff;
		return m_memory[addr];
	}

	void Write(byte value, word addr)
	{
		// Limit to 11 bit address
		addr &= 0x7ff;
		m_memory[addr] = value;
	}

private:
	byte* m_memory = new byte[0x800];

};