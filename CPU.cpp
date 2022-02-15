#include "CPU.h"
#include "NES.h"


CPU::CPU(NES* parentNES)
{
	m_pNES = parentNES;

	RESET();
}

CPU::~CPU()
{
	m_pNES = nullptr;
}

void CPU::NMI()
{
	PCToStack();

	m_statusFlags[FLAG_INTERRUPT_DISABLE] = true;
	m_statusFlags[FLAG_BREAK] = false;
	FlagsToStack();

	m_cycle = 8;

	// Hard coded pointer to NMI at 0xfffa
	byte lowByte = m_pNES->Read(0xfffa);
	byte highByte = m_pNES->Read(0xfffb);
	m_programCounter = CombineAddrBytes(highByte, lowByte);
}

void CPU::RESET()
{
	m_A = 0x0;
	m_X = 0x0;
	m_Y = 0x0;
	m_stackPointer = 0xfd;

	m_statusFlags[FLAG_CARRY] = false;
	m_statusFlags[FLAG_ZERO] = false;
	m_statusFlags[FLAG_INTERRUPT_DISABLE] = true;
	m_statusFlags[FLAG_DECIMAL] = false;
	m_statusFlags[FLAG_BREAK] = false;
	m_statusFlags[FLAG_UNUSED] = true;
	m_statusFlags[FLAG_OVERFLOW] = false;
	m_statusFlags[FLAG_NEGATIVE] = false;

	m_cycle = 8;

	// Hard coded pointer to RESET at 0xfffc
	byte lowByte = m_pNES->Read(0xfffc);
	byte highByte = m_pNES->Read(0xfffd);
	m_programCounter = CombineAddrBytes(highByte, lowByte);
}

void CPU::IRQ()
{
	if (!m_statusFlags[FLAG_INTERRUPT_DISABLE])
	{
		PCToStack();

		m_statusFlags[FLAG_INTERRUPT_DISABLE] = true;
		m_statusFlags[FLAG_BREAK] = false;
		FlagsToStack();

		m_cycle = 7;

		// Hard coded pointer to IRQ at 0xfffe
		byte lowByte = m_pNES->Read(0xfffe);
		byte highByte = m_pNES->Read(0xffff);
		m_programCounter = CombineAddrBytes(highByte, lowByte);
	}
}

