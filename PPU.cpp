#include "PPU.h"
#include "NES.h"


PPU::PPU(SDL_Renderer* renderer, byte* chr, NES* parentNES)
{
	for (unsigned int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
	{
		m_screenPixels[i] = 0x0;
	}

	m_pFrameBuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	m_scale = 3;
	m_pCHR = chr;
	m_pNES = parentNES;

	m_isNMIEnabled = false;
	m_latch = false;
	m_pixel = 0;
	m_scanline = 0;
	m_OAMADDR = 0x0;
	m_VRAMAddr = 0x0;
	m_tempVRAMAddr = 0x0;
	m_prevPPUDATARead = 0x0;
	m_isNMI = false;
	m_isDMA = false;
	m_isEvenCycle = true;
	m_DMAPage = 0x0;
	m_DMAValue = 0x0;
	m_DMAAddr = 0x0;
	m_readAddr = 0x0;
	m_tableIndex = m_isVerticalMirroring ? 0 : 4;

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
	delete[] m_screenPixels;
	m_pCHR = nullptr;
	m_pNES = nullptr;
}

void PPU::DrawTile(byte y, byte tile, byte attr, byte x)
{
	if (y > 0 && y < 0xef)
	{
		byte lowPlane, highPlane, mask, colour;
		const int flipVert = attr & 0x80 ? -1 : 1;
		const int flipHori = attr & 0x40 ? -1 : 1;
		const int height = m_PPUCTRL & 0b00100000 ? 16 : 8;
		int offset = 0;
		y = attr & 0x80 ? y + height - 1 : y;
		x = attr & 0x40 ? x + 7 : x;
		for (unsigned int i = 0; i < height; ++i)
		{
			if (i == 8)
			{
				offset = 8;
			}
			if (y < SCREEN_HEIGHT)
			{
				lowPlane = m_pCHR[16 * tile + i + offset];
				highPlane = m_pCHR[16 * tile + i + 8 + offset];
				mask = 0x80;
				for (unsigned int j = 0; j < 8; ++j)
				{
					colour = (bool)(lowPlane & mask) + 2 * (bool)(highPlane & mask);
					if (colour)
					{
						m_screenPixels[x + (SCREEN_WIDTH * y)] = m_FullNESPalette[m_palette[4 * (attr & 0b11) + colour + 16]];
						m_PPUSTATUS |= 0x40;
					}
					mask /= 0x2;
					x += flipHori;
				}
			}
			y += flipVert;
			x -= (flipHori * 8);
		}
	}
}

void PPU::DrawPixel(int palette, int colour, byte x, byte y)
{
	m_screenPixels[x + (SCREEN_WIDTH * y)] = m_FullNESPalette[m_palette[4 * palette + colour]];
}

void PPU::Render(SDL_Renderer* renderer)
{
	SDL_UpdateTexture(m_pFrameBuffer, NULL, m_screenPixels, SCREEN_WIDTH * sizeof(uint32_t));
	SDL_RenderCopy(renderer, m_pFrameBuffer, NULL, NULL);
}

bool PPU::Update(unsigned long long int m_totalCycles)
{
	// Scanline 0 - before screen
	// Scanlines 1 to 240 - screen
	// Scanlines 241 to 261 - VBlank
	if (m_scanline <= 240)
	{
		if (m_pixel == 1 && m_scanline == 0)
		{
			// Initialise frame
			m_PPUSTATUS &= 0b00011111;
		}
		else if ((m_pixel >= 2 && m_pixel <= 257) || (m_pixel >= 321 && m_pixel <= 337))
		{
			// If drawing frame, or fetching for next frame
			// Shift the temporary registers
			if (m_PPUMASK && 0b00001000)
			{
				// Background shifters
				m_tilePatternShift[0] *= 2;
				m_tilePatternShift[1] *= 2;

				m_tileAttrShift[0] *= 2;
				m_tileAttrShift[0] += (m_fetchAttr & 0b1);
				m_tileAttrShift[1] *= 2;
				m_tileAttrShift[1] += (m_fetchAttr & 0b10);
			}

			switch ((m_pixel - 1) % 8)
			{
			case 0:
				// Load background shifters
				// Low byte will be empty so this is safe
				m_tilePatternShift[0] |= m_fetchPattern[0];
				m_tilePatternShift[1] |= m_fetchPattern[1];

				// Tile in nametable
				m_readAddr = m_VRAMAddr & 0x0fff;
				m_fetchNametable = m_Nametables[Tables[m_tableIndex + (m_readAddr / 0x400)]][m_readAddr & 0x3ff];
				break;

			case 2:
				// Palette in nametable
				m_readAddr = 0x3c0 + ((m_VRAMAddr & 0b0000000000011111) / 4) + ((m_VRAMAddr & 0b0000001110000000) / 16) + (m_VRAMAddr & 0b0000110000000000);
				m_fetchAttr = m_Nametables[Tables[m_tableIndex + (m_readAddr / 0x400)]][m_readAddr & 0x3ff];

				// Find tile in 2x2 set
				m_fetchAttr /= Powers[( 4 * (int)(m_VRAMAddr && 0b0000000001000000) + 2 * (int)(m_VRAMAddr && 0b0000000000000010))];
				break;

			case 4:
				// Graphics from CHR
				m_readAddr = ((bool)(m_PPUCTRL & 0b00010000) * 0x1000) + (word)(m_fetchNametable * 16) + ((m_VRAMAddr & 0b0111000000000000) / 4096);

				// Pattern tables
				m_fetchPattern[0] = m_pCHR[m_readAddr];
				break;

			case 6:
				// Pattern tables 2
				m_fetchPattern[1] = m_pCHR[m_readAddr + 8];

				// Increment coarse X - 0 to 31
				if (m_PPUMASK & 0b00011000)
				{
					if ((m_VRAMAddr & 0b11111) != 0b11111)
					{
						++m_VRAMAddr;
					}
					else
					{
						// Reset, change nametable
						m_VRAMAddr &= 0b0111111111100000;
						m_VRAMAddr ^= 0b0000010000000000;
					}
				}
				break;
			}

			if (m_pixel == 256)
			{
				// Begin HBlank
				// Increment Y
				if (m_PPUMASK & 0b00011000)
				{
					if ((m_VRAMAddr & 0b0111000000000000) != 0b0111000000000000)
					{
						m_VRAMAddr += 0b0001000000000000;
					}
					else
					{
						// Reset 7 to 0
						m_VRAMAddr &= 0b0000111111111111;
						if ((m_VRAMAddr & 0b0000001111100000) == 0b0000001110100000)
						{
							// If coarse Y = 29
							m_VRAMAddr &= 0b0111110000011111;
							m_VRAMAddr ^= 0b0000100000000000;
						}
						else
						{
							m_VRAMAddr += 0b0000000000100000;
						}
					}
				}
			}
			else if (m_pixel == 257)
			{
				// Continue HBlank
				m_tilePatternShift[0] |= m_fetchPattern[0];
				m_tilePatternShift[1] |= m_fetchPattern[1];
				if (m_PPUMASK & 0b00011000)
				{
					// Set addr to temp addr - X
					m_VRAMAddr &= 0b0111101111100000;
					m_VRAMAddr += (m_tempVRAMAddr & 0b0000010000011111);
				}
			}
		}
		else if (m_pixel >= 280 && m_pixel <= 304 && m_scanline == 0)
		{
			// End of VBlank
			if (m_PPUMASK & 0b00011000)
			{
				// Set addr to temp addr - Y
				m_VRAMAddr &= 0b0000010000011111;
				m_VRAMAddr += (m_tempVRAMAddr & 0b0111101111100000);
			}
		}

		else if (m_pixel == 0 && m_scanline == 1)
		{
			// Skip the cycle at the end of the scaneline for odd frames
			++m_pixel;
		}

		// Background drawing
		if (m_PPUMASK && 0b00001000 && m_pixel >= 2 && m_pixel <= 257)
		{
			// Colour, 0 to 3
			pixelLow = (m_tilePatternShift[0] / Powers[15 - m_fineX]) & 0x1;
			pixelHigh = (m_tilePatternShift[1] / Powers[15 - m_fineX]) & 0x1;
			pixelColour = (byte)pixelHigh * 0x2 + (byte)pixelLow;

			// Palette, 0 to 7
			pixelLow = (m_tileAttrShift[0] / Powers[7 - m_fineX]) & 0x1;
			pixelHigh = (m_tileAttrShift[1] / Powers[7 - m_fineX]) & 0x1;
			pixelPalette = (byte)pixelHigh * 0x2 + (byte)pixelLow;

			DrawPixel(pixelPalette, pixelColour, m_pixel-2, m_scanline-1);
		}
	}

	// End of frame
	// VBlank bit set
	// Sprite drawing
	else if (m_pixel == 1 && m_scanline == 242)
	{
		m_PPUSTATUS |= 0x80;
		m_isNMI = m_PPUCTRL & 0x80 ? true : m_isNMI;

		for (unsigned int i = 0; i < 64; ++i)
		{
			DrawTile(m_OAM[4 * i]-1, m_OAM[4 * i + 1], m_OAM[4 * i + 2], m_OAM[4 * i + 3]);
		}
	}

	// Coordinate looping
	if (m_pixel == 340)
	{
		// End of scanline + HBlank
		if (m_scanline == 261)
		{
			// End of screen + VBlank + In-between scanline
			m_scanline = -1;
		}
		++m_scanline;

		m_pixel = -1;
	}
	++m_pixel;

	// OAM DMA
	if (m_totalCycles % 3 == 0)
	{
		if (m_isDMA)
		{
			if (m_isEvenCycle)
			{
				// Wait for next even cycle
				if ((m_totalCycles & 0x1) == 1)
				{
					// If odd
					m_isEvenCycle = !m_isEvenCycle;
				}
			}
			else
			{
				if (!(m_totalCycles & 0x1))
				{
					// If even
					m_DMAValue = m_pNES->Read( (word)m_DMAPage * 0x100 + (word)m_DMAAddr );
				}
				else
				{
					// Do DMA
					m_OAM[m_DMAAddr] = m_DMAValue;

					++m_DMAAddr;
					if (!m_DMAAddr)
					{
						// Full 256 writes, overflows back to zero
						m_isEvenCycle = true;
						m_isDMA = false;
					}
				}
			}
		}
		else
		{
			return true;
		}
	}

	return false;
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
		m_prevPPUDATARead = ReadFromPPU(m_VRAMAddr);
		if (m_VRAMAddr >= 0x3f00)
		{
			temp = m_prevPPUDATARead;
		}

		// Increment mode
		m_VRAMAddr = (0b00000100 & m_PPUCTRL ? m_VRAMAddr + 0x20 : m_VRAMAddr + 0x1);

		return temp;
	}

	return 0x0;
}

