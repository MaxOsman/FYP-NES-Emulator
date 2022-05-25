#include "ROM.h"
#include "Types.h"
#include <iostream>
#include <fstream>


ROM::ROM()
{
	for (unsigned int i = 0; i < 0x8000; ++i)
	{
		m_pPRG[i] = 0xff;
	}
	for (unsigned int i = 0; i < 512; ++i)
	{
		m_pPRG[i] = 0x0;
	}
}

ROM::~ROM()
{
	delete[] m_pPRG;
	delete[] m_pCHR;
}

void ROM::LoadROM()
{
	std::string romName = "";
	std::cout << "Input ROM name: " << std::endl;
	std::cin >> romName;

	byte romHeader[0x10];
	byte romTrainer[0x200];
	byte* romPRG = new byte[0x8000];
	byte* romCHR = new byte[0x2000];

	std::ifstream inFile;
	inFile.open("ROMs/" + romName + ".nes", std::ios_base::binary);
	if (inFile)
	{
		// Header
		inFile.read((char*)&romHeader[0], 0x10);
		if (romHeader[0] == 0x4e && romHeader[1] == 0x45 && romHeader[2] == 0x53 && romHeader[3] == 0x1a)
		{
			// First 4 bytes should be "NES" + DOS end-of-file character
			// ROM is valid

			// ROM mirroring
			byte mirror = romHeader[6] & 0b00000001;
			m_isVerticalMirroring = (bool)mirror;

			// Load trainer, if present
			if (romHeader[6] & 0b00000100)
			{
				inFile.read((char*)&romTrainer[0], 0x200);
			}

			word prgSize = 0x4000 * romHeader[4];
			if (prgSize > 0x8000)
			{
				// Must use a mapper, so not supported!
				std::cout << "Mappers not supported!" << std::endl;
			}
			else
			{
				inFile.read((char*)&romPRG[0], prgSize);
				inFile.read((char*)&romCHR[0], 0x2000);
				inFile.close();

				// PRG ROM
				for (unsigned int i = 0; i < prgSize; ++i)
				{
					m_pPRG[i] = romPRG[i];
				}
				if (romHeader[4] == 0x1)
				{
					// If PRG does not take up all $8000 bytes, then it is mirrored, it has $4000 bytes of ROM, but two instances of it
					for (unsigned int i = 0; i < prgSize; ++i)
					{
						m_pPRG[0x4000 + i] = romPRG[i];
					}
				}

				// CHR ROM
				for (unsigned int i = 0; i < 0x2000; ++i)
				{
					m_pCHR[i] = romCHR[i];
				}
			}
		}
		else
		{
			std::cout << "ROM not valid!" << std::endl;
		}
	}
	else
	{
		std::cout << "ROM with that name not found!" << std::endl;
	}

	delete[] romPRG;
	delete[] romCHR;
}

byte ROM::Read(word addr)
{
	if (addr & 0x8000)
	{
		// If in the ROM address range
		addr -= 0x8000;
		return m_pPRG[addr];
	}
}
