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
	//m_spritesOnLine = 0x0;
	m_isDMA = false;
	m_isEvenCycle = true;
	m_DMAPage = 0x0;
	m_DMAValue = 0x0;
	m_DMAAddr = 0x0;

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

void PPU::DrawTile(int index, int palette, int x, int y)
{
	//// Prepare all 4 possible colours
	//uint32_t RGBColours[4];
	//RGBColours[0] = mFullNESPalette[m_palette[0]];
	//RGBColours[1] = mFullNESPalette[m_palette[4 * palette + 1]];
	//RGBColours[2] = mFullNESPalette[m_palette[4 * palette + 2]];
	//RGBColours[3] = mFullNESPalette[m_palette[4 * palette + 3]];

	//bool lowBit, highBit;
	//byte colourInPalette;
	//for (unsigned int i = 0; i < 8; ++i)
	//{
	//	for (unsigned int j = 0; j < 8; ++j)
	//	{
	//		int offsetX = 0, offsetY = 0;

	//		// For 2 nametables
	//		/*if ((j + x) < -SCREEN_WIDTH)
	//			offsetX = SCREEN_WIDTH * 2;
	//		if ((i + y) < -SCREEN_HEIGHT)
	//			offsetY = SCREEN_HEIGHT * 2;*/

	//		// Prevent drawing outside array
	//		if ((j + x) >= SCREEN_WIDTH || ((i + y) >= SCREEN_HEIGHT || ((j + x) < 0 && (j + x) >= -SCREEN_WIDTH) || ((i + y) < 0 && (i + y) >= -SCREEN_HEIGHT)))
	//			continue;

	//		colourInPalette = m_pCHR[index].colours[(8 * i) + j];
	//		m_screenPixels[(j + x + offsetX) + (SCREEN_WIDTH * (i + y + offsetY))] = RGBColours[colourInPalette];
	//	}
	//}
}

void PPU::DrawPixel(int palette, int colour, int x, int y)
{
	// Prevent drawing outside array
	if (x >= SCREEN_WIDTH || (y >= SCREEN_HEIGHT || (x < 0 && x >= -SCREEN_WIDTH) || (y < 0 && y >= -SCREEN_HEIGHT)))
		return;

	m_screenPixels[x + (SCREEN_WIDTH * y)] = mFullNESPalette[m_palette[4 * palette + colour]];
}

void PPU::Render(SDL_Renderer* renderer)
{
	SDL_UpdateTexture(m_pFrameBuffer, NULL, m_screenPixels, SCREEN_WIDTH * sizeof(uint32_t));
	SDL_RenderCopy(renderer, m_pFrameBuffer, NULL, NULL);

	//for (unsigned int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
	//{
	//	m_screenPixels[i] = mFullNESPalette[m_palette[0]];
	//}

	// Test - draw every tile in CHR
	/*for (unsigned int i = 0; i < SCREEN_HEIGHT/8; ++i)
	{
		for (unsigned int j = 0; j < SCREEN_WIDTH/8; ++j)
		{
			DrawTile(i * 8 + j, (i * 8 + j) % 8, m_scale, j*8, i*8);
		}
	}*/

	// Test - Draw nametable 0 + sprites
	/*for (unsigned int i = 0; i < SCREEN_HEIGHT / 8; ++i)
	{
		for (unsigned int j = 0; j < SCREEN_WIDTH / 8; ++j)
		{
			int index = i * (SCREEN_WIDTH / 8) + j;
			byte attrOffset = m_Nametables[0][(j & 0b11111100) / 4 + 2 * (i & 0b11111100) + 0x3c0];
			byte mask = (attrOffset & Masks[index % 4]) / Factors[index % 4];

			DrawTile(m_Nametables[0][index] + 256, mask, j*8, i*8);
		}
	}
	for (unsigned int i = 0; i < 64; ++i)
	{
		DrawTile(m_OAM[4 * i + 1], (m_OAM[4 * i + 2] & 0x3) + 0x4, m_OAM[4 * i + 3], m_OAM[4 * i]);
	}*/
}