void PPU::Write(byte value, word addr)
{
	if (addr == 0x4014)
	{
		// OAM DMA
		m_isDMA = true;
		m_DMAPage = value;
		m_DMAAddr = 0x0;
	}
	else
	{
		word mask;
		addr &= 0x7;
		switch (addr)
		{
		case 0x0:
			// PPU CTRL
			m_tempVRAMAddr &= 0b0111001111111111;
			m_tempVRAMAddr |= (m_PPUCTRL & 0b11);
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
			if (m_latch)
			{
				// then Y scroll
				m_tempVRAMAddr &= 0b1000110000011111;
				m_tempVRAMAddr += ((value & 0b11111000) * 4);
				m_tempVRAMAddr += ((value & 0b00000111) * 4096);
			}
			else
			{
				// X scroll
				m_tempVRAMAddr &= 0b1111111111100000;
				m_tempVRAMAddr += (value / 8);
				m_fineX = value & 0b111;
			}
			m_latch = !m_latch;
			break;

		case 0x6:
			// PPU ADDR
			if (m_latch)
			{
				// then Low byte
				m_tempVRAMAddr = (m_tempVRAMAddr & 0xff00) + (word)value;
				m_VRAMAddr = m_tempVRAMAddr;
			}
			else
			{
				// High byte
				m_tempVRAMAddr = (word)(value & 0x3f) * 0x100 + (m_tempVRAMAddr & 0xff);
			}
			m_latch = !m_latch;
			break;

		case 0x7:
			// PPU DATA
			WriteToPPU(value, m_VRAMAddr);

			// Increment mode
			m_VRAMAddr = (0b00000100 & m_PPUCTRL ? m_VRAMAddr + 0x20 : m_VRAMAddr + 0x1);
			break;
		}
	}
}

byte PPU::ReadFromPPU(word addr)
{
	if (addr < 0x2000)
	{
		// Pattern tables
		return m_pCHR[addr];
	}
	else if (addr < 0x3f00)
	{
		// Nametables
		addr &= 0xfff;
		return m_Nametables[Tables[m_tableIndex + (addr / 0x400)]][addr & 0x3ff];
	}
	else if (addr < 0x4000)
	{
		return m_palette[addr & 0x1f];
	}
}

void PPU::WriteToPPU(byte value, word addr)
{
	if (addr < 0x2000)
	{
		m_pCHR[addr] = value;
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
		addr -= 0x2000;

		m_Nametables[tables[index + (addr / 0x400)]][addr & 0x3ff] = value;
	}
	else if (addr < 0x4000)
	{
		m_palette[addr & 0x1f] = value;
	}
}