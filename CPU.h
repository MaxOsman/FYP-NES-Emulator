#pragma once
#include <assert.h>
#include "Types.h"

#define NMI_VECTOR 0xfffa
#define RESET_VECTOR 0xfffc
#define IRQ_VECTOR 0xfffe

class NES;


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
		ADDR_ABS,
		ADDR_ABX,
		ADDR_ABY,
		ADDR_IND,
		ADDR_INX,
		ADDR_INY,
		ADDR_NON
	};

	enum OPCODE_TYPE
	{
		OP_ADC = 0,	OP_AND,  OP_ASL,  OP_BCC,  OP_BCS,  OP_BEQ,  OP_BIT,  OP_BMI,  OP_BNE,  OP_BPL,  OP_BRK,  OP_BVC,  OP_BVS,  OP_CLC,

		OP_CLD,     OP_CLI,  OP_CLV,  OP_CMP,  OP_CPX,  OP_CPY,  OP_DEC,  OP_DEX,  OP_DEY,  OP_EOR,  OP_INC,  OP_INX,  OP_INY,  OP_JMP,

		OP_JSR,		OP_LDA,  OP_LDX,  OP_LDY,  OP_LSR,  OP_NOP,  OP_ORA,  OP_PHA,  OP_PHP,  OP_PLA,  OP_PLP,  OP_ROL,  OP_ROR,  OP_RTI,

		OP_RTS,		OP_SBC,  OP_SEC,  OP_SED,  OP_SEI,  OP_STA,  OP_STX,  OP_STY,  OP_TAX,  OP_TAY,  OP_TSX,  OP_TXA,  OP_TXS,  OP_TYA,

		OP_NON
	};

	// 151 opcodes
	const ADDRESSING_TYPE OpcodeAddrTypes[0x100] =
	{
		ADDR_IMP, ADDR_INX, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ZER, ADDR_ZER, ADDR_NON, ADDR_IMP, ADDR_IMM, ADDR_ACC, ADDR_NON, ADDR_NON, ADDR_ABS, ADDR_ABS, ADDR_NON,
		ADDR_IMM, ADDR_INY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ZEX, ADDR_ZEX, ADDR_NON, ADDR_IMP, ADDR_ABY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ABX, ADDR_ABX, ADDR_NON,
		ADDR_ABS, ADDR_INX, ADDR_NON, ADDR_NON, ADDR_ZER, ADDR_ZER, ADDR_ZER, ADDR_NON, ADDR_IMP, ADDR_IMM, ADDR_ACC, ADDR_NON, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_NON,
		ADDR_IMM, ADDR_INY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ZEX, ADDR_ZEX, ADDR_NON, ADDR_IMP, ADDR_ABY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ABX, ADDR_ABX, ADDR_NON,
		ADDR_IMP, ADDR_INX, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ZER, ADDR_ZER, ADDR_NON, ADDR_IMP, ADDR_IMM, ADDR_ACC, ADDR_NON, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_NON,
		ADDR_IMM, ADDR_INY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ZEX, ADDR_ZEX, ADDR_NON, ADDR_IMP, ADDR_ABY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ABX, ADDR_ABX, ADDR_NON,
		ADDR_IMP, ADDR_INX, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ZER, ADDR_ZER, ADDR_NON, ADDR_IMP, ADDR_IMM, ADDR_ACC, ADDR_NON, ADDR_IND, ADDR_ABS, ADDR_ABS, ADDR_NON,
		ADDR_IMM, ADDR_INY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ZEX, ADDR_ZEX, ADDR_NON, ADDR_IMP, ADDR_ABY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ABX, ADDR_ABX, ADDR_NON,
		ADDR_NON, ADDR_INX, ADDR_NON, ADDR_NON, ADDR_ZER, ADDR_ZER, ADDR_ZER, ADDR_NON, ADDR_IMP, ADDR_NON, ADDR_IMP, ADDR_NON, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_NON,
		ADDR_IMM, ADDR_INY, ADDR_NON, ADDR_NON, ADDR_ZEX, ADDR_ZEX, ADDR_ZEY, ADDR_NON, ADDR_IMP, ADDR_ABY, ADDR_IMP, ADDR_NON, ADDR_NON, ADDR_ABX, ADDR_NON, ADDR_NON,
		ADDR_IMM, ADDR_INX, ADDR_IMM, ADDR_NON, ADDR_ZER, ADDR_ZER, ADDR_ZER, ADDR_NON, ADDR_IMP, ADDR_IMM, ADDR_IMP, ADDR_NON, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_NON,
		ADDR_IMM, ADDR_INY, ADDR_NON, ADDR_NON, ADDR_ZEX, ADDR_ZEX, ADDR_ZEY, ADDR_NON, ADDR_IMP, ADDR_ABY, ADDR_IMP, ADDR_NON, ADDR_ABX, ADDR_ABX, ADDR_ABY, ADDR_NON,
		ADDR_IMM, ADDR_INX, ADDR_NON, ADDR_NON, ADDR_ZER, ADDR_ZER, ADDR_ZER, ADDR_NON, ADDR_IMP, ADDR_IMM, ADDR_IMP, ADDR_NON, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_NON,
		ADDR_IMM, ADDR_INY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ZEX, ADDR_ZEX, ADDR_NON, ADDR_IMP, ADDR_ABY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ABX, ADDR_ABX, ADDR_NON,
		ADDR_IMM, ADDR_INX, ADDR_NON, ADDR_NON, ADDR_ZER, ADDR_ZER, ADDR_ZER, ADDR_NON, ADDR_IMP, ADDR_IMM, ADDR_IMP, ADDR_NON, ADDR_ABS, ADDR_ABS, ADDR_ABS, ADDR_NON,
		ADDR_IMM, ADDR_INY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ZEX, ADDR_ZEX, ADDR_NON, ADDR_IMP, ADDR_ABY, ADDR_NON, ADDR_NON, ADDR_NON, ADDR_ABX, ADDR_ABX, ADDR_NON
	};

	// 56 uniques opcodes, not counting unofficial ones
	const OPCODE_TYPE OpcodeTypes[0x100] =
	{
		OP_BRK, OP_ORA, OP_NON, OP_NON, OP_NON, OP_ORA, OP_ASL, OP_NON, OP_PHP, OP_ORA, OP_ASL, OP_NON, OP_NON, OP_ORA, OP_ASL, OP_NON,
		OP_BPL, OP_ORA, OP_NON, OP_NON, OP_NON, OP_ORA, OP_ASL, OP_NON, OP_CLC, OP_ORA, OP_NON, OP_NON, OP_NON, OP_ORA, OP_ASL, OP_NON,
		OP_JSR, OP_AND, OP_NON, OP_NON, OP_BIT, OP_AND, OP_ROL, OP_NON, OP_PLP, OP_AND, OP_ROL, OP_NON, OP_BIT, OP_AND, OP_ROL, OP_NON,
		OP_BMI, OP_AND, OP_NON, OP_NON, OP_NON, OP_AND, OP_ROL, OP_NON, OP_SEC, OP_AND, OP_NON, OP_NON, OP_NON, OP_AND, OP_ROL, OP_NON,
		OP_RTI, OP_EOR, OP_NON, OP_NON, OP_NON, OP_EOR, OP_LSR, OP_NON, OP_PHA, OP_EOR, OP_LSR, OP_NON, OP_JMP, OP_EOR, OP_LSR, OP_NON,
		OP_BVC, OP_EOR, OP_NON, OP_NON, OP_NON, OP_EOR, OP_LSR, OP_NON, OP_CLI, OP_EOR, OP_NON, OP_NON, OP_NON, OP_EOR, OP_LSR, OP_NON,
		OP_RTS, OP_ADC, OP_NON, OP_NON, OP_NON, OP_ADC, OP_ROR, OP_NON, OP_PLA, OP_ADC, OP_ROR, OP_NON, OP_JMP, OP_ADC, OP_ROR, OP_NON,
		OP_BVS, OP_ADC, OP_NON, OP_NON, OP_NON, OP_ADC, OP_ROR, OP_NON, OP_SEI, OP_ADC, OP_NON, OP_NON, OP_NON, OP_ADC, OP_ROR, OP_NON,
		OP_NON, OP_STA, OP_NON, OP_NON, OP_STY, OP_STA, OP_STX, OP_NON, OP_DEY, OP_NON, OP_TXA, OP_NON, OP_STY, OP_STA, OP_STX, OP_NON,
		OP_BCC, OP_STA, OP_NON, OP_NON, OP_STY, OP_STA, OP_STX, OP_NON, OP_TYA, OP_STA, OP_TXS, OP_NON, OP_NON, OP_STA, OP_NON, OP_NON,
		OP_LDY, OP_LDA, OP_LDX, OP_NON, OP_LDY, OP_LDA, OP_LDX, OP_NON, OP_TAY, OP_LDA, OP_TAX, OP_NON, OP_LDY, OP_LDA, OP_LDX, OP_NON,
		OP_BCS, OP_LDA, OP_NON, OP_NON, OP_LDY, OP_LDA, OP_LDX, OP_NON, OP_CLV, OP_LDA, OP_TSX, OP_NON, OP_LDY, OP_LDA, OP_LDX, OP_NON,
		OP_CPY, OP_CMP, OP_NON, OP_NON, OP_CPY, OP_CMP, OP_DEC, OP_NON, OP_INY, OP_CMP, OP_DEX, OP_NON, OP_CPY, OP_CMP, OP_DEC, OP_NON,
		OP_BNE, OP_CMP, OP_NON, OP_NON, OP_NON, OP_CMP, OP_DEC, OP_NON, OP_CLD, OP_CMP, OP_NON, OP_NON, OP_NON, OP_CMP, OP_DEC, OP_NON,
		OP_CPX, OP_SBC, OP_NON, OP_NON, OP_CPX, OP_SBC, OP_INC, OP_NON, OP_INX, OP_SBC, OP_NOP, OP_NON, OP_CPX, OP_SBC, OP_INC, OP_NON,
		OP_BEQ, OP_SBC, OP_NON, OP_NON, OP_NON, OP_SBC, OP_INC, OP_NON, OP_SED, OP_SBC, OP_NON, OP_NON, OP_NON, OP_SBC, OP_INC, OP_NON
	};

	const byte OpcodeTimings[0x100] =
	{
		7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,
		2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
		6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,
		2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
		6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,
		2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
		6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,
		2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
		0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0,
		2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0,
		2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
		2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,
		2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
		2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
		2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
		2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0
	};

	// Registers
	byte m_A, m_X, m_Y;
	word m_programCounter;
	byte m_stackPointer;
	bool m_statusFlags[8] = { false, false, true, false, false, true, false, false };

	NES* m_pNES;

	byte m_cycle;
	bool m_hasCrossedBoundary;
	unsigned long long int m_totalCycles;

	CPU(NES* parentNES);
	~CPU();

	// Interrupts
	void NMI();
	void RESET();
	void IRQ();

	void Update();

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

	word CombineAddrBytes(byte high, byte low);

	// Opcodes
	void ADC(byte value, byte opcode);
	void AND(byte value);
	void ASL(byte value, byte opcode, word addr);
	void BCC(byte value);
	void BCS(byte value);
	void BEQ(byte value);
	void BIT(byte value);
	void BMI(byte value);
	void BNE(byte value);
	void BPL(byte value);
	void BRK();
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