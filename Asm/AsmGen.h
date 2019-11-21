/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/*
 * This file contains the prototypes of the methods used to insert extra
 * code into the code of the program that is to be protected.
 *
 * The functions are named after the assembly instruction they insert.
 *
 * Code for POP and PUSH instructions is located in the according <targetISA>_Functions.cpp
 * file in the Targets folder.
 */

#ifndef ASM_ASMGEN_H_
#define ASM_ASMGEN_H_

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>
#include <emit-rtl.h>

class AsmGen{
	public:
		static rtx_insn* emitCmpRegInt(unsigned int regNumber, int number,rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitCmpRegReg(unsigned int reg1, unsigned int reg2, rtx_insn* attachRtx, basic_block bb, bool after);

		static rtx_insn* emitAndRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitEorRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after);

		// Dividend divided by divisor
		static rtx_insn* emitUdivRegRegReg(unsigned int destReg, unsigned int dividendReg, unsigned int divisorReg,
				rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitMulRegReg(unsigned int destReg, unsigned int operand1, unsigned int operand2,
				rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitSubRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitAddRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitCondSubRegInt(unsigned int regNumber, int number, rtx_code condition, rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitCondAddRegInt(unsigned int regNumber, int number, rtx_code condition, rtx_insn* attachRtx, basic_block bb, bool after);

		static rtx_insn* emitMovRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitMovRegReg(unsigned int destReg, unsigned int srcReg, rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitCondMovRegInt(rtx_code condition, unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after);


		static rtx_insn* emitCodeLabel(unsigned int insnID, rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitBeq(rtx_insn* codeLabel, rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitBne(rtx_insn* codelabel, rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitBne(unsigned int regNumber, int cmpNumber, rtx_insn* codeLabel, rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitBhs(rtx_insn* codeLabel, rtx_insn* attachRtx, basic_block bb, bool after);

		static rtx_insn* emitCall(rtx_insn* codeLabel);

		static rtx_insn* emitAsmInput(const char* asmInstr, rtx_insn* attachRtx, basic_block bb, bool after);

	private:
		static rtx_insn* emitInsn(rtx rtxInsn,rtx_insn* attachRtx, basic_block bb, bool after);
		static rtx_insn* emitLabel(rtx label, rtx_insn* attachRtx, bool after);

		static rtx createConstInt(int number);
		static rtx createCondition(rtx_code condition);
};



#endif /* ASM_ASMGEN_H_ */
