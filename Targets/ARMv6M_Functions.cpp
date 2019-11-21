/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "ARMv6M_Functions.h"
#include "AsmGen.h"


ARMv6M_Functions::ARMv6M_Functions(processor_type cpu)
	:ARM_ISA(cpu) {}

void ARMv6M_Functions::changeCBZ(){
	// Does not exist in ARMv6M, so nothing to do
}

/**
 * Function to emit the necessary PUSH instruction
 * Emits STR reg, [r6] for each necessary register (reg)
 * Uses the pushPopStrings variable and the emitAsmInput method of AsmGen class.
 */
void ARMv6M_Functions::insertPush(vector<unsigned int> regs){
	// 1) Get necessary emit variables
	rtx_insn* next = get_first_nonnote_insn();
	basic_block bb = BASIC_BLOCK_FOR_FN(cfun,2);
	bool after = false;

	// 2) Emit a push for each register
	vector<unsigned int>::const_iterator it;
	for(it = regs.begin(); it != regs.end(); it++){
		next = AsmGen::emitAddRegInt(this->stackPointer, -4, next, bb, after);
		after = true;
		string push = "str r" + to_string(*it) + ", [r" + to_string(this->stackPointer) + "]";
		pushPopStrings.push_back(push);
		next = AsmGen::emitAsmInput(pushPopStrings[pushPopStrings.size()-1].c_str(), next, bb, after);
	}
}

/**
 * Function to emit the necessary POP instruction
 * Emits LDR reg, [r6] for each necessary register (reg)
 * Uses the pushPopStrings variable and the emitAsmInput method of AsmGen class.
 */
void ARMv6M_Functions::insertPop(vector<unsigned int> regs, rtx_insn* last, basic_block bb){
	bool after = false;
	// 1) emit a pop for each register
	vector<unsigned int>::const_iterator it;
	for(it = regs.begin(); it != regs.end(); it++){
		string popBB = "ldr r" + to_string(*it) + ", [r" + to_string(this->stackPointer) + "]";
		pushPopStrings.push_back(popBB);
		last = AsmGen::emitAsmInput(pushPopStrings[pushPopStrings.size()-1].c_str(), last, bb, after);
		after = true;
		last = AsmGen::emitAddRegInt(this->stackPointer, 4, last, bb, after);
	}
}
