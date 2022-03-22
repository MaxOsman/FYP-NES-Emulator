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
	PPU(SDL_Renderer* renderer, Tile* chr, NES* parentNES);
	~PPU();

	void DrawTile(int index, int palette, int scale, int x, int y);
	void Render(SDL_Renderer* renderer);
	void Update();

	int GetScale() { return m_scale; }
	bool GetNMIEnabled() { return m_isNMIEnabled; }

	void SetMirroring(bool mirror) { m_isVerticalMirroring = mirror; }

	byte Read(word addr);
	void Write(byte value, word addr);
	byte ReadFromPPU(word addr);
	void WriteToPPU(byte value, word addr);

private:
	// Drawing
	const uint32_t mFullNESPalette[64] = {	0x656565,	0x002D69,	0x131F7F,	0x3C137C,	0x600B62,	0x730A37,	0x710F07,	0x5A1A00,
											0x342800,	0x0B3400,	0x003C00,	0x003D10,	0x003840,	0x000000,	0x000000,	0x000000,
											0xAEAEAE,	0x0F63B3,	0x4051D0,	0x7841CC,	0xA736A9,	0xC03470,	0xBD3C30,	0x9F4A00,
											0x6D5C00,	0x366D00,	0x077704,	0x00793D,	0x00727D,	0x000000,	0x000000,	0x000000,
											0xFEFEFF,	0x5DB3FF,	0x8FA1FF,	0xC890FF,	0xF785FA,	0xFF83C0,	0xFF8B7F,	0xEF9A49,
											0xBDAC2C,	0x85BC2F,	0x55C753,	0x3CC98C,	0x3EC2CD,	0x4E4E4E,	0x000000,	0x000000,
											0xFEFEFF,	0xBCDFFF,	0xD1D8FF,	0xE8D1FF,	0xFBCDFD,	0xFFCCE5,	0xFFCFCA,	0xF8D5B4,
											0xE4DCA8,	0xCCE3A9,	0xB9E8B8,	0xAEE8D0,	0xAFE5EA,	0xB6B6B6,	0x000000,	0x000000	};

	// Temp numbers
	byte m_palette[32] = {	0x0f, 0x00, 0x10, 0x20,		0x0f, 0x1c, 0x2c, 0x20,		0x0f, 0x00, 0x10, 0x20,		0x0f, 0x1c, 0x2c, 0x20,
							0x0f, 0x16, 0x27, 0x20,		0x0f, 0x12, 0x21, 0x20,		0x0f, 0x09, 0x29, 0x20,		0x0f, 0x14, 0x25, 0x34		};

	SDL_Texture* m_pFrameBuffer;
	uint32_t* m_pixels = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];
	Tile* m_pCHR = new Tile[512];

	int m_scale;

	// Emulation
	NES* m_pNES;

	byte m_Nametables[2][0x400];
	byte m_OAM[256];

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

	byte m_OAMDATA;		// $2004

	byte m_PPUSCROLL;	// $2005

	word m_PPUADDR;		// $2006

	byte m_PPUDATA;		// $2007

	byte m_OAMDMA;		// $4014

	bool m_isNMIEnabled;

	// High byte then low byte
	bool m_latch;

	bool m_isVerticalMirroring;
	int m_cycle;
	int m_scanline;

	byte m_prevPPUDATARead;
};