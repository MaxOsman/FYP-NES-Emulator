#include "CPU.h"
#include <iostream>
using std::cout;
using std::endl;

int main()
{
	CPU* cpu = new CPU();

	cpu->X = 0x5;
	cpu->RAM[0x0] = 0xea;
	cpu->RAM[0x1] = 0xea;
	cpu->RAM[0x2] = 0xea;
	cpu->RAM[0x3] = 0x75;
	cpu->RAM[0x4] = 0x42;
	cpu->RAM[0x47] = 0x68;

	while (!cpu->Update())
	{
		cout << "A: " << (int)cpu->A << endl;
	}

	return 0;
}