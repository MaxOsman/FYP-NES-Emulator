#include "CPU.h"
#include "Debug.h"
#include <iostream>
using std::cout;
using std::endl; 
using std::string;

int main()
{
	const string AddrStrings[] =
	{
		"IMP", "ACC", "IMM", "ZER", "ZEX", "ZEY", "REL", "ABS", "ABX", "ABY", "IND", "INX", "INY", "NON"
	};
	const string OpStrings[] =
	{
		"ADC",	"AND",  "ASL",  "BCC",  "BCS",  "BEQ",  "BIT",  "BMI",  "BNE",  "BPL",  "BRK",  "BVC",  "BVS",  "CLC",

		"CLD",  "CLI",  "CLV",  "CMP",  "CPX",  "CPY",  "DEC",  "DEX",  "DEY",  "EOR",  "INC",  "INX",  "INY",  "JMP",

		"JSR",	"LDA",  "LDX",  "LDY",  "LSR",  "NOP",  "ORA",  "PHA",  "PHP",  "PLA",  "PLP",  "ROL",  "ROR",  "RTI",

		"RTS",	"SBC",  "SEC",  "SED",  "SEI",  "STA",  "STX",  "STY",  "TAX",  "TAY",  "TSX",  "TXA",  "TXS",  "TYA",

		"NON"
	};

	Debug* debug = new Debug();
	CPU* cpu = new CPU();

	cpu->RAM[0x8000] = 0x78;
	cpu->RAM[0x8001] = 0xd8;
	cpu->RAM[0x8002] = 0xa2;
	cpu->RAM[0x8003] = 0x40;
	cpu->RAM[0x8004] = 0x8e;
	cpu->RAM[0x8005] = 0x17;
	cpu->RAM[0x8006] = 0x40;
	cpu->RAM[0x8007] = 0xa2;
	cpu->RAM[0x8008] = 0xff;
	cpu->RAM[0x8009] = 0x9a;
	cpu->RAM[0x800a] = 0xe8;
	cpu->RAM[0x800b] = 0x8e;
	cpu->RAM[0x800c] = 0x00;
	cpu->RAM[0x800d] = 0x20;
	cpu->RAM[0x800e] = 0x8e;
	cpu->RAM[0x800f] = 0x01;
	cpu->RAM[0x8010] = 0x20;
	cpu->RAM[0x8011] = 0x8e;
	cpu->RAM[0x8012] = 0x10;
	cpu->RAM[0x8013] = 0x40;
	cpu->RAM[0x8014] = 0x2c;
	cpu->RAM[0x8015] = 0x02;
	cpu->RAM[0x8016] = 0x20;
	cpu->RAM[0x8017] = 0x10;
	cpu->RAM[0x8018] = 0xfb;
	cpu->RAM[0x8019] = 0x8a;

	cpu->RAM[0x801a] = 0x95;
	cpu->RAM[0x801b] = 0x00;
	cpu->RAM[0x801c] = 0x9d;
	cpu->RAM[0x801d] = 0x00;
	cpu->RAM[0x801e] = 0x01;
	cpu->RAM[0x801f] = 0x9d;
	cpu->RAM[0x8020] = 0x00;
	cpu->RAM[0x8021] = 0x03;
	cpu->RAM[0x8022] = 0x9d;
	cpu->RAM[0x8023] = 0x00;
	cpu->RAM[0x8024] = 0x04;
	cpu->RAM[0x8025] = 0x9d;
	cpu->RAM[0x8026] = 0x00;
	cpu->RAM[0x8027] = 0x05;
	cpu->RAM[0x8028] = 0x9d;
	cpu->RAM[0x8029] = 0x00;
	cpu->RAM[0x802a] = 0x06;
	cpu->RAM[0x802b] = 0x9d;
	cpu->RAM[0x802c] = 0x00;
	cpu->RAM[0x802d] = 0x07;

	cpu->RAM[0x802e] = 0xa9;
	cpu->RAM[0x802f] = 0xfd;
	cpu->RAM[0x8030] = 0x9d;
	cpu->RAM[0x8031] = 0x00;
	cpu->RAM[0x8032] = 0x20;
	cpu->RAM[0x8033] = 0xa9;
	cpu->RAM[0x8034] = 0x00;
	cpu->RAM[0x8035] = 0xe8;
	cpu->RAM[0x8036] = 0xd0;
	cpu->RAM[0x8037] = 0xe2;
	cpu->RAM[0x8038] = 0x2c;
	cpu->RAM[0x8039] = 0x02;
	cpu->RAM[0x803a] = 0x20;
	cpu->RAM[0x803b] = 0x10;
	cpu->RAM[0x803c] = 0xfb;
	cpu->RAM[0x803d] = 0xa9;
	cpu->RAM[0x803e] = 0x02;
	cpu->RAM[0x803f] = 0x8d;
	cpu->RAM[0x8040] = 0x14;
	cpu->RAM[0x8041] = 0x40;
	cpu->RAM[0x8042] = 0xea;
	cpu->RAM[0x8043] = 0xa9;
	cpu->RAM[0x8044] = 0x3f;
	cpu->RAM[0x8045] = 0x8d;
	cpu->RAM[0x8046] = 0x06;
	cpu->RAM[0x8047] = 0x20;
	cpu->RAM[0x8048] = 0xa9;
	cpu->RAM[0x8049] = 0x00;
	cpu->RAM[0x804a] = 0x8d;
	cpu->RAM[0x804b] = 0x06;
	cpu->RAM[0x804c] = 0x20;
	cpu->RAM[0x804d] = 0xa2;
	cpu->RAM[0x804e] = 0x00;
	cpu->RAM[0x804f] = 0xbd;
	cpu->RAM[0x8050] = 0x1c;
	cpu->RAM[0x8051] = 0x81;
	cpu->RAM[0x8052] = 0x8d;
	cpu->RAM[0x8053] = 0x07;
	cpu->RAM[0x8054] = 0x20;
	cpu->RAM[0x8055] = 0xe8;
	cpu->RAM[0x8056] = 0xe0;
	cpu->RAM[0x8057] = 0x20;
	cpu->RAM[0x8058] = 0xd0;
	cpu->RAM[0x8059] = 0xf5;
	cpu->RAM[0x805a] = 0xa2;
	cpu->RAM[0x805b] = 0x00;
	cpu->RAM[0x805c] = 0xbd;
	cpu->RAM[0x805d] = 0x3c;
	cpu->RAM[0x805e] = 0x81;
	cpu->RAM[0x805f] = 0x9d;
	cpu->RAM[0x8060] = 0x00;
	cpu->RAM[0x8061] = 0x02;
	cpu->RAM[0x8062] = 0xe8;
	cpu->RAM[0x8063] = 0xe0;
	cpu->RAM[0x8064] = 0x0c;
	cpu->RAM[0x8065] = 0xd0;
	cpu->RAM[0x8066] = 0xf5;
	cpu->RAM[0x8067] = 0xad;
	cpu->RAM[0x8068] = 0x04;
	cpu->RAM[0x8069] = 0x02;
	cpu->RAM[0x806a] = 0x85;
	cpu->RAM[0x806b] = 0x03;
	cpu->RAM[0x806c] = 0xad;
	cpu->RAM[0x806d] = 0x07;
	cpu->RAM[0x806e] = 0x02;
	cpu->RAM[0x806f] = 0x85;
	cpu->RAM[0x8070] = 0x04;
	cpu->RAM[0x8071] = 0xa9;
	cpu->RAM[0x8072] = 0x00;
	cpu->RAM[0x8073] = 0x85;
	cpu->RAM[0x8074] = 0xd0;

	cpu->RAM[0x8075] = 0xa9;
	cpu->RAM[0x8076] = 0x13;
	cpu->RAM[0x8077] = 0x85;
	cpu->RAM[0x8078] = 0x1e;
	cpu->RAM[0x8079] = 0xa9;
	cpu->RAM[0x807a] = 0x37;
	cpu->RAM[0x807b] = 0x85;
	cpu->RAM[0x807c] = 0x1f;

	// Skip the jsrs for now

	cpu->RAM[0x807d] = 0xa9;
	cpu->RAM[0x807e] = 0xb0;
	cpu->RAM[0x807f] = 0x8d;
	cpu->RAM[0x8080] = 0x00;
	cpu->RAM[0x8081] = 0x20;
	cpu->RAM[0x8082] = 0x85;
	cpu->RAM[0x8083] = 0xd3;
	cpu->RAM[0x8084] = 0xa9;
	cpu->RAM[0x8085] = 0x1e;
	cpu->RAM[0x8086] = 0x8d;
	cpu->RAM[0x8087] = 0x01;
	cpu->RAM[0x8088] = 0x20;
	cpu->RAM[0x8089] = 0x58;

	while (!cpu->Update())
	{
		cout << debug->HexString(cpu->programCounter) << ":        " << AddrStrings[cpu->OpcodeAddrTypes[cpu->RAM[cpu->programCounter]]] << "    " << OpStrings[cpu->OpcodeTypes[cpu->RAM[cpu->programCounter]]] << "    " << debug->HexString(cpu->RAM[cpu->programCounter]) << endl;
		//cout << "A: " << (int)cpu->A << endl;
	}

	return 0;
}