#include "ROMLoading.h"
#include "Types.h"
#include "CPU.h"
#include <iostream>
#include <fstream>
using std::cin;
using std::cout;
using std::endl;
using std::string;


void ROMLoading::LoadROM(CPU* cpu)
{
	std::string romName = "";
	cout << "Input ROM name: " << endl;
	cin >> romName;

	byte romHeader[0x10];
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
			word prgSize = 0x4000 * romHeader[4];
			if (prgSize > 0x8000)
			{
				// Must use a mapper, so not supported!
				cout << "Mappers not supported!" << endl;
			}
			else
			{
				inFile.read((char*)&romPRG[0], prgSize);
				inFile.read((char*)&romCHR[0], 0x2000);
				inFile.close();

				// Load PRG ROM
				for (unsigned int i = 0; i < prgSize; ++i)
				{
					cpu->RAM[0x8000 + i] = romPRG[i];
				}
				if (romHeader[4] == 0x1)
				{
					// If PRG does not take up all $8000 bytes, then it is mirrored, e.g. in Donkey Kong it has $4000 bytes of ROM, but two instances of it
					for (unsigned int i = 0; i < prgSize; ++i)
					{
						cpu->RAM[0xc000 + i] = romPRG[i];
					}
				}

				// Load CHR ROM
				// To do once PPU made
			}
		}
		else
		{
			cout << "ROM not valid!" << endl;
		}
	}
	else
	{
		cout << "ROM with that name not found!" << endl;
	}

	delete[] romPRG;
	delete[] romCHR;
}