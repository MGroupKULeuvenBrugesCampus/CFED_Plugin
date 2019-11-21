/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "ARMv7M_Functions.h"
#include "AsmGen.h"
#include "InstrType.h"


ARMv7M_Functions::ARMv7M_Functions(processor_type cpu)
	:ARM_ISA(cpu) {}

/**
 * Function to split
 * 	- CBZ into a CMP and a BEQ instruction;
 * 	- CBNZ into a CMP and a BNE instruction;
 * Necessary for certain CFE detection methods that
 * use conditional updating of a control variable.
 */
void ARMv7M_Functions::changeCBZ(){
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		rtx_insn* insn;
		FOR_BB_INSNS(bb, insn){
			if(InstrType::isCBZ(insn)){
				rtx par = XEXP(insn, 3);
				rtx set = XVECEXP(par, 0,0);
				rtx ifThenElse = XEXP(set, 1);
				rtx condRTX = XEXP(ifThenElse, 0);
				rtx label = XEXP(ifThenElse, 1);
				unsigned int labelNr = XINT(XEXP(label,0), 5);
				rtx regRtx = XEXP(condRTX, 0);
				unsigned int regNr = XINT(regRtx, 0);
				rtx_code cond = InstrType::getCondCode(insn);
				AsmGen::emitCmpRegInt(regNr, 0, insn, bb, false);
				rtx_insn* labelToUse = findLabel(labelNr);
				if (cond == EQ){
					XEXP(insn,3) = createBeq(labelToUse);
				}
				else{
					XEXP(insn, 3) = createBne(labelToUse);
				}
				(insn->u).fld[5].rt_int = -1;
			}
		}
	}
}

/**
 * Function to emit the necessary PUSH instruction
 * Emits STMDB r6!, {<reglist>} with each necessary register contained in <reglist>
 * Uses the pushPopStrings variable and the emitAsmInput method of AsmGen class.
 */
void ARMv7M_Functions::insertPush(vector<unsigned int> regs){
	// 1) Get necessary emit variables
	rtx_insn* next = get_first_nonnote_insn();
	basic_block bb = BASIC_BLOCK_FOR_FN(cfun,2);

	// 2) Emit the push
	string push = "r" + to_string(regs[regs.size()-1]);
	for(int i = regs.size()-2; i > -1 ; i--){
		push += ", r" + to_string(regs[i]);
	}
	string fullPush = "STMDB r" + to_string(this->stackPointer) + "!, {" + push + "}";
	pushPopStrings.push_back(fullPush);
	AsmGen::emitAsmInput(pushPopStrings[pushPopStrings.size()-1].c_str(), next, bb, false);
}

/**
 * Function to emit the necessary POP instruction
 * Emits LDMIA r6!, {<reglist} with each necessary register contained in <reglist>
 * Uses the pushPopStrings variable and the emitAsmInput method of AsmGen class.
 */
void ARMv7M_Functions::insertPop(vector<unsigned int> regs, rtx_insn* last, basic_block bb){
	string pop = "r" + to_string(regs[regs.size()-1]);
	for(int i = regs.size()-2; i > -1 ; i--){
		pop += ", r" + to_string(regs[i]);
	}
	string popPush = "LDMIA r" + to_string(this->stackPointer) + "!, {" + pop + "}";
	pushPopStrings.push_back(popPush);
	AsmGen::emitAsmInput(pushPopStrings[pushPopStrings.size()-1].c_str(), last, bb, false);
}

/**
 * Emits BEQ .codelabel
 */
rtx ARMv7M_Functions::createBeq(rtx_insn* codeLabel){
	rtx regCC = gen_rtx_REG(CCmode, CC_REGNUM);
	rtx constInt = gen_rtx_CONST_INT(VOIDmode, 0);
	rtx eq = gen_rtx_EQ(CCmode,regCC,constInt);
	return gen_arm_cond_branch(codeLabel, eq, regCC);
}

/**
 * Emits BNE .codelabel
 */
rtx ARMv7M_Functions::createBne(rtx_insn* codeLabel){
	rtx regCC = gen_rtx_REG(CCmode, CC_REGNUM);
	rtx constInt = gen_rtx_CONST_INT(VOIDmode, 0);
	rtx ne = gen_rtx_NE(CCmode,regCC,constInt);
	return gen_arm_cond_branch(codeLabel, ne, regCC);
}

/**
 * Function to find the label rtx_insn containing the specified number
 * Used to split CBZ and CBNZ instructions.
 */
rtx_insn* ARMv7M_Functions::findLabel(unsigned int number){
	basic_block bb;
	rtx_insn* insn;
	FOR_EACH_BB_FN(bb, cfun){
		FOR_BB_INSNS(bb, insn){
			if(LABEL_P(insn)){
				int currentNr = XINT(insn,5);
				if(currentNr == number){
					return insn;
				}
			}
		}
	}
	return 0x00;
}