bool CPU::Update()
{
	if (!m_cycle)
	{
		// Get the next opcode
		byte opcode = m_pNES->Read(m_programCounter);

		// Temp - end program
		if (opcode == 0xff)
		{
			return true;
		}

		m_cycle = OpcodeTimings[opcode];
		++m_programCounter;

		// Retrieves the byte of data used by the opcode, using the appropriate addressing mode
		word addr = 0x0;
		byte value = 0x0;
		m_hasCrossedBoundary = false;
		switch (OpcodeAddrTypes[opcode])
		{
		case ADDR_IMP:
			// Nothing, no value is used
			break;
		case ADDR_ACC:
			value = m_A;
			break;
		case ADDR_IMM:
			value = AddrImmediate();
			break;
		case ADDR_ZER:
			addr = AddrZero();
			break;
		case ADDR_ZEX:
			addr = AddrZeroX();
			break;
		case ADDR_ZEY:
			addr = AddrZeroY();
			break;
		case ADDR_REL:
			value = AddrRelative();
			break;
		case ADDR_ABS:
			addr = AddrAbsolute();
			break;
		case ADDR_ABX:
			addr = AddrAbsoluteX();
			break;
		case ADDR_ABY:
			addr = AddrAbsoluteY();
			break;
		case ADDR_IND:
			addr = AddrIndirect();
			break;
		case ADDR_INX:
			addr = AddrIndirectX();
			break;
		case ADDR_INY:
			addr = AddrIndirectY();
			break;
		case ADDR_NON:
			// Should only happen for unofficial opcodes
			assert(OpcodeAddrTypes[opcode] != ADDR_NON);
			break;
		}

		if (OpcodeAddrTypes[opcode] != ADDR_IMP && OpcodeAddrTypes[opcode] != ADDR_ACC && OpcodeAddrTypes[opcode] != ADDR_IMM && OpcodeAddrTypes[opcode] != ADDR_REL)
		{
			value = m_pNES->Read(addr);
		}

		// Perform opcode using the retrieved byte
		switch (OpcodeTypes[opcode])
		{
		case OP_ADC:
			ADC(value, opcode);
			break;
		case OP_AND:
			AND(value);
			break;
		case OP_ASL:
			ASL(value, opcode, addr);
			break;
		case OP_BCC:
			BCC(value);
			break;
		case OP_BCS:
			BCS(value);
			break;
		case OP_BEQ:
			BEQ(value);
			break;
		case OP_BIT:
			BIT(value);
			break;
		case OP_BMI:
			BMI(value);
			break;
		case OP_BNE:
			BNE(value);
			break;
		case OP_BPL:
			BPL(value);
			break;
		case OP_BRK:
			BRK();
			break;
		case OP_BVC:
			BVC(value);
			break;
		case OP_BVS:
			BVS(value);
			break;
		case OP_CLC:
			CLC();
			break;
		case OP_CLD:
			CLD();
			break;
		case OP_CLI:
			CLI();
			break;
		case OP_CLV:
			CLV();
			break;
		case OP_CMP:
			CMP(value);
			break;
		case OP_CPX:
			CPX(value);
			break;
		case OP_CPY:
			CPY(value);
			break;
		case OP_DEC:
			DEC(addr);
			break;
		case OP_DEX:
			DEX();
			break;
		case OP_DEY:
			DEY();
			break;
		case OP_EOR:
			EOR(value);
			break;
		case OP_INC:
			INC(addr);
			break;
		case OP_INX:
			INX();
			break;
		case OP_INY:
			INY();
			break;
		case OP_JMP:
			JMP(addr);
			break;
		case OP_JSR:
			JSR(addr);
			break;
		case OP_LDA:
			LDA(value);
			break;
		case OP_LDX:
			LDX(value);
			break;
		case OP_LDY:
			LDY(value);
			break;
		case OP_LSR:
			LSR(value, opcode, addr);
			break;
		case OP_NOP:
			NOP();
			break;
		case OP_ORA:
			ORA(value);
			break;
		case OP_PHA:
			PHA();
			break;
		case OP_PHP:
			PHP();
			break;
		case OP_PLA:
			PLA();
			break;
		case OP_PLP:
			PLP();
			break;
		case OP_ROL:
			ROL(value, opcode, addr);
			break;
		case OP_ROR:
			ROR(value, opcode, addr);
			break;
		case OP_RTI:
			RTI();
			break;
		case OP_RTS:
			RTS();
			break;
		case OP_SBC:
			SBC(value);
			break;
		case OP_SEC:
			SEC();
			break;
		case OP_SED:
			SED();
			break;
		case OP_SEI:
			SEI();
			break;
		case OP_STA:
			STA(addr);
			break;
		case OP_STX:
			STX(addr);
			break;
		case OP_STY:
			STY(addr);
			break;
		case OP_TAX:
			TAX();
			break;
		case OP_TAY:
			TAY();
			break;
		case OP_TSX:
			TSX();
			break;
		case OP_TXA:
			TXA();
			break;
		case OP_TXS:
			TXS();
			break;
		case OP_TYA:
			TYA();
			break;
		case OP_NON:
			assert(OpcodeTypes[opcode] != OP_NON);
			break;

		}
	}

	--m_cycle;

	return false;
}

byte CPU::AddrImmediate()
{
	// Get byte after the opcode
	byte value = m_pNES->Read(m_programCounter);
	++m_programCounter;

	return value;
}

