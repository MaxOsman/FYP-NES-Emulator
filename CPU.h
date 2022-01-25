#pragma once
#include <assert.h>

typedef unsigned char byte;
typedef unsigned short word;

#define NMI_VECTOR 0xfffa
#define RESET_VECTOR 0xfffc
#define IRQ_VECTOR 0xfffe

class CPU
{
public:
	enum STATUS_FLAGS
	{
		FLAG_CARRY = 0,
		FLAG_ZERO,
		FLAG_INTERRUPT_DISABLE,		// Initialised to 1
		FLAG_DECIMAL,
		FLAG_BREAK,
		FLAG_UNUSED,				// Should always be set to 1
		FLAG_OVERFLOW,
		FLAG_NEGATIVE,
	};

	enum ADDRESSING_TYPE
	{
		ADDR_IMP = 0,
		ADDR_ACC,
		ADDR_IMM,
		ADDR_ZER,
		ADDR_ZEX,
		ADDR_ZEY,
		ADDR_REL,
		ADDR_ABS,
		ADDR_ABX,
		ADDR_ABY,
		ADDR_IND,
		ADDR_INX,
		ADDR_INY,
		ADDR_XXX
	};

	enum OPCODE_TYPE
	{
		OP_ADC = 0,	OP_AND,  OP_ASL,  OP_BCC,  OP_BCS,  OP_BEQ,  OP_BIT,  OP_BMI,  OP_BNE,  OP_BPL,  OP_BRK,  OP_BVC,  OP_BVS,  OP_CLC,

		OP_CLD,     OP_CLI,  OP_CLV,  OP_CMP,  OP_CPX,  OP_CPY,  OP_DEC,  OP_DEX,  OP_DEY,  OP_EOR,  OP_INC,  OP_INX,  OP_INY,  OP_JMP,

		OP_JSR,		OP_LDA,  OP_LDX,  OP_LDY,  OP_LSR,  OP_NOP,  OP_ORA,  OP_PHA,  OP_PHP,  OP_PLA,  OP_PLP,  OP_ROL,  OP_ROR,  OP_RTI,

		OP_RTS,		OP_SBC,  OP_SEC,  OP_SED,  OP_SEI,  OP_STA,  OP_STX,  OP_STY,  OP_TAX,  OP_TAY,  OP_TSX,  OP_TXA,  OP_TXS,  OP_TYA,

		OP_XXX
	};