bool PPU::Update(unsigned long long int m_totalCycles)
{
	// Scanline 0 - before screen
	// Scanlines 1 to 240 - screen
	// Scanlines 241 to 261 - VBlank
	if (m_scanline >= 0 && m_scanline <= 240)
	{
		if (m_pixel == 1 && m_scanline == 0)
		{
			// Initialise frame
			for (unsigned int i = 0; i < 8; ++i)
			{

			}
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
			if ((m_PPUMASK && 0b00010000) && m_pixel >= 1 && m_pixel <= 257)
			{
				// Sprite shifters
				for (unsigned int i = 0; i < 8; ++i)
				{
					// X pos
					if (m_secondaryOAM[4 * i + 3] != 0)
					{
						// Sprite is valid
						--m_secondaryOAM[4 * i + 3];
					}
					else
					{
						m_spritePatternShift[0][i] *= 2;
						m_spritePatternShift[1][i] *= 2;
					}
				}
			}

			const int index = m_isVerticalMirroring ? 0 : 4;
			word addr;
			byte arrayAddr, colourAddr;
			switch ((m_pixel - 1) % 8)
			{
			case 0:
				// Load background shifters
				m_tilePatternShift[0] &= 0xff00;
				m_tilePatternShift[1] &= 0xff00;
				m_tilePatternShift[0] += m_fetchPattern[0];
				m_tilePatternShift[1] += m_fetchPattern[1];

				// Tile in nametable
				addr = m_VRAMAddr & 0x0fff;
				if (addr < 0x400)
				{
					m_fetchNametable = m_Nametables[tables[index]][addr];
				}
				else if (addr < 0x800)
				{
					m_fetchNametable = m_Nametables[tables[index + 1]][addr & 0x3ff];
				}
				else if (addr < 0xc00)
				{
					m_fetchNametable = m_Nametables[tables[index + 2]][addr & 0x3ff];
				}
				else if (addr < 0x1000)
				{
					m_fetchNametable = m_Nametables[tables[index + 3]][addr & 0x3ff];
				}
				break;

			case 2:
				// Palette in nametable
				addr = 0x3c0 + ((m_VRAMAddr & 0b0000000000011111) / 4) + (m_VRAMAddr & 0b0000110000000000) + ((m_VRAMAddr & 0b0000001110000000) / 16);
				if (addr < 0x400)
				{
					m_fetchAttr = m_Nametables[tables[index]][addr];
				}
				else if (addr < 0x800)
				{
					m_fetchAttr = m_Nametables[tables[index + 1]][addr & 0x3ff];
				}
				else if (addr < 0xc00)
				{
					m_fetchAttr = m_Nametables[tables[index + 2]][addr & 0x3ff];
				}
				else if (addr < 0x1000)
				{
					m_fetchAttr = m_Nametables[tables[index + 3]][addr & 0x3ff];
				}

				// Find tile in 2x2 set
				m_fetchAttr /= Powers[( 4 * (int)(m_VRAMAddr && 0b0000000001000000) + 2 * (int)(m_VRAMAddr && 0b0000000000000010))];
				break;

			case 4:
				// Graphics from CHR
				addr = ((word)(m_PPUCTRL && 0b00010000) * 0x1000);
				addr = ((word)(m_PPUCTRL && 0b00010000) * 0x1000) + (word)(m_fetchNametable * 16) + ((m_VRAMAddr & 0b0111000000000000) / 4096);

				// Pattern tables
				/*arrayAddr = addr / 0x10;
				colourAddr = addr - arrayAddr;
				m_fetchPattern[0] = m_pCHR[arrayAddr].colours[4 * colourAddr] + 4 * m_pCHR[arrayAddr].colours[4 * colourAddr + 1] + 16 * m_pCHR[arrayAddr].colours[4 * colourAddr + 2] + 64 * m_pCHR[arrayAddr].colours[4 * colourAddr + 3];
				addr += 8;
				arrayAddr = addr / 0x10;
				colourAddr = addr - arrayAddr;
				m_fetchPattern[1] = m_pCHR[arrayAddr].colours[4 * colourAddr] + 4 * m_pCHR[arrayAddr].colours[4 * colourAddr + 1] + 16 * m_pCHR[arrayAddr].colours[4 * colourAddr + 2] + 64 * m_pCHR[arrayAddr].colours[4 * colourAddr + 3];*/
				m_fetchPattern[0] = m_pCHR[addr];
				m_fetchPattern[1] = m_pCHR[addr + 8];
				break;

			case 6:
				// Increment coarse X - 0 to 31
				if (m_PPUMASK & 0b00011000)
				{
					byte coarseX = m_VRAMAddr & 0b11111;
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
							// TODO - add case for = 31?
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
				m_tilePatternShift[0] &= 0xff00;
				m_tilePatternShift[1] &= 0xff00;
				m_tilePatternShift[0] += m_fetchPattern[0];
				m_tilePatternShift[1] += m_fetchPattern[1];
				if (m_PPUMASK & 0b00011000)
				{
					// Set addr to temp addr
					m_VRAMAddr &= 0b0111101111100000;
					m_VRAMAddr += (m_tempVRAMAddr & 0b0000010000011111);
				}

				if (m_scanline >= 1)
				{
					// Sprite evaluation

				}
			}
		}
		else if (m_pixel == 340)
		{
			// End of scanline
			// Prepare sprites
		}
		else if (m_pixel >= 280 && m_pixel <= 304 && m_scanline == 0)
		{
			// End of VBlank
			if (m_PPUMASK & 0b00011000)
			{
				// Set addr to temp addr
				m_VRAMAddr &= 0b0000010000011111;
				m_VRAMAddr += (m_tempVRAMAddr & 0b0111101111100000);
			}
		}

		else if (m_pixel == 0 && m_scanline == 1)
		{
			// Skip the cycle at the end of the scaneline for odd frames
			++m_pixel;
		}
	}
	else if (m_pixel == 1 && m_scanline == 242)
	{
		// End of frame
		// VBlank bit set
		m_PPUSTATUS |= 0x80;
		m_isNMI = m_PPUCTRL & 0x80 ? true : m_isNMI;
	}
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

	// Background drawing
	byte pixelColour;
	byte pixelPalette;
	if (m_PPUMASK && 0b00001000)
	{
		// Colour, 0 to 3
		bool pixelLow = ( m_tilePatternShift[0] / Powers[15 - m_fineX] ) & 0x1;
		bool pixelHigh = ( m_tilePatternShift[1] / Powers[15 - m_fineX] ) & 0x1;
		pixelColour = (byte)pixelHigh * 0x2 + (byte)pixelLow;

		// Palette, 0 to 7
		pixelLow = ( m_tileAttrShift[0] / Powers[7 - m_fineX] ) & 0x1;
		pixelHigh = ( m_tileAttrShift[1] / Powers[7 - m_fineX] ) & 0x1;
		pixelPalette = (byte)pixelHigh * 0x2 + (byte)pixelLow;

		DrawPixel(pixelPalette, pixelColour, m_pixel, m_scanline);
	}

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
				if ((m_totalCycles & 0x1) == 0)
				{
					// If even
					m_DMAValue = m_pNES->Read( (word)m_DMAPage * 0x100 + (word)m_DMAAddr );
				}
				else
				{
					// Do DMA
					m_OAM[m_DMAAddr] = m_DMAValue;

					++m_DMAAddr;
					if (m_DMAAddr == 0x0)
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
			//m_tempVRAMAddr &= 0b1111001111111111;
			mask = ~((m_PPUCTRL & 0x3) * 0b10000000000);
			m_tempVRAMAddr &= mask;
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
				// Y scroll
				//m_tempVRAMAddr.coarseY = value / 8;
				//m_tempVRAMAddr.fineY = value & 0x7;
				mask = ~((value / 8) * 0b100000);
				m_tempVRAMAddr &= mask;
				mask = ~((value & 0x7) * 0b1000000000000);
				m_tempVRAMAddr &= mask;
			}
			else
			{
				// X scroll
				//m_tempVRAMAddr.coarseX = value / 8;
				m_tempVRAMAddr &= ~(value / 8);
				m_fineX = value & 0b111;
			}
			m_latch = !m_latch;
			break;

		case 0x6:
			// PPU ADDR
			//word temp;
			if (m_latch)
			{
				// then Low byte
				//temp = m_PPUADDR & 0xff00;
				//m_PPUADDR = temp + (word)value;
				m_tempVRAMAddr = (m_tempVRAMAddr & 0xff00) + (word)value;
				m_VRAMAddr = m_tempVRAMAddr;
			}
			else
			{
				// High byte
				//temp = (value & 0x3f) * 0x100;
				//m_PPUADDR &= 0xff;
				//m_PPUADDR += temp;
				m_tempVRAMAddr = (value & 0x3f) * 0x100 + (m_tempVRAMAddr & 0xff);

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
		/*word arrayAddr = addr / 0x10;
		byte colourAddr = addr - arrayAddr;
		return m_pCHR[arrayAddr].colours[4 * colourAddr] + 4 * m_pCHR[arrayAddr].colours[4 * colourAddr + 1] + 16 * m_pCHR[arrayAddr].colours[4 * colourAddr + 2] + 64 * m_pCHR[arrayAddr].colours[4 * colourAddr + 3];*/
		return m_pCHR[addr];
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
		const int index = m_isVerticalMirroring ? 0 : 4;

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
			return m_Nametables[tables[index+2]][addr & 0x3ff];
		}
		else if (addr < 0x3000)
		{
			return m_Nametables[tables[index+3]][addr & 0x3ff];
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
			m_Nametables[tables[index + 2]][addr & 0x3ff] = value;
		}
		else if (addr < 0x3000)
		{
			m_Nametables[tables[index + 3]][addr & 0x3ff] = value;
		}
	}
	else if (addr < 0x4000)
	{
		m_palette[addr & 0x1f] = value;
	}
}