byte CPU::AddrZero()
{
	// Get byte after the opcode
	byte addr = m_pNES->Read(m_programCounter);

	++m_programCounter;

	// Use this byte as a zero page address
	return addr;
}

byte CPU::AddrZeroX()
{
	// Offset address by X - using a byte data type will make the address wrap around as expected
	byte addr = m_pNES->Read(m_programCounter) + m_X;

	++m_programCounter;

	// Use this byte as a zero page address
	return addr;
}

byte CPU::AddrZeroY()
{
	// Offset address by Y - using a byte data type will make the address wrap around as expected
	byte addr = m_pNES->Read(m_programCounter) + m_Y;

	++m_programCounter;

	// Use this byte as a zero page address
	return addr;
}

byte CPU::AddrRelative()
{
	// Get byte after the opcode
	byte relAddr = m_pNES->Read(m_programCounter);
	++m_programCounter;

	return relAddr;
}

word CPU::AddrAbsolute()
{
	// Get byte after the opcode
	byte addrLow = m_pNES->Read(m_programCounter);

	// Get next byte
	++m_programCounter;
	byte addrHigh = m_pNES->Read(m_programCounter);

	++m_programCounter;

	// Combine for actual address
	return CombineAddrBytes(addrHigh, addrLow);
}

word CPU::AddrAbsoluteX()
{
	// Get byte after the opcode
	byte addrLow = m_pNES->Read(m_programCounter);
	++m_programCounter;

	// Get next byte
	byte addrHigh = m_pNES->Read(m_programCounter);
	++m_programCounter;

	// Combine for actual address
	word valueAddr = CombineAddrBytes(addrHigh, addrLow) + m_X;

	// Extra cycle if crossing boundary
	m_hasCrossedBoundary = (valueAddr / 0x100 != addrHigh ? false : true);

	return valueAddr;
}

word CPU::AddrAbsoluteY()
{
	// Get byte after the opcode
	byte addrLow = m_pNES->Read(m_programCounter);
	++m_programCounter;

	// Get next byte
	byte addrHigh = m_pNES->Read(m_programCounter);
	++m_programCounter;

	// Combine for actual address
	word valueAddr = CombineAddrBytes(addrHigh, addrLow) + m_Y;

	// Extra cycle if crossing boundary
	m_hasCrossedBoundary = (valueAddr / 0x100 != addrHigh ? false : true);

	return valueAddr;
}

word CPU::AddrIndirect()
{
	// Get byte after the opcode
	word addrPointerLow = m_pNES->Read(m_programCounter);

	// Get next byte
	++m_programCounter;
	word addrPointerHigh = m_pNES->Read(m_programCounter);

	// Combine for pointer
	word addrPointer = CombineAddrBytes(addrPointerHigh, addrPointerLow);

	++m_programCounter;

	// Get final address from pointer destination
	word addrLow = m_pNES->Read(addrPointer);
	word addrHigh;
	if (addrPointerLow == 0xff)
	{
		// Emulated bug in the original 6502
		// When crossing page boundary, reads the high byte from the same page instead of the next page
		addrHigh = m_pNES->Read(addrPointer + 1 - 0x100);
	}
	else
	{
		addrHigh = m_pNES->Read(addrPointer + 1);
	}

	return CombineAddrBytes(addrHigh, addrLow);
}

word CPU::AddrIndirectX()
{
	// Get pointer
	byte addrPointer = m_pNES->Read(m_programCounter) + m_X;

	++m_programCounter;

	// Get address to read from
	word addrLow = m_pNES->Read(addrPointer);
	word addrHigh = m_pNES->Read(addrPointer + 1);

	return CombineAddrBytes(addrHigh, addrLow);
}

word CPU::AddrIndirectY()
{
	// Get pointer
	byte addrPointer = m_pNES->Read(m_programCounter);
	++m_programCounter;

	word addrLow = m_pNES->Read(addrPointer);
	word addrHigh = m_pNES->Read(addrPointer + 1);

	word valueAddr = CombineAddrBytes(addrHigh, addrLow) + m_Y;

	// Extra cycle if crossing boundary
	m_hasCrossedBoundary = (valueAddr / 0x100 != addrHigh ? false : true);

	return valueAddr;
}