	// 151 opcodes
	const ADDRESSING_TYPE OpcodeAddrTypes[0x100] =
	{
		ADDR_IMP, ADDR_INX, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ZER, ADDR_ZER, ADDR_XXX, ADDR_IMP, ADDR_IMM, ADDR_ACC, ADDR_XXX, ADDR_XXX, ADDR_ABS, ADDR_ABS, ADDR_XXX,
		ADDR_REL, ADDR_INY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ZEX, ADDR_ZEX, ADDR_XXX, ADDR_IMP, ADDR_ABY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ABX, ADDR_ABX, ADDR_XXX,
		ADDR_ABS, ADDR_INX, ADDR_XXX, ADDR_XXX, ADDR_ZER, ADDR_ZER, ADDR_ZER, ADDR_XXX, ADDR_IMP, ADDR_IMM, ADDR_ACC, ADDR_XXX, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_XXX,
		ADDR_REL, ADDR_INY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ZEX, ADDR_ZEX, ADDR_XXX, ADDR_IMP, ADDR_ABY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ABX, ADDR_ABX, ADDR_XXX,
		ADDR_IMP, ADDR_INX, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ZER, ADDR_ZER, ADDR_XXX, ADDR_IMP, ADDR_IMM, ADDR_ACC, ADDR_XXX, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_XXX,
		ADDR_REL, ADDR_INY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ZEX, ADDR_ZEX, ADDR_XXX, ADDR_IMP, ADDR_ABY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ABX, ADDR_ABX, ADDR_XXX,
		ADDR_IMP, ADDR_INX, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ZER, ADDR_ZER, ADDR_XXX, ADDR_IMP, ADDR_IMM, ADDR_ACC, ADDR_XXX, ADDR_IND, ADDR_ABS, ADDR_ABS, ADDR_XXX,
		ADDR_REL, ADDR_INY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ZEX, ADDR_ZEX, ADDR_XXX, ADDR_IMP, ADDR_ABY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ABX, ADDR_ABX, ADDR_XXX,
		ADDR_XXX, ADDR_INX, ADDR_XXX, ADDR_XXX, ADDR_ZER, ADDR_ZER, ADDR_ZER, ADDR_XXX, ADDR_IMP, ADDR_XXX, ADDR_IMP, ADDR_XXX, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_XXX,
		ADDR_REL, ADDR_INY, ADDR_XXX, ADDR_XXX, ADDR_ZEX, ADDR_ZEX, ADDR_ZEY, ADDR_XXX, ADDR_IMP, ADDR_ABY, ADDR_IMP, ADDR_XXX, ADDR_XXX, ADDR_ABX, ADDR_XXX, ADDR_XXX,
		ADDR_IMM, ADDR_INX, ADDR_IMM, ADDR_XXX, ADDR_ZER, ADDR_ZER, ADDR_ZER, ADDR_XXX, ADDR_IMP, ADDR_IMM, ADDR_IMM, ADDR_XXX, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_XXX,
		ADDR_REL, ADDR_INY, ADDR_XXX, ADDR_XXX, ADDR_ZEX, ADDR_ZEX, ADDR_ZEY, ADDR_XXX, ADDR_IMP, ADDR_ABY, ADDR_IMP, ADDR_XXX, ADDR_ABX, ADDR_ABX, ADDR_ABY, ADDR_XXX,
		ADDR_IMM, ADDR_INX, ADDR_XXX, ADDR_XXX, ADDR_ZER, ADDR_ZER, ADDR_ZER, ADDR_XXX, ADDR_IMP, ADDR_IMM, ADDR_IMP, ADDR_XXX, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_XXX,
		ADDR_REL, ADDR_INY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ZEX, ADDR_ZEX, ADDR_XXX, ADDR_IMP, ADDR_ABY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ABX, ADDR_ABX, ADDR_XXX,
		ADDR_IMM, ADDR_INX, ADDR_XXX, ADDR_XXX, ADDR_ZER, ADDR_ZER, ADDR_ZER, ADDR_XXX, ADDR_IMP, ADDR_IMM, ADDR_IMP, ADDR_XXX, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_XXX,
		ADDR_REL, ADDR_INY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ZEX, ADDR_ZEX, ADDR_XXX, ADDR_IMP, ADDR_ABY, ADDR_XXX, ADDR_XXX, ADDR_XXX, ADDR_ABX, ADDR_ABX, ADDR_XXX
	};

	// 56 uniques opcodes, not counting unofficial ones
	const OPCODE_TYPE OpcodeTypes[0x100] =
	{
		OP_BRK, OP_ORA, OP_XXX, OP_XXX, OP_XXX, OP_ORA, OP_ASL, OP_XXX, OP_PHP, OP_ORA, OP_ASL, OP_XXX, OP_XXX, OP_ORA, OP_ASL, OP_XXX,
		OP_BPL, OP_ORA, OP_XXX, OP_XXX, OP_XXX, OP_ORA, OP_ASL, OP_XXX, OP_CLC, OP_ORA, OP_XXX, OP_XXX, OP_XXX, OP_ORA, OP_ASL, OP_XXX,
		OP_JSR, OP_AND, OP_XXX, OP_XXX, OP_BIT, OP_AND, OP_ROL, OP_XXX, OP_PLP, OP_AND, OP_ROL, OP_XXX, OP_BIT, OP_AND, OP_ROL, OP_XXX,
		OP_BMI, OP_AND, OP_XXX, OP_XXX, OP_XXX, OP_AND, OP_ROL, OP_XXX, OP_SEC, OP_AND, OP_XXX, OP_XXX, OP_XXX, OP_AND, OP_ROL, OP_XXX,
		OP_RTI, OP_EOR, OP_XXX, OP_XXX, OP_XXX, OP_EOR, OP_LSR, OP_XXX, OP_PHA, OP_EOR, OP_LSR, OP_XXX, OP_JMP, OP_EOR, OP_LSR, OP_XXX,
		OP_BVC, OP_EOR, OP_XXX, OP_XXX, OP_XXX, OP_EOR, OP_LSR, OP_XXX, OP_CLI, OP_EOR, OP_XXX, OP_XXX, OP_XXX, OP_EOR, OP_LSR, OP_XXX,
		OP_RTS, OP_ADC, OP_XXX, OP_XXX, OP_XXX, OP_ADC, OP_ROR, OP_XXX, OP_PLA, OP_ADC, OP_ROR, OP_XXX, OP_JMP, OP_ADC, OP_ROR, OP_XXX,
		OP_BVS, OP_ADC, OP_XXX, OP_XXX, OP_XXX, OP_ADC, OP_ROR, OP_XXX, OP_SEI, OP_ADC, OP_XXX, OP_XXX, OP_XXX, OP_ADC, OP_ROR, OP_XXX,
		OP_XXX, OP_STA, OP_XXX, OP_XXX, OP_STY, OP_STA, OP_STX, OP_XXX, OP_DEY, OP_XXX, OP_TXA, OP_XXX, OP_STY, OP_STA, OP_STX, OP_XXX,
		OP_BCC, OP_STA, OP_XXX, OP_XXX, OP_STY, OP_STA, OP_STX, OP_XXX, OP_TYA, OP_STA, OP_TXS, OP_XXX, OP_XXX, OP_STA, OP_XXX, OP_XXX,
		OP_LDY, OP_LDA, OP_LDX, OP_XXX, OP_LDY, OP_LDA, OP_LDX, OP_XXX, OP_TAY, OP_LDA, OP_TAX, OP_XXX, OP_LDY, OP_LDA, OP_LDX, OP_XXX,
		OP_BCS, OP_LDA, OP_XXX, OP_XXX, OP_LDY, OP_LDA, OP_LDX, OP_XXX, OP_CLV, OP_LDA, OP_TSX, OP_XXX, OP_LDY, OP_LDA, OP_LDX, OP_XXX,
		OP_CPY, OP_CMP, OP_XXX, OP_XXX, OP_CPY, OP_CMP, OP_DEC, OP_XXX, OP_INY, OP_CMP, OP_DEX, OP_XXX, OP_CPY, OP_CMP, OP_DEC, OP_XXX,
		OP_BNE, OP_CMP, OP_XXX, OP_XXX, OP_XXX, OP_CMP, OP_DEC, OP_XXX, OP_CLD, OP_CMP, OP_XXX, OP_XXX, OP_XXX, OP_CMP, OP_DEC, OP_XXX,
		OP_CPX, OP_SBC, OP_XXX, OP_XXX, OP_CPX, OP_SBC, OP_INC, OP_XXX, OP_INX, OP_SBC, OP_NOP, OP_XXX, OP_CPX, OP_SBC, OP_INC, OP_XXX,
		OP_BEQ, OP_SBC, OP_XXX, OP_XXX, OP_XXX, OP_SBC, OP_INC, OP_XXX, OP_SED, OP_SBC, OP_XXX, OP_XXX, OP_XXX, OP_SBC, OP_INC, OP_XXX,
	};

