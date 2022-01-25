#include "CPU.h"

CPU::CPU()
{
	RESET();

	for (unsigned int i = 0; i < 0x10000; ++i)
	{
		// To do - initialise to random numbers
		RAM[i] = 0xff;
	}
}

CPU::~CPU()
{
	delete[] RAM;
}

void CPU::NMI()
{
	PCToStack();

	statusFlags[FLAG_INTERRUPT_DISABLE] = true;
	statusFlags[FLAG_BREAK] = false;
	FlagsToStack();

	// Hard coded pointer to NMI at 0xfffa
	byte lowByte = RAM[0xfffa];
	byte highByte = RAM[0xfffb];
	programCounter = CombineAddrBytes(highByte, lowByte);
}

void CPU::RESET()
{
	A = 0x0;
	X = 0x0;
	Y = 0x0;
	stackPointer = 0xfd;

	// Hard coded pointer to RESET at 0xfffc
	byte lowByte = RAM[0xfffc];
	byte highByte = RAM[0xfffd];
	programCounter = CombineAddrBytes(highByte, lowByte);

	statusFlags[FLAG_CARRY] = false;
	statusFlags[FLAG_ZERO] = false;
	statusFlags[FLAG_INTERRUPT_DISABLE] = true;
	statusFlags[FLAG_DECIMAL] = false;
	statusFlags[FLAG_BREAK] = false;
	statusFlags[FLAG_UNUSED] = true;
	statusFlags[FLAG_OVERFLOW] = false;
	statusFlags[FLAG_NEGATIVE] = false;
}

void CPU::IRQ()
{
	if (!statusFlags[FLAG_INTERRUPT_DISABLE])
	{
		PCToStack();

		statusFlags[FLAG_INTERRUPT_DISABLE] = true;
		statusFlags[FLAG_BREAK] = false;
		FlagsToStack();

		// Hard coded pointer to IRQ at 0xfffe
		byte lowByte = RAM[0xfffe];
		byte highByte = RAM[0xffff];
		programCounter = CombineAddrBytes(highByte, lowByte);
	}
}

bool CPU::Update()
{
	// Get the next opcode
	byte opcode = RAM[programCounter];

	// Temp
	if (opcode == 0xff)
	{
		return true;
	}

	++programCounter;

	// Retrieves the byte of data used by the opcode, using the appropriate addressing mode
	word addr = 0x0;
	byte value = 0x0;
	switch (OpcodeAddrTypes[opcode])
	{
	case ADDR_IMP:
		// Nothing, no value is used
		break;
	case ADDR_ACC:
		value = A;
		break;
	case ADDR_IMM:
		addr = AddrImmediate();
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
		addr = AddrRelative();
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
	case ADDR_XXX:
		// Should only happen for unofficial opcodes
		assert(OpcodeAddrTypes[opcode] != ADDR_XXX);
		break;
	}

	if (OpcodeAddrTypes[opcode] != ADDR_IMP && OpcodeAddrTypes[opcode] != ADDR_ACC)
	{
		value = RAM[addr];
		++programCounter;
	}

	// Perform opcode using the retrieved byte
	switch (OpcodeTypes[opcode])
	{
	case OP_ADC:
		ADC(value);
		break;
	case OP_AND:
		AND(value);
		break;
	case OP_ASL:
		ASL(value, opcode, addr);
		break;
	case OP_NOP:
		NOP();
		break;
	case OP_XXX:
		assert(OpcodeTypes[opcode] != OP_XXX);
		break;
	}

	return false;
}

byte CPU::AddrImmediate()
{
	// Get byte after the opcode
	return programCounter;
}

byte CPU::AddrZero()
{
	// Get byte after the opcode
	byte addr = RAM[programCounter];

	++programCounter;

	// Use this byte as a zero page address
	return addr;
}

