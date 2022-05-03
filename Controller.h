#pragma once
#include "Types.h"
#include "SDL.h"


class Controller
{
public:
	Controller();

	void Write(byte value, word addr);
	byte Read(word addr);

private:
	// 2 of each, 1 for each controller

	// 0 - A
	// 1 - B
	// 2 - Select
	// 3 - Start
	// 4 - Up
	// 5 - Down
	// 6 - Left
	// 7 - Right
	byte m_inputs[2];
	bool m_isSerialMode[2];
	int m_offset[2];

	const byte Masks[8] =
	{
		0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80
	};
};