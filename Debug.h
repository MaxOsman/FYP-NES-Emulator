#pragma once
#include "CPU.h"
#include <iostream>

class Debug
{
public:
	Debug() {}

	const std::string Chars[0x10] =
	{
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"
	};

	const std::string AddrStrings[14] =
	{
		"IMP", "ACC", "IMM", "ZER", "ZEX", "ZEY", "REL", "ABS", "ABX", "ABY", "IND", "INX", "INY", "NON"
	};

	const std::string OpStrings[57] =
	{
		"ADC",	"AND",  "ASL",  "BCC",  "BCS",  "BEQ",  "BIT",  "BMI",  "BNE",  "BPL",  "BRK",  "BVC",  "BVS",  "CLC",

		"CLD",  "CLI",  "CLV",  "CMP",  "CPX",  "CPY",  "DEC",  "DEX",  "DEY",  "EOR",  "INC",  "INX",  "INY",  "JMP",

		"JSR",	"LDA",  "LDX",  "LDY",  "LSR",  "NOP",  "ORA",  "PHA",  "PHP",  "PLA",  "PLP",  "ROL",  "ROR",  "RTI",

		"RTS",	"SBC",  "SEC",  "SED",  "SEI",  "STA",  "STX",  "STY",  "TAX",  "TAY",  "TSX",  "TXA",  "TXS",  "TYA",

		"NON"
	};

	std::string HexString(byte value)
	{
		byte tens = value / 0x10;
		byte digits = value - tens * 0x10;

		return "0x" + Chars[tens] + Chars[digits];
	}

	std::string HexString(word value)
	{
		// 0x1234
		// 0x1000, 0x0200, 0x0030, 0x0004
		byte thousands = value / 0x1000;
		byte hundreds = (value - thousands * 0x1000) / 0x100;
		byte tens = (value - (thousands * 0x1000 + hundreds * 0x100)) / 0x10;
		byte digits = value - (thousands * 0x1000 + hundreds * 0x100 + tens * 0x10);

		return "0x" + Chars[thousands] + Chars[hundreds] + Chars[tens] + Chars[digits];
	}

	void OutputLine(CPU* cpu)
	{
		std::cout << HexString(cpu->programCounter) << ":        " << AddrStrings[cpu->OpcodeAddrTypes[cpu->RAM[cpu->programCounter]]] << "    " << OpStrings[cpu->OpcodeTypes[cpu->RAM[cpu->programCounter]]] << "    " << HexString(cpu->RAM[cpu->programCounter]) <<
			"    |    " << "A = " << HexString(cpu->A) << ",  " << "X = " << HexString(cpu->X) << ",  " << "Y = " << HexString(cpu->Y) << std::endl;
	}
};