byte CPU::AddrZeroX()
{
	// Offset address by X - using a byte data type will make the address wrap around as expected
	byte addr = RAM[programCounter] + X;

	++programCounter;

	// Use this byte as a zero page address
	return addr;
}

byte CPU::AddrZeroY()
{
	// Offset address by Y - using a byte data type will make the address wrap around as expected
	byte addr = RAM[programCounter] + Y;

	++programCounter;

	// Use this byte as a zero page address
	return addr;
}

byte CPU::AddrRelative()
{
	// Get byte after the opcode
	return programCounter;
}

word CPU::AddrAbsolute()
{
	// Get byte after the opcode
	byte addrLow = RAM[programCounter];

	// Get next byte
	++programCounter;
	byte addrHigh = RAM[programCounter];

	++programCounter;

	// Combine for actual address
	return CombineAddrBytes(addrHigh, addrLow);
}

word CPU::AddrAbsoluteX()
{
	// Get byte after the opcode
	byte addrLow = RAM[programCounter];

	// Get next byte
	++programCounter;
	byte addrHigh = RAM[programCounter];

	++programCounter;

	// Combine for actual address
	return CombineAddrBytes(addrHigh, addrLow) + X;
}

word CPU::AddrAbsoluteY()
{
	// Get byte after the opcode
	byte addrLow = RAM[programCounter];

	// Get next byte
	++programCounter;
	byte addrHigh = RAM[programCounter];

	++programCounter;

	// Combine for actual address
	return CombineAddrBytes(addrHigh, addrLow) + Y;
}

word CPU::AddrIndirect()
{
	// Get byte after the opcode
	word addrPointerLow = RAM[programCounter];

	// Get next byte
	++programCounter;
	word addrPointerHigh = RAM[programCounter];

	// Combine for pointer
	word addrPointer = CombineAddrBytes(addrPointerHigh, addrPointerLow);

	++programCounter;

	// Get final address from pointer destination
	word addrLow = RAM[addrPointer];
	word addrHigh;
	if (addrPointerLow == 0xff)
	{
		// Emulated bug in the original 6502
		// When crossing page boundary, reads the high byte from the same page instead of the next page
		addrHigh = RAM[addrPointer + 1 - 0x100];
	}
	else
	{
		addrHigh = RAM[addrPointer + 1];
	}

	return CombineAddrBytes(addrHigh, addrLow);
}

word CPU::AddrIndirectX()
{
	// Get pointer
	byte addrPointer = RAM[programCounter] + X;

	++programCounter;

	// Get address to read from
	word addrLow = RAM[addrPointer];
	word addrHigh = RAM[addrPointer + 1];

	return CombineAddrBytes(addrHigh, addrLow);
}

word CPU::AddrIndirectY()
{
	// Get pointer
	byte addrPointer = RAM[programCounter];

	++programCounter;

	word addrLow = RAM[addrPointer];
	word addrHigh = RAM[addrPointer + 1];

	return CombineAddrBytes(addrHigh, addrLow) + Y;
}

void CPU::Branch(byte value)
{
	if (value & 0x80)
	{
		byte reverseValue = 0x100 - value;
		programCounter -= reverseValue;
	}
	else
	{
		programCounter += value;
	}
}

void CPU::PCFromStack()
{
	++stackPointer;
	byte lowByte = RAM[0x100 + stackPointer];
	++stackPointer;
	byte highByte = RAM[0x100 + stackPointer];

	programCounter = (word)highByte * 0x100 + (word)lowByte;
}

void CPU::PCToStack()
{
	byte lowByte = programCounter & 0xff;
	byte highByte = (programCounter - lowByte) / 0x100;

	RAM[0x0100 + stackPointer] = highByte;
	--stackPointer;
	RAM[0x0100 + stackPointer] = lowByte;
	--stackPointer;
}

