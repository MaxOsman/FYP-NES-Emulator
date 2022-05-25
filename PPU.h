#pragma once
#include "Types.h"
#include "SDL.h"
#include "SDL_ttf.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

class NES;


class PPU
{
public:
	PPU(SDL_Renderer* renderer, byte* chr, NES* parentNES);
	~PPU();

	void DrawTile(byte y, byte tile, byte attr, byte x);
	void DrawPixel(int palette, int colour, byte x, byte y);
	void Render(SDL_Renderer* renderer);
	bool Update(unsigned long long int m_totalCycles);

	int GetScale() { return m_scale; }
	bool GetNMIEnabled() { return m_isNMIEnabled; }
	bool GetNMI() { return m_isNMI; }

	void SetMirroring(bool mirror) { m_isVerticalMirroring = mirror; }
	void FlipCycle() { m_isEvenCycle = !m_isEvenCycle;}
	void NMIOff() { m_isNMI = false; }

	byte Read(word addr);
	void Write(byte value, word addr);
	byte ReadFromPPU(word addr);
	void WriteToPPU(byte value, word addr);

private:
	// Drawing
	const uint32_t m_FullNESPalette[64] = {	0x656565,	0x002D69,	0x131F7F,	0x3C137C,	0x600B62,	0x730A37,	0x710F07,	0x5A1A00,
											0x342800,	0x0B3400,	0x003C00,	0x003D10,	0x003840,	0x000000,	0x000000,	0x000000,
											0xAEAEAE,	0x0F63B3,	0x4051D0,	0x7841CC,	0xA736A9,	0xC03470,	0xBD3C30,	0x9F4A00,
											0x6D5C00,	0x366D00,	0x077704,	0x00793D,	0x00727D,	0x000000,	0x000000,	0x000000,
											0xFEFEFF,	0x5DB3FF,	0x8FA1FF,	0xC890FF,	0xF785FA,	0xFF83C0,	0xFF8B7F,	0xEF9A49,
											0xBDAC2C,	0x85BC2F,	0x55C753,	0x3CC98C,	0x3EC2CD,	0x4E4E4E,	0x000000,	0x000000,
											0xFEFEFF,	0xBCDFFF,	0xD1D8FF,	0xE8D1FF,	0xFBCDFD,	0xFFCCE5,	0xFFCFCA,	0xF8D5B4,
											0xE4DCA8,	0xCCE3A9,	0xB9E8B8,	0xAEE8D0,	0xAFE5EA,	0xB6B6B6,	0x000000,	0x000000	};

	// Temp numbers
	byte m_palette[32];

	SDL_Texture* m_pFrameBuffer;
	uint32_t* m_screenPixels = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];
	byte* m_pCHR = new byte[0x2000];
	int m_scale;

	// Emulation
	NES* m_pNES;

	byte m_Nametables[2][0x400];
	byte m_OAM[256];

	// DMA
	bool m_isDMA;
	bool m_isEvenCycle;
	byte m_DMAPage;
	byte m_DMAValue;
	byte m_DMAAddr;

	// Bit 0 to 1 - Base nametable
	// Bit 2 - VRAM increment
	// Bit 3 - Sprite pattern table
	// Bit 4 - Background pattern table
	// Bit 5 - Sprite size
	// Bit 6 - Master/slave select (Don't bother)
	// Bit 7 - Do NMI
	byte m_PPUCTRL;		// $2000

	// Bit 0 - Greyscale
	// Bit 1 - Mask background
	// Bit 2 - Mask sprites
	// Bit 3 - Draw background
	// Bit 4 - Draw sprites
	// Bit 5 to 7 - Emphasize red to green
	byte m_PPUMASK;		// $2001

	// Bit 0 to 4 - Janky nonsense
	// Bit 5 - Sprite overflow
	// Bit 6 - Sprite 0
	// Bit 7 - VBlank
	byte m_PPUSTATUS;	// $2002

	byte m_OAMADDR;		// $2003

	bool m_isNMIEnabled;

	// High byte then low byte
	bool m_latch;

	bool m_isVerticalMirroring;
	int m_tableIndex;
	// Pixel along each scanline
	int m_pixel;
	int m_scanline;
	bool m_isNMI;

	byte m_prevPPUDATARead;

	// Scrolling
	// 0yyyNNYYYYYXXXXX
	// Bit 0 to 4 - coarse X
	// Bit 5 to 9 - coarse Y
	// Bit 10 to 11 - nametable
	// Bit 12 to 14 - fine Y
	word m_VRAMAddr;
	word m_tempVRAMAddr;
	int m_fineX;
	word m_readAddr;

	word m_tilePatternShift[2];
	byte m_tileAttrShift[2];
	byte m_fetchNametable;
	byte m_fetchAttr;
	byte m_fetchPattern[2];

	byte pixelColour;
	byte pixelPalette;
	bool pixelLow;
	bool pixelHigh;

	const byte Masks[4] =
	{
		0b11000000, 0b00110000, 0b00001100, 0b00000011
	};
	const byte Factors[4] =
	{
		64, 16, 4, 1
	};
	const int Tables[8] =
	{
		0, 1, 0, 1, 0, 0, 1, 1
	};
	const word Powers[16] =
	{
		1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
	};
};