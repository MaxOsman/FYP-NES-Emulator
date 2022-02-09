#include "CPU.h"
#include "Debug.h"
#include "ROMLoading.h"
#include <iostream>
using std::cin;
using std::cout;
using std::endl; 
using std::string;


int main()
{
	Debug* debug = new Debug();
	CPU* cpu = new CPU();

	ROMLoading::LoadROM(cpu);

	while (!cpu->Update())
	{
		debug->OutputLine(cpu);
	}

	return 0;
}