void CPU::Branch(byte value)
{
	word previousCounter = m_programCounter;
	if (value & 0x80)
	{
		byte reverseValue = 0x100 - value;
		m_programCounter -= reverseValue;
	}
	else
	{
		m_programCounter += value;
	}

	// 2 cycles if no branch - default
	// 3 cycles if normal branch
	// 4 cycles if crossing page
	if (!( (m_programCounter & 0xff00) != (previousCounter & 0xff00) ))
	{
		++m_cycle;
	}
	else
	{
		// If page crossed
		m_cycle += 2;
	}
}

void CPU::PCFromStack()
{
	++m_stackPointer;
	byte lowByte = m_pNES->Read(0x100 + m_stackPointer);
	++m_stackPointer;
	byte highByte = m_pNES->Read(0x100 + m_stackPointer);

	m_programCounter = (word)highByte * 0x100 + (word)lowByte;
}

void CPU::PCToStack()
{
	byte lowByte = m_programCounter & 0xff;
	byte highByte = (m_programCounter - lowByte) / 0x100;

	m_pNES->Write(highByte, 0x100 + m_stackPointer);
	--m_stackPointer;
	m_pNES->Write(lowByte, 0x100 + m_stackPointer);
	--m_stackPointer;
}

void CPU::FlagsToStack()
{
	byte status =	0x1 * (byte)m_statusFlags[FLAG_CARRY] + 0x2 * (byte)m_statusFlags[FLAG_ZERO] + 0x4 * (byte)m_statusFlags[FLAG_INTERRUPT_DISABLE] + 0x8 * (byte)m_statusFlags[FLAG_DECIMAL]
					+ 0x10 * (byte)m_statusFlags[FLAG_BREAK] + 0x20 * (byte)m_statusFlags[FLAG_UNUSED] + 0x40 * (byte)m_statusFlags[FLAG_OVERFLOW] + 0x80 * (byte)m_statusFlags[FLAG_NEGATIVE];

	// Possibly - clear the B and U flags?

	m_pNES->Write(status, 0x100 + m_stackPointer);
	--m_stackPointer;
}

word CPU::CombineAddrBytes(byte high, byte low)
{
	return ( (word)high * 0x100 + (word)low );
}

// Overflow logic from stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
void CPU::ADC(byte value, byte opcode)
{
	word tempA = m_A + value + (byte)m_statusFlags[FLAG_CARRY];

	m_statusFlags[FLAG_CARRY] = (tempA & 0xff00 ? true : false);
	m_statusFlags[FLAG_OVERFLOW] = ((~(m_A ^ value)) & (m_A ^ tempA) & 0x80 ? true : false);

	m_A = tempA & 0xff;

	m_statusFlags[FLAG_ZERO] = (m_A == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_A & 0x80 ? true : false);

	// Add cycle if boundary is crossed
	m_cycle += (byte)m_hasCrossedBoundary;
}

void CPU::AND(byte value)
{
	m_A &= value;

	m_statusFlags[FLAG_ZERO] = (m_A == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_A & 0x80 ? true : false);

	// Add cycle if boundary is crossed
	m_cycle += (byte)m_hasCrossedBoundary;
}

