#include "Controller.h"


Controller::Controller()
{
	m_offset[0] = 0;
	m_offset[1] = 0;
	m_isSerialMode[0] = true;
	m_isSerialMode[1] = true;
}

void Controller::Write(byte value, word addr)
{
	int controllerNumber = addr & 0b1;
	if (value & 0b1)
	{
		m_isSerialMode[controllerNumber] = false;
		const Uint8* keyState = SDL_GetKeyboardState(NULL);
		if ((addr & 0b1) == 0)
		{
			// Controller 1
			m_inputs[0] =	keyState[SDL_SCANCODE_X] + 0x2 * keyState[SDL_SCANCODE_Z] + 0x4 * keyState[SDL_SCANCODE_SPACE] + 0x8 * keyState[SDL_SCANCODE_RETURN] + 0x10 * keyState[SDL_SCANCODE_UP] + 
							0x20 * keyState[SDL_SCANCODE_DOWN] + 0x40 * keyState[SDL_SCANCODE_LEFT] + 0x80 * keyState[SDL_SCANCODE_RIGHT];
		}
		else
		{
			// Controller 2
			m_inputs[1] =	keyState[SDL_SCANCODE_L] + 0x2 * keyState[SDL_SCANCODE_K] + 0x4 * keyState[SDL_SCANCODE_O] + 0x8 * keyState[SDL_SCANCODE_P] + 0x10 * keyState[SDL_SCANCODE_W] +
							0x20 * keyState[SDL_SCANCODE_S] + 0x40 * keyState[SDL_SCANCODE_A] + 0x80 * keyState[SDL_SCANCODE_D];
		}
	}
	else if(!(value & 0b1))
	{
		m_isSerialMode[controllerNumber] = true;
	}
}

byte Controller::Read(word addr)
{
	int controllerNumber = addr & 0b1;
	if (m_isSerialMode[controllerNumber])
	{
		// Open bus nonsense, add 0x40
		byte value = (bool)(m_inputs[controllerNumber] & Masks[m_offset[controllerNumber]]) + 0x40;
		m_offset[controllerNumber] = (m_offset[controllerNumber] == 7 ? 0 : m_offset[controllerNumber] + 1);

		return value;
	}
}