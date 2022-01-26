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
};