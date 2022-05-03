#include "NES.h"


NES::NES(SDL_Renderer* renderer)
{
	m_pROM = new ROM();
	m_pROM->LoadROM();

	m_pCPU = new CPU(this);
	m_pRAM = new RAM();
	m_pDebug = new Debug();
	m_pPPU = new PPU(renderer, m_pROM->GetCHR(), this);
	m_pPPU->SetMirroring(m_pROM->GetMirroring());

	m_timer = 0;
	m_pSurface = nullptr;
	m_pTextTexture = nullptr;
	m_pFont = TTF_OpenFont("arial.ttf", 20);

	m_pCtrl = new Controller();
}

NES::~NES()
{
	delete m_pCPU;
	delete m_pRAM;
	delete m_pDebug;
	m_pSurface = nullptr;
	delete m_pSurface;
	m_pTextTexture = nullptr;
	delete m_pTextTexture;
	TTF_CloseFont(m_pFont);
}

bool NES::Update(SDL_Renderer* renderer, float deltaTime)
{
	m_timer += deltaTime;

	if (m_pPPU->Update(m_totalCycles))
	{
		m_pCPU->Update();
		//m_pDebug->OutputLine(m_pROM, m_pCPU);
	}
	if (m_pPPU->GetNMI())
	{
		m_pCPU->NMI();
		m_pPPU->NMIOff();
		Render(renderer);
		m_storedTimer = m_timer - m_prevTimer;
		m_prevTimer = m_timer;
	}

	//if (m_timer >= 1)
	/*if (m_timer >= 0.01667f)
	{
		m_prevOutputCycles = m_totalCycles;
		Render(renderer);
		m_timer = 0;
	}*/

	++m_totalCycles;

	return false;
}

void NES::Render(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(renderer);

	m_pPPU->Render(renderer);

	oss.str(std::string());
	oss << 1 / m_storedTimer;
	DrawText(renderer, Vec2D( 4 * m_pPPU->GetScale(), 4 * m_pPPU->GetScale() ), (std::string("FPS: ") + oss.str()).c_str());

	SDL_RenderPresent(renderer);
}

byte NES::Read(word addr)
{
	if (addr < 0x2000)
	{
		// RAM
		return m_pRAM->Read(addr);
	}
	else if (addr < 0x4000)
	{
		// PPU registers
		return m_pPPU->Read(addr);
	}
	else if (addr < 0x4018)
	{
		// APU + controller registers
		if (addr == 0x4016 || addr == 0x4017)
		{
			// Controller poll
			return m_pCtrl->Read(addr);
		}
	}
	else
	{
		// ROM
		return m_pROM->Read(addr);
	}
}

void NES::Write(byte value, word addr)
{
	if (addr < 0x2000)
	{
		// RAM
		m_pRAM->Write(value, addr);
	}
	else if (addr < 0x4000)
	{
		// PPU registers
		m_pPPU->Write(value, addr);
	}
	else if (addr < 0x4018)
	{
		// APU + controller registers
		if (addr == 0x4014)
		{
			// OAM DMA
			m_pPPU->Write(value, addr);
		}
		else if (addr == 0x4016 || addr == 0x4017)
		{
			// Controller poll
			m_pCtrl->Write(value, addr);
		}
	}
}

void NES::DrawText(SDL_Renderer* renderer, Vec2D position, const char* text)
{
	if (!(m_pSurface = TTF_RenderText_Shaded(m_pFont, text, { 0xff, 0xff, 0x00 }, { 0, 0, 0 })))
		printf("Text surface error.\n");

	m_pTextTexture = SDL_CreateTextureFromSurface(renderer, m_pSurface);

	int texW = 0, texH = 0;
	SDL_QueryTexture(m_pTextTexture, NULL, NULL, &texW, &texH);
	SDL_Rect textRect = { position.x, position.y, texW, texH };
	SDL_RenderCopy(renderer, m_pTextTexture, NULL, &textRect);
	SDL_DestroyTexture(m_pTextTexture);
	SDL_FreeSurface(m_pSurface);
}