void CPU::FlagsToStack()
{
	byte status =	0x1 * (byte)statusFlags[FLAG_CARRY] + 0x2 * (byte)statusFlags[FLAG_ZERO] + 0x4 * (byte)statusFlags[FLAG_INTERRUPT_DISABLE] + 0x8 * (byte)statusFlags[FLAG_DECIMAL]
					+ 0x10 * (byte)statusFlags[FLAG_BREAK] + 0x20 * (byte)statusFlags[FLAG_UNUSED] + 0x40 * (byte)statusFlags[FLAG_OVERFLOW] + 0x80 * (byte)statusFlags[FLAG_NEGATIVE];

	// Possibly - clear the B and U flags?

	RAM[0x100 + stackPointer] = status;
	--stackPointer;
}

word CPU::CombineAddrBytes(byte high, byte low)
{
	return ( (word)high * 0x100 + (word)low );
}

// Overflow logic from stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
void CPU::ADC(byte value)
{
	word tempA = A + value + (byte)statusFlags[FLAG_CARRY];

	statusFlags[FLAG_CARRY] = (tempA & 0xff00 ? true : false);
	statusFlags[FLAG_OVERFLOW] = ((~(A ^ value)) & (A ^ tempA) & 0x80 ? true : false);

	A = tempA & 0xff;

	statusFlags[FLAG_ZERO] = (A == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (A & 0x80 ? true : false);
}

void CPU::AND(byte value)
{
	A &= value;

	statusFlags[FLAG_ZERO] = (A == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (A & 0x80 ? true : false);
}

void CPU::ASL(byte value, byte opcode, word addr)
{
	statusFlags[FLAG_CARRY] = (value & 0x80 ? true : false);

	// When done in 8 bit space this will produce the correct result
	value *= 2;

	statusFlags[FLAG_ZERO] = (value == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (value & 0x80 ? true : false);

	if (OpcodeAddrTypes[opcode] == ADDR_ACC)
	{
		A = value;
	}
	else
	{
		RAM[addr] = value;
	}
}

void CPU::BCC(byte value)
{
	if (!statusFlags[FLAG_CARRY])
	{
		Branch(value);
	}
}

void CPU::BCS(byte value)
{
	if (statusFlags[FLAG_CARRY])
	{
		Branch(value);
	}
}

void CPU::BEQ(byte value)
{
	if (statusFlags[FLAG_ZERO])
	{
		Branch(value);
	}
}

void CPU::BIT(byte value)
{
	statusFlags[FLAG_OVERFLOW] = (value & 0x40 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (value & 0x80 ? true : false);

	value &= A;

	statusFlags[FLAG_ZERO] = (value == 0x0 ? true : false);
}

void CPU::BMI(byte value)
{
	if (statusFlags[FLAG_NEGATIVE])
	{
		Branch(value);
	}
}

void CPU::BNE(byte value)
{
	if (!statusFlags[FLAG_ZERO])
	{
		Branch(value);
	}
}

void CPU::BPL(byte value)
{
	if (!statusFlags[FLAG_NEGATIVE])
	{
		Branch(value);
	}
}

void CPU::BRK(byte value)
{
	++programCounter;
	PCToStack();

	statusFlags[FLAG_INTERRUPT_DISABLE] = true;
	FlagsToStack();

	statusFlags[FLAG_BREAK] = true;

	// Hard coded pointer to IRQ at 0xfffe
	byte lowByte = RAM[0xfffe];
	byte highByte = RAM[0xffff];
	programCounter = CombineAddrBytes(highByte, lowByte);
}

void CPU::BVC(byte value)
{
	if (!statusFlags[FLAG_OVERFLOW])
	{
		Branch(value);
	}
}

void CPU::BVS(byte value)
{
	if (statusFlags[FLAG_OVERFLOW])
	{
		Branch(value);
	}
}

void CPU::CLC()
{
	statusFlags[FLAG_CARRY] = false;
}

void CPU::CLD()
{
	statusFlags[FLAG_DECIMAL] = false;
}

void CPU::CLI()
{
	statusFlags[FLAG_INTERRUPT_DISABLE] = false;
}

void CPU::CLV()
{
	statusFlags[FLAG_OVERFLOW] = false;
}

void CPU::CMP(byte value)
{
	statusFlags[FLAG_CARRY] = (A >= value ? true : false);
	statusFlags[FLAG_ZERO] = (A == value ? true : false);

	byte tempA = A - value;

	statusFlags[FLAG_NEGATIVE] = (tempA & 0x80 ? true : false);
}

void CPU::CPX(byte value)
{
	statusFlags[FLAG_CARRY] = (X >= value ? true : false);
	statusFlags[FLAG_ZERO] = (X == value ? true : false);

	byte tempX = X - value;

	statusFlags[FLAG_NEGATIVE] = (tempX & 0x80 ? true : false);
}

void CPU::CPY(byte value)
{
	statusFlags[FLAG_CARRY] = (Y >= value ? true : false);
	statusFlags[FLAG_ZERO] = (Y == value ? true : false);

	byte tempY = Y - value;

	statusFlags[FLAG_NEGATIVE] = (tempY & 0x80 ? true : false);
}

void CPU::DEC(word addr)
{
	--RAM[addr];

	statusFlags[FLAG_ZERO] = (RAM[addr] == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (RAM[addr] & 0x80 ? true : false);
}

void CPU::DEX()
{
	--X;

	statusFlags[FLAG_ZERO] = (X == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (X & 0x80 ? true : false);
}

void CPU::DEY()
{
	--Y;

	statusFlags[FLAG_ZERO] = (Y == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (Y & 0x80 ? true : false);
}

void CPU::EOR(byte value)
{
	A ^= value;

	statusFlags[FLAG_ZERO] = (A == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (A & 0x80 ? true : false);
}

void CPU::INC(word addr)
{
	++RAM[addr];

	statusFlags[FLAG_ZERO] = (RAM[addr] == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (RAM[addr] & 0x80 ? true : false);
}

void CPU::INX()
{
	++X;

	statusFlags[FLAG_ZERO] = (X == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (X & 0x80 ? true : false);
}

void CPU::INY()
{
	++Y;

	statusFlags[FLAG_ZERO] = (Y == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (Y & 0x80 ? true : false);
}

void CPU::JMP(word addr)
{
	programCounter = addr;
}

void CPU::JSR(word addr)
{
	--programCounter;

	PCToStack();

	programCounter = addr;
}

void CPU::LDA(byte value)
{
	A = value;

	statusFlags[FLAG_ZERO] = (A == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (A & 0x80 ? true : false);
}

void CPU::LDX(byte value)
{
	X = value;

	statusFlags[FLAG_ZERO] = (X == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (X & 0x80 ? true : false);
}

void CPU::LDY(byte value)
{
	Y = value;

	statusFlags[FLAG_ZERO] = (Y == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (Y & 0x80 ? true : false);
}

void CPU::LSR(byte value, byte opcode, word addr)
{
	statusFlags[FLAG_CARRY] = (value & 0x1 ? true : false);

	value / 2;

	statusFlags[FLAG_ZERO] = (value == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (value & 0x80 ? true : false);

	if (OpcodeAddrTypes[opcode] == ADDR_ACC)
	{
		A = value;
	}
	else
	{
		RAM[addr] = value;
	}
}

void CPU::NOP()
{
	// Nothing
	// Don't forget to add clock cycles here when you get to those!
}

void CPU::ORA(byte value)
{
	A |= value;

	statusFlags[FLAG_ZERO] = (A == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (A & 0x80 ? true : false);
}

void CPU::PHA()
{
	RAM[0x100 + stackPointer] = A;
	--stackPointer;
}

void CPU::PHP()
{
	statusFlags[FLAG_BREAK] = true;

	FlagsToStack();
}

void CPU::PLA()
{
	++stackPointer;
	A = RAM[0x100 + stackPointer];

	statusFlags[FLAG_ZERO] = (A == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (A & 0x80 ? true : false);
}

void CPU::PLP()
{
	++stackPointer;
	byte status = RAM[0x100 + stackPointer];

	statusFlags[FLAG_CARRY] = status & 0x1;
	statusFlags[FLAG_ZERO] = status & 0x2;
	statusFlags[FLAG_INTERRUPT_DISABLE] = status & 0x4;
	statusFlags[FLAG_DECIMAL] = status & 0x8;
	statusFlags[FLAG_BREAK] = status & 0x10;
	statusFlags[FLAG_UNUSED] = true;
	statusFlags[FLAG_OVERFLOW] = status & 0x40;
	statusFlags[FLAG_NEGATIVE] = status & 0x80;
}

void CPU::ROL(byte value, byte opcode, word addr)
{
	bool oldCarry = statusFlags[FLAG_CARRY];
	statusFlags[FLAG_CARRY] = (value & 0x80 ? true : false);

	value *= 2;
	// Set bit 0 to the old carry flag
	value |= (byte)oldCarry;

	statusFlags[FLAG_ZERO] = (value == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (value & 0x80 ? true : false);

	if (OpcodeAddrTypes[opcode] == ADDR_ACC)
	{
		A = value;
	}
	else
	{
		RAM[addr] = value;
	}
}

void CPU::ROR(byte value, byte opcode, word addr)
{
	byte oldCarry = (byte)statusFlags[FLAG_CARRY];
	oldCarry *= 0x80;
	statusFlags[FLAG_CARRY] = (value & 0x1 ? true : false);

	value /= 2;
	value |= oldCarry;

	statusFlags[FLAG_ZERO] = (value == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (value & 0x80 ? true : false);

	if (OpcodeAddrTypes[opcode] == ADDR_ACC)
	{
		A = value;
	}
	else
	{
		RAM[addr] = value;
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

	++programCounter;
}

void CPU::SBC(byte value)
{
	word tempA = A - value - ( 0x1 - (byte)statusFlags[FLAG_CARRY] );

	statusFlags[FLAG_CARRY] = (tempA & 0xff00 ? true : false);
	statusFlags[FLAG_OVERFLOW] = ((~(A ^ value)) & (A ^ tempA) & 0x80 ? true : false);

	A = tempA & 0xff;

	statusFlags[FLAG_ZERO] = (A == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (A & 0x80 ? true : false);
}

void CPU::SEC()
{
	statusFlags[FLAG_CARRY] = true;
}

void CPU::SED()
{
	statusFlags[FLAG_DECIMAL] = true;
}

void CPU::SEI()
{
	statusFlags[FLAG_INTERRUPT_DISABLE] = true;
}

void CPU::STA(word addr)
{
	RAM[addr] = A;
}

void CPU::STX(word addr)
{
	RAM[addr] = X;
}

void CPU::STY(word addr)
{
	RAM[addr] = Y;
}

void CPU::TAX()
{
	X = A;

	statusFlags[FLAG_ZERO] = (X == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (X & 0x80 ? true : false);
}

void CPU::TAY()
{
	Y = A;

	statusFlags[FLAG_ZERO] = (Y == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (Y & 0x80 ? true : false);
}

void CPU::TSX()
{
	X = stackPointer;

	statusFlags[FLAG_ZERO] = (X == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (X & 0x80 ? true : false);
}

void CPU::TXA()
{
	A = X;

	statusFlags[FLAG_ZERO] = (A == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (A & 0x80 ? true : false);
}

void CPU::TXS()
{
	stackPointer = X;
}

void CPU::TYA()
{
	A = Y;

	statusFlags[FLAG_ZERO] = (Y == 0x0 ? true : false);
	statusFlags[FLAG_NEGATIVE] = (Y & 0x80 ? true : false);
}
