#include "PPU.h"
#include "NES.h"


PPU::PPU(SDL_Renderer* renderer, Tile* chr, NES* parentNES)
{
	for (unsigned int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
	{
		m_pixels[i] = 0x0;
	}

	m_pFrameBuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	m_scale = 3;
	m_pCHR = chr;
	m_pNES = parentNES;

	m_isNMIEnabled = false;
	m_latch = false;
	m_cycle = 0;
	m_scanline = 0;
	m_OAMADDR = 0x0;
	m_PPUADDR = 0x0;
	m_prevPPUDATARead = 0x0;

	for (size_t i = 0; i < 0x400; ++i)
	{
		m_Nametables[0][i] = 0x0;
		m_Nametables[1][i] = 0x0;
	}
}

PPU::~PPU()
{
	m_pFrameBuffer = nullptr;
	delete m_pFrameBuffer;
	delete[] m_pixels;
	m_pCHR = nullptr;
	m_pNES = nullptr;
}

void PPU::DrawTile(int index, int palette, int scale, int x, int y)
{
	// Prepare all 4 possible colours
	uint32_t RGBColours[4];
	RGBColours[0] = mFullNESPalette[m_palette[0]];
	RGBColours[1] = mFullNESPalette[m_palette[4 * palette + 1]];
	RGBColours[2] = mFullNESPalette[m_palette[4 * palette + 2]];
	RGBColours[3] = mFullNESPalette[m_palette[4 * palette + 3]];

	bool lowBit, highBit;
	byte colourInPalette;
	for (unsigned int i = 0; i < 8; ++i)
	{
		for (unsigned int j = 0; j < 8; ++j)
		{
			int offsetX = 0, offsetY = 0;

			// For 2 nametables
			/*if ((j + x) < -SCREEN_WIDTH)
				offsetX = SCREEN_WIDTH * 2;
			if ((i + y) < -SCREEN_HEIGHT)
				offsetY = SCREEN_HEIGHT * 2;*/

			// Prevent drawing outside array
			if ((j + x) >= SCREEN_WIDTH || ((i + y) >= SCREEN_HEIGHT || ((j + x) < 0 && (j + x) >= -SCREEN_WIDTH) || ((i + y) < 0 && (i + y) >= -SCREEN_HEIGHT)))
				continue;

			colourInPalette = m_pCHR[index].colours[(8 * i) + j];
			m_pixels[(j + x + offsetX) + (SCREEN_WIDTH * (i + y + offsetY))] = RGBColours[colourInPalette];
		}
	}
}

void PPU::Render(SDL_Renderer* renderer)
{
	SDL_UpdateTexture(m_pFrameBuffer, NULL, m_pixels, SCREEN_WIDTH * sizeof(uint32_t));
	SDL_RenderCopy(renderer, m_pFrameBuffer, NULL, NULL);

	for (unsigned int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
	{
		m_pixels[i] = mFullNESPalette[m_palette[0]];
	}

	// Test - draw every tile in CHR
	/*for (unsigned int i = 0; i < SCREEN_HEIGHT/8; ++i)
	{
		for (unsigned int j = 0; j < SCREEN_WIDTH/8; ++j)
		{
			DrawTile(i * 8 + j, (i * 8 + j) % 8, m_scale, j*8, i*8);
		}
	}*/

	// Test - Draw nametable 0
	/*byte palette;
	for (unsigned int i = 0; i < 0x3c0; ++i)
	{
		palette = m_Nametables[0][i / 4 + 0x3c0];
		DrawTile(m_Nametables[0][i] + 256, palette, m_scale, (i % 32) * 8, (i / 32) * 8);
	}*/

	byte Masks[4] =
	{
		0b11000000, 0b00110000, 0b00001100, 0b00000011
	};
	byte Factors[4] =
	{
		64, 16, 4, 1
	};
	for (unsigned int i = 0; i < SCREEN_HEIGHT / 8; ++i)
	{
		for (unsigned int j = 0; j < SCREEN_WIDTH / 8; ++j)
		{
			int index = i * (SCREEN_WIDTH / 8) + j;
			byte attrOffset = m_Nametables[0][(j & 0b11111100) / 4 + 2 * (i & 0b11111100) + 0x3c0];
			byte mask = (attrOffset & Masks[index % 4]) / Factors[index % 4];

			DrawTile(m_Nametables[0][index] + 256, mask, m_scale, j*8, i*8);
		}
	}
}

void PPU::Update()
{

}

byte PPU::Read(word addr)
{
	addr &= 0x7;
	byte temp;
	switch (addr)
	{
	case 0x2:
		return 0xff;
		// PPU STATUS
		// Only top 3 bits used, may be an accuracy issue
		temp = m_PPUSTATUS & 0b11100000;

		// Clear bit 7
		m_PPUSTATUS &= 0b01111111;

		// Reset latch
		m_latch = false;

		return temp;

	case 0x4:
		// OAM DATA
		return m_OAM[m_OAMADDR];

	case 0x7:
		// PPU DATA
		// Delay read by 1 frame
		byte temp = m_prevPPUDATARead;
		m_prevPPUDATARead = ReadFromPPU(m_PPUADDR);
		if (m_PPUADDR >= 0x3f00)
		{
			temp = m_prevPPUDATARead;
		}

		// Increment mode
		m_PPUADDR = (0b00000100 & m_PPUCTRL ? m_PPUADDR + 0x20 : m_PPUADDR + 0x1);

		return temp;
	}

	return 0x0;
}

void PPU::Write(byte value, word addr)
{
	if (addr == 0x4014)
	{
		// OAM DMA
	}
	else
	{
		addr &= 0x7;
		switch (addr)
		{
		case 0x0:
			// PPU CTRL
			m_PPUCTRL = value;
			break;

		case 0x1:
			// PPU MASK
			m_PPUMASK = value;
			break;

		case 0x3:
			// OAM ADDR
			m_OAMADDR = value;
			break;

		case 0x4:
			// OAM DATA
			m_OAM[m_OAMADDR] = value;
			break;

		case 0x5:
			// PPU SCROLL
			break;

		case 0x6:
			// PPU ADDR
			word temp;
			if (m_latch)
			{
				// Low byte
				temp = m_PPUADDR & 0xff00;
				m_PPUADDR = temp + (word)value;
			}
			else
			{
				// High byte
				temp = (value & 0x3f) * 0x100;
				m_PPUADDR &= 0xff;
				m_PPUADDR += temp;
			}
			m_latch = !m_latch;
			break;

		case 0x7:
			// PPU DATA
			WriteToPPU(value, m_PPUADDR);

			// Increment mode
			m_PPUADDR = (0b00000100 & m_PPUCTRL ? m_PPUADDR + 0x20 : m_PPUADDR + 0x1);
			break;
		}
	}
}

byte PPU::ReadFromPPU(word addr)
{
	if (addr < 0x2000)
	{
		// Pattern tables
		word arrayAddr = addr / 0x10;
		byte colourAddr = addr - arrayAddr;
		return m_pCHR[arrayAddr].colours[4 * colourAddr] + 4 * m_pCHR[arrayAddr].colours[4 * colourAddr + 1] + 16 * m_pCHR[arrayAddr].colours[4 * colourAddr + 2] + 64 * m_pCHR[arrayAddr].colours[4 * colourAddr + 3];
	}
	else if (addr < 0x3f00)
	{
		// Nametables
		if (addr >= 0x3000)
		{
			// Mirrors
			addr -= 0x1000;
		}

		const int tables[8] =
		{
			0, 1, 0, 1, 0, 0, 1, 1
		};
		int index = m_isVerticalMirroring ? 0 : 4;

		if (addr < 0x2400)
		{
			return m_Nametables[tables[index]][addr & 0x3ff];
		}
		else if (addr < 0x2800)
		{
			return m_Nametables[tables[index+1]][addr & 0x3ff];
		}
		else if (addr < 0x2c00)
		{
			return m_Nametables[tables[index+1]][addr & 0x3ff];
		}
		else if (addr < 0x3000)
		{
			return m_Nametables[tables[index+1]][addr & 0x3ff];
		}
	}
	else if (addr < 0x4000)
	{
		return m_palette[addr & 0x1f];
	}
}

void PPU::WriteToPPU(byte value, word addr)
{
	if (addr < 0x3f00)
	{
		// Nametables
		if (addr >= 0x3000)
		{
			// Mirrors
			addr -= 0x1000;
		}

		const int tables[8] =
		{
			0, 1, 0, 1, 0, 0, 1, 1
		};
		int index = m_isVerticalMirroring ? 0 : 4;

		if (addr < 0x2400)
		{
			m_Nametables[tables[index]][addr & 0x3ff] = value;
		}
		else if (addr < 0x2800)
		{
			m_Nametables[tables[index + 1]][addr & 0x3ff] = value;
		}
		else if (addr < 0x2c00)
		{
			m_Nametables[tables[index + 1]][addr & 0x3ff] = value;
		}
		else if (addr < 0x3000)
		{
			m_Nametables[tables[index + 1]][addr & 0x3ff] = value;
		}
	}
	else if (addr < 0x4000)
	{
		m_palette[addr & 0x1f] = value;
	}
}