	// Registers
	byte A, X, Y;
	word programCounter;
	byte stackPointer;
	bool statusFlags[8] = { false, false, true, false, false, true, false, false };

	// Temp until bus
	byte* RAM = new byte[0x10000];

	CPU();
	~CPU();

	// Interrupts
	void NMI();
	void RESET();
	void IRQ();

	bool Update();

	// Addressing modes
	// They return the address of the byte to be retrieved
	byte AddrImmediate();
	byte AddrZero();
	byte AddrZeroX();
	byte AddrZeroY();
	byte AddrRelative();
	word AddrAbsolute();
	word AddrAbsoluteX();
	word AddrAbsoluteY();
	word AddrIndirect();
	word AddrIndirectX();
	word AddrIndirectY();

	// Helper functions
	void Branch(byte value);
	void PCFromStack();
	void PCToStack();
	void FlagsToStack();

	// Opcodes
	void ADC(byte value);
	void AND(byte value);
	void ASL(byte value, byte opcode, word addr);
	void BCC(byte value);
	void BCS(byte value);
	void BEQ(byte value);
	void BIT(byte value);
	void BMI(byte value);
	void BNE(byte value);
	void BPL(byte value);
	void BRK(byte value);
	void BVC(byte value);
	void BVS(byte value);
	void CLC();

	void CLD();
	void CLI();
	void CLV();
	void CMP(byte value);
	void CPX(byte value);
	void CPY(byte value);
	void DEC(word addr);
	void DEX();
	void DEY();
	void EOR(byte value);
	void INC(word addr);
	void INX();
	void INY();
	void JMP(word addr);

	void JSR(word addr);
	void LDA(byte value);
	void LDX(byte value);
	void LDY(byte value);
	void LSR(byte value, byte opcode, word addr);
	void NOP();
	void ORA(byte value);
	void PHA();
	void PHP();
	void PLA();
	void PLP();
	void ROL(byte value, byte opcode, word addr);
	void ROR(byte value, byte opcode, word addr);
	void RTI();

	void RTS();
	void SBC(byte value);
	void SEC();
	void SED();
	void SEI();
	void STA(word addr);
	void STX(word addr);
	void STY(word addr);
	void TAX();
	void TAY();
	void TSX();
	void TXA();
	void TXS();
	void TYA();
};