void CPU::ASL(byte value, byte opcode, word addr)
{
	m_statusFlags[FLAG_CARRY] = (value & 0x80 ? true : false);

	// When done in 8 bit space this will produce the correct result
	value *= 2;

	m_statusFlags[FLAG_ZERO] = (value == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (value & 0x80 ? true : false);

	if (OpcodeAddrTypes[opcode] == ADDR_ACC)
	{
		m_A = value;
	}
	else
	{
		m_pNES->Write(value, addr);
	}
}

void CPU::BCC(byte value)
{
	if (!m_statusFlags[FLAG_CARRY])
	{
		Branch(value);
	}
}

void CPU::BCS(byte value)
{
	if (m_statusFlags[FLAG_CARRY])
	{
		Branch(value);
	}
}

void CPU::BEQ(byte value)
{
	if (m_statusFlags[FLAG_ZERO])
	{
		Branch(value);
	}
}

void CPU::BIT(byte value)
{
	m_statusFlags[FLAG_OVERFLOW] = (value & 0x40 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (value & 0x80 ? true : false);

	value &= m_A;

	m_statusFlags[FLAG_ZERO] = (value == 0x0 ? true : false);
}

void CPU::BMI(byte value)
{
	if (m_statusFlags[FLAG_NEGATIVE])
	{
		Branch(value);
	}
}

void CPU::BNE(byte value)
{
	if (!m_statusFlags[FLAG_ZERO])
	{
		Branch(value);
	}
}

void CPU::BPL(byte value)
{
	if (!m_statusFlags[FLAG_NEGATIVE])
	{
		Branch(value);
	}
}

void CPU::BRK()
{
	// Skip padding byte
	++m_programCounter;
	PCToStack();

	m_statusFlags[FLAG_INTERRUPT_DISABLE] = true;
	m_statusFlags[FLAG_BREAK] = true;
	FlagsToStack();

	// Hard coded pointer to IRQ at 0xfffe
	byte lowByte = m_pNES->Read(0xfffe);
	byte highByte = m_pNES->Read(0xffff);
	m_programCounter = CombineAddrBytes(highByte, lowByte);
}

void CPU::BVC(byte value)
{
	if (!m_statusFlags[FLAG_OVERFLOW])
	{
		Branch(value);
	}
}

void CPU::BVS(byte value)
{
	if (m_statusFlags[FLAG_OVERFLOW])
	{
		Branch(value);
	}
}

void CPU::CLC()
{
	m_statusFlags[FLAG_CARRY] = false;
}

void CPU::CLD()
{
	m_statusFlags[FLAG_DECIMAL] = false;
}

void CPU::CLI()
{
	m_statusFlags[FLAG_INTERRUPT_DISABLE] = false;
}

void CPU::CLV()
{
	m_statusFlags[FLAG_OVERFLOW] = false;
}

void CPU::CMP(byte value)
{
	m_statusFlags[FLAG_CARRY] = (m_A >= value ? true : false);
	m_statusFlags[FLAG_ZERO] = (m_A == value ? true : false);

	byte tempA = m_A - value;

	m_statusFlags[FLAG_NEGATIVE] = (tempA & 0x80 ? true : false);

	// Add cycle if boundary is crossed
	m_cycle += (byte)m_hasCrossedBoundary;
}

void CPU::CPX(byte value)
{
	m_statusFlags[FLAG_CARRY] = (m_X >= value ? true : false);
	m_statusFlags[FLAG_ZERO] = (m_X == value ? true : false);

	byte tempX = m_X - value;

	m_statusFlags[FLAG_NEGATIVE] = (tempX & 0x80 ? true : false);
}

void CPU::CPY(byte value)
{
	m_statusFlags[FLAG_CARRY] = (m_Y >= value ? true : false);
	m_statusFlags[FLAG_ZERO] = (m_Y == value ? true : false);

	byte tempY = m_Y - value;

	m_statusFlags[FLAG_NEGATIVE] = (tempY & 0x80 ? true : false);
}

void CPU::DEC(word addr)
{
	byte value = m_pNES->Read(addr);
	--value;
	m_pNES->Write(value, addr);

	m_statusFlags[FLAG_ZERO] = (m_pNES->Read(addr) == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_pNES->Read(addr) & 0x80 ? true : false);
}

void CPU::DEX()
{
	--m_X;

	m_statusFlags[FLAG_ZERO] = (m_X == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_X & 0x80 ? true : false);
}

void CPU::DEY()
{
	--m_Y;

	m_statusFlags[FLAG_ZERO] = (m_Y == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_Y & 0x80 ? true : false);
}

void CPU::EOR(byte value)
{
	m_A ^= value;

	m_statusFlags[FLAG_ZERO] = (m_A == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_A & 0x80 ? true : false);

	// Add cycle if boundary is crossed
	m_cycle += (byte)m_hasCrossedBoundary;
}

void CPU::INC(word addr)
{
	byte value = m_pNES->Read(addr);
	++value;
	m_pNES->Write(value, addr);

	m_statusFlags[FLAG_ZERO] = (m_pNES->Read(addr) == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_pNES->Read(addr) & 0x80 ? true : false);
}

void CPU::INX()
{
	++m_X;

	m_statusFlags[FLAG_ZERO] = (m_X == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_X & 0x80 ? true : false);
}

void CPU::INY()
{
	++m_Y;

	m_statusFlags[FLAG_ZERO] = (m_Y == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_Y & 0x80 ? true : false);
}

void CPU::JMP(word addr)
{
	m_programCounter = addr;
}

void CPU::JSR(word addr)
{
	--m_programCounter;

	PCToStack();

	m_programCounter = addr;
}

void CPU::LDA(byte value)
{
	m_A = value;

	m_statusFlags[FLAG_ZERO] = (m_A == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_A & 0x80 ? true : false);

	// Add cycle if boundary is crossed
	m_cycle += (byte)m_hasCrossedBoundary;
}

void CPU::LDX(byte value)
{
	m_X = value;

	m_statusFlags[FLAG_ZERO] = (m_X == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_X & 0x80 ? true : false);

	// Add cycle if boundary is crossed
	m_cycle += (byte)m_hasCrossedBoundary;
}

void CPU::LDY(byte value)
{
	m_Y = value;

	m_statusFlags[FLAG_ZERO] = (m_Y == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_Y & 0x80 ? true : false);

	// Add cycle if boundary is crossed
	m_cycle += (byte)m_hasCrossedBoundary;
}

void CPU::LSR(byte value, byte opcode, word addr)
{
	m_statusFlags[FLAG_CARRY] = (value & 0x1 ? true : false);

	value / 2;

	m_statusFlags[FLAG_ZERO] = (value == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (value & 0x80 ? true : false);

	if (OpcodeAddrTypes[opcode] == ADDR_ACC)
	{
		m_A = value;
	}
	else
	{
		m_pNES->Write(value, addr);
	}
}

void CPU::NOP()
{
	// Nothing
}

void CPU::ORA(byte value)
{
	m_A |= value;

	m_statusFlags[FLAG_ZERO] = (m_A == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_A & 0x80 ? true : false);

	// Add cycle if boundary is crossed
	m_cycle += (byte)m_hasCrossedBoundary;
}

void CPU::PHA()
{
	m_pNES->Write(m_A, 0x100 + m_stackPointer);
	--m_stackPointer;
}

void CPU::PHP()
{
	m_statusFlags[FLAG_BREAK] = true;

	FlagsToStack();
}

void CPU::PLA()
{
	++m_stackPointer;
	m_A = m_pNES->Read(0x100 + m_stackPointer);

	m_statusFlags[FLAG_ZERO] = (m_A == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_A & 0x80 ? true : false);
}

void CPU::PLP()
{
	++m_stackPointer;
	byte status = m_pNES->Read(0x100 + m_stackPointer);

	m_statusFlags[FLAG_CARRY] = status & 0x1;
	m_statusFlags[FLAG_ZERO] = status & 0x2;
	m_statusFlags[FLAG_INTERRUPT_DISABLE] = status & 0x4;
	m_statusFlags[FLAG_DECIMAL] = status & 0x8;
	m_statusFlags[FLAG_BREAK] = status & 0x10;
	m_statusFlags[FLAG_UNUSED] = true;
	m_statusFlags[FLAG_OVERFLOW] = status & 0x40;
	m_statusFlags[FLAG_NEGATIVE] = status & 0x80;
}

void CPU::ROL(byte value, byte opcode, word addr)
{
	bool oldCarry = m_statusFlags[FLAG_CARRY];
	m_statusFlags[FLAG_CARRY] = (value & 0x80 ? true : false);

	value *= 2;
	// Set bit 0 to the old carry flag
	value |= (byte)oldCarry;

	m_statusFlags[FLAG_ZERO] = (value == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (value & 0x80 ? true : false);

	if (OpcodeAddrTypes[opcode] == ADDR_ACC)
	{
		m_A = value;
	}
	else
	{
		m_pNES->Write(value, addr);
	}
}

void CPU::ROR(byte value, byte opcode, word addr)
{
	byte oldCarry = (byte)m_statusFlags[FLAG_CARRY];
	oldCarry *= 0x80;
	m_statusFlags[FLAG_CARRY] = (value & 0x1 ? true : false);

	value /= 2;
	value |= oldCarry;

	m_statusFlags[FLAG_ZERO] = (value == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (value & 0x80 ? true : false);

	if (OpcodeAddrTypes[opcode] == ADDR_ACC)
	{
		m_A = value;
	}
	else
	{
		m_pNES->Write(value, addr);
	}
}

void CPU::RTI()
{
	// If there are any problems with interrupts, look here first!
	PLP();
	
	PCFromStack();
}

void CPU::RTS()
{
	// Same for subroutines!
	PCFromStack();

	++m_programCounter;
}

void CPU::SBC(byte value)
{
	word tempA = m_A - value - ( 0x1 - (byte)m_statusFlags[FLAG_CARRY] );

	m_statusFlags[FLAG_CARRY] = (tempA & 0xff00 ? true : false);
	m_statusFlags[FLAG_OVERFLOW] = ((~(m_A ^ value)) & (m_A ^ tempA) & 0x80 ? true : false);

	m_A = tempA & 0xff;

	m_statusFlags[FLAG_ZERO] = (m_A == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_A & 0x80 ? true : false);

	// Add cycle if boundary is crossed
	m_cycle += (byte)m_hasCrossedBoundary;
}

void CPU::SEC()
{
	m_statusFlags[FLAG_CARRY] = true;
}

void CPU::SED()
{
	m_statusFlags[FLAG_DECIMAL] = true;
}

void CPU::SEI()
{
	m_statusFlags[FLAG_INTERRUPT_DISABLE] = true;
}

void CPU::STA(word addr)
{
	m_pNES->Write(m_A, addr);
}

void CPU::STX(word addr)
{
	m_pNES->Write(m_X, addr);
}

void CPU::STY(word addr)
{
	m_pNES->Write(m_Y, addr);
}

void CPU::TAX()
{
	m_X = m_A;

	m_statusFlags[FLAG_ZERO] = (m_X == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_X & 0x80 ? true : false);
}

void CPU::TAY()
{
	m_Y = m_A;

	m_statusFlags[FLAG_ZERO] = (m_Y == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_Y & 0x80 ? true : false);
}

void CPU::TSX()
{
	m_X = m_stackPointer;

	m_statusFlags[FLAG_ZERO] = (m_X == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_X & 0x80 ? true : false);
}

void CPU::TXA()
{
	m_A = m_X;

	m_statusFlags[FLAG_ZERO] = (m_A == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_A & 0x80 ? true : false);
}

void CPU::TXS()
{
	m_stackPointer = m_X;
}

void CPU::TYA()
{
	m_A = m_Y;

	m_statusFlags[FLAG_ZERO] = (m_Y == 0x0 ? true : false);
	m_statusFlags[FLAG_NEGATIVE] = (m_Y & 0x80 ? true : false);
}
