/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>
#include <emit-rtl.h>

#include "AsmGen.h"

/**
 * Emits: CMP reg,#number
 */
rtx_insn* AsmGen::emitCmpRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx regSig = gen_rtx_REG(SImode, regNumber);
	rtx constInt = createConstInt(number);
	rtx cmp = gen_rtx_COMPARE(CCmode, regSig, constInt);
	rtx regCC = gen_rtx_REG(CCmode, CC_REGNUM);
	rtx set = gen_rtx_SET(regCC, cmp);
	rtx_insn* insn = emitInsn(set, attachRtx, bb, after);
	return insn;
}

/**
 * Emits: CMP reg,reg
 */
rtx_insn* AsmGen::emitCmpRegReg(unsigned int reg1, unsigned int reg2, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx regRtx1 = gen_rtx_REG(SImode, reg1);
	rtx regRtx2 = gen_rtx_REG(SImode, reg2);
	rtx cmp = gen_rtx_COMPARE(CCmode, regRtx1, regRtx2);
	rtx regCC = gen_rtx_REG(CCmode, CC_REGNUM);
	rtx set = gen_rtx_SET(regCC, cmp);
	return emitInsn(set, attachRtx, bb, after);
}

/**
 * Emits: AND reg,#number
 */
rtx_insn* AsmGen::emitAndRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx regSig = gen_rtx_REG(SImode, regNumber);
	rtx constInt = createConstInt(number);
	rtx andR = gen_rtx_AND(SImode, regSig, constInt);
	rtx set = gen_movsi(regSig, andR);
	rtx_insn* insn = emitInsn(set, attachRtx, bb, after);
	return insn;
}

/**
 * Emits: EOR reg,#number
 */
rtx_insn* AsmGen::emitEorRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx reg = gen_rtx_REG(SImode, regNumber);
	rtx constInt = createConstInt(number);
	rtx xorRtx = gen_rtx_XOR(SImode, reg, constInt);
	rtx set = gen_movsi(reg, xorRtx);
	rtx_insn* insn = emitInsn(set, attachRtx, bb, after);
	return insn;
}

/**
 * Emits: UDIV reg,reg,reg
 */
rtx_insn* AsmGen::emitUdivRegRegReg(unsigned int destReg, unsigned int dividendReg, unsigned int divisorReg,
									rtx_insn* attachRtx, basic_block bb, bool after){
	rtx destRegRtx = gen_rtx_REG(SImode, destReg);
	rtx dividentRegRtx = gen_rtx_REG(SImode, dividendReg);
	rtx divisorRegRTX = gen_rtx_REG(SImode, divisorReg);
	rtx udiv = gen_rtx_UDIV(SImode, dividentRegRtx, divisorRegRTX);
	rtx set = gen_movsi(destRegRtx, udiv);
	rtx_insn* insn = emitInsn(set, attachRtx, bb, after);
	return insn;
}

/**
 * Emits: MUL reg,reg,reg
 */
rtx_insn* AsmGen::emitMulRegReg(unsigned int destReg, unsigned int operand1, unsigned int operand2,
								rtx_insn* attachRtx, basic_block bb, bool after){
	rtx destRegRtx = gen_rtx_REG(SImode, destReg);
	rtx operand1Rtx = gen_rtx_REG(SImode, operand1);
	rtx operand2Rtx = gen_rtx_REG(SImode, operand2);
	rtx mult = gen_rtx_MULT(SImode, operand1Rtx, operand2Rtx);
	rtx set = gen_movsi(destRegRtx, mult);
	return emitInsn(set, attachRtx, bb, after);
}

/**
 * Emits: SUB reg,#number
 */
rtx_insn* AsmGen::emitSubRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx reg = gen_rtx_REG(SImode, regNumber);
	rtx constInt = createConstInt(number);
	rtx sub = gen_rtx_MINUS(SImode, reg, constInt);
	rtx set = gen_movsi(reg, sub);
	rtx_insn* insn = emitInsn(set, attachRtx, bb, after);
	return insn;
}

/**
 * Emits: ADD reg,#number
 */
rtx_insn* AsmGen::emitAddRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx reg = gen_rtx_REG(SImode, regNumber);
	rtx constInt = createConstInt(number);
	rtx add = gen_rtx_PLUS(SImode, reg, constInt);
	rtx set = gen_movsi(reg, add);
	rtx_insn* insn = emitInsn(set, attachRtx, bb, after);
	return insn;
}

/**
 * Emits: SUB<cond> reg,#number
 */
rtx_insn* AsmGen::emitCondSubRegInt(unsigned int regNumber, int number, rtx_code condition, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx condRTX = createCondition(condition);
	rtx reg = gen_rtx_REG(SImode, regNumber);
	rtx constInt = createConstInt(number);
	rtx sub = gen_rtx_MINUS(SImode, reg, constInt);
	rtx set = gen_rtx_SET(reg, sub);
	rtx_insn* insn = emitInsn(gen_rtx_COND_EXEC(VOIDmode, condRTX, set), attachRtx, bb, after);
	return insn;
}

/**
 * Emits: ADD<cond> reg,#number
 */
rtx_insn* AsmGen::emitCondAddRegInt(unsigned int regNumber, int number, rtx_code condition, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx condRTX = createCondition(condition);
	rtx reg = gen_rtx_REG(SImode, regNumber);
	rtx constInt = createConstInt(number);
	rtx add = gen_rtx_PLUS(SImode, reg, constInt);
	rtx set = gen_rtx_SET(reg, add);
	rtx_insn* insn = emitInsn(gen_rtx_COND_EXEC(VOIDmode, condRTX, set), attachRtx, bb, after);
	return insn;
}

/**
 * Emits: MOV reg,#number
 */
rtx_insn* AsmGen::emitMovRegInt(unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx regSig = gen_rtx_REG(SImode, regNumber);
	rtx constInt = createConstInt(number);
	rtx_insn* insn = emitInsn(gen_movsi(regSig, constInt), attachRtx, bb, after);
	return insn;
}

/**
 * Emits: MOV reg,reg
 */
rtx_insn* AsmGen::emitMovRegReg(unsigned int destReg, unsigned int srcReg, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx regDest = gen_rtx_REG(SImode, destReg);
	rtx regSrc = gen_rtx_REG(SImode, srcReg);
	rtx_insn* insn = emitInsn(gen_movsi(regDest, regSrc), attachRtx, bb, after);
	return insn;
}

/**
 * Emits: MOV<cond> reg,#number
 */
rtx_insn* AsmGen::emitCondMovRegInt(rtx_code condition, unsigned int regNumber, int number, rtx_insn* attachRtx, basic_block bb, bool after){
	// create and get the condition rtx
	rtx conditionRTX = createCondition(condition);
	// create MOV
	rtx regSig = gen_rtx_REG(SImode, regNumber);
	rtx constInt = createConstInt(number);
	rtx set = gen_rtx_SET(regSig, constInt);
	// Create insn
	rtx_insn* insn = emitInsn(gen_rtx_COND_EXEC(VOIDmode, conditionRTX, set), attachRtx, bb, after);
	return insn;
}

/**
 * Emits: .codeLabel
 */
rtx_insn* AsmGen::emitCodeLabel(unsigned int insnID, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx_insn* next = NEXT_INSN(attachRtx);
	rtx codeLab = gen_label_rtx();
	rtx_insn* insn = emitLabel(codeLab, attachRtx, after);
	return insn;
}

/**
 * Emits: BEQ .codeLabel
 */
rtx_insn* AsmGen::emitBeq(rtx_insn* codeLabel, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx regCC = gen_rtx_REG(CCmode, CC_REGNUM);
	rtx constInt = gen_rtx_CONST_INT(VOIDmode, 0);
	rtx eq = gen_rtx_EQ(CCmode,regCC,constInt);
	rtx_insn* insn = emitInsn(gen_arm_cond_branch(codeLabel, eq, regCC), attachRtx, bb, after);
	return insn;
}

/**
 * Emits: BNE .codeLabel (armV7-M syntax)
 */
rtx_insn* AsmGen::emitBne(rtx_insn* codeLabel, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx regCC = gen_rtx_REG(CCmode, CC_REGNUM);
	rtx constInt = gen_rtx_CONST_INT(VOIDmode, 0);
	rtx ne = gen_rtx_NE(CCmode,regCC,constInt);
	rtx_insn* insn = emitInsn(gen_arm_cond_branch(codeLabel, ne, regCC), attachRtx, bb, after);
	return insn;
}

/**
 * Emits: BHS .codelabel
 */
rtx_insn* AsmGen::emitBhs(rtx_insn* codeLabel, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx regCC = gen_rtx_REG(CCmode, CC_REGNUM);
	rtx constInt = gen_rtx_CONST_INT(VOIDmode, 0);
	rtx hs = gen_rtx_GEU(CCmode, regCC, constInt);
	return emitInsn(gen_arm_cond_branch(codeLabel, hs, regCC), attachRtx, bb, after);
}

/**
 * Emits: CMP reg,#number
 * 		  BNE .codelabel (armV6-M syntax)
 */
rtx_insn* AsmGen::emitBne(unsigned int regNumber, int cmpNumber, rtx_insn* codeLabel, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx reg = gen_rtx_REG(SImode, regNumber);
	rtx constInt = createConstInt(cmpNumber);
	rtx ne = gen_rtx_NE(SImode, reg, constInt);
	rtx ITE = gen_rtx_IF_THEN_ELSE(VOIDmode, ne, gen_rtx_LABEL_REF(VOIDmode, codeLabel), pc_rtx);
	rtx_insn* branch = emitInsn(gen_movsi(pc_rtx, ITE), attachRtx, bb, after);
	return branch;
}

/**
 * Emits: BL CFED_Detected
 *  = Emits the call to CFED_Detected function, which
 *  is the error handler to execute when a CFE is detected
 */
rtx_insn* AsmGen::emitCall(rtx_insn* codeLabel){
	rtvec vec = rtvec_alloc(3);
	rtx constInt = gen_rtx_CONST_INT(VOIDmode, 0);
	// Create call
	rtx symbol = gen_rtx_SYMBOL_REF(SImode, "CFED_Detected");
	rtx mem = gen_rtx_MEM(SImode, symbol);
	rtx call = gen_rtx_CALL(VOIDmode, mem, constInt);
	// Create use
	rtx use = gen_rtx_USE(VOIDmode, constInt);
	// Create clobber
	rtx clob = gen_rtx_CLOBBER(VOIDmode,(gen_rtx_REG(SImode, LR_REGNUM)));
	// Create parallel
	vec->elem[0] = call;
	vec->elem[1] = use;
	vec->elem[2] = clob;
	rtx par = gen_rtx_PARALLEL(VOIDmode, vec);
	// Emit
	return emit_call_insn_after_noloc(par, codeLabel);
}

/**
 * Emits the provided assembly instruction
 */
rtx_insn* AsmGen::emitAsmInput(const char* asmInstr, rtx_insn* attachRtx, basic_block bb, bool after){
	rtx asmBxLR = gen_rtx_ASM_INPUT_loc(VOIDmode, asmInstr, 1);
	asmBxLR->volatil=1;
	rtx memRTX = gen_rtx_MEM(BLKmode, gen_rtx_SCRATCH(VOIDmode));
	rtx clobber = gen_rtx_CLOBBER(VOIDmode, memRTX);
	rtvec vec = rtvec_alloc(2);
	vec->elem[0] = asmBxLR;
	vec->elem[1] = clobber;
	rtx par = gen_rtx_PARALLEL(VOIDmode, vec);
	rtx_insn* insn = emitInsn(par, attachRtx, bb, after);
	return insn;
}

//------------------------------ Private Section --------------------- \\

/*
 * Actually emits the insn at the desired place.
 */
rtx_insn* AsmGen::emitInsn(rtx rtxInsn,rtx_insn* attachRtx, basic_block bb, bool after){
	if (after){
		return emit_insn_after_noloc(rtxInsn, attachRtx, bb);
	}
	else{
		return emit_insn_before_noloc(rtxInsn, attachRtx, bb);
	}
}

/*
 * Actually emits the codelabel at the desired place
 */
rtx_insn* AsmGen::emitLabel(rtx label, rtx_insn* attachRtx, bool after){
	if(after){
		return emit_label_after(label, attachRtx);
	}
	else{
		return emit_label_before(label, attachRtx);
	}
}

/**
 * Method to easily create a CONST_INT rtx
 */
rtx AsmGen::createConstInt(int number){
	rtx constInt = rtx_alloc(CONST_INT);
	XWINT(constInt,0) = number;
	return constInt;
}

/**
 * Method to create an rtx with the desired condition
 */
rtx AsmGen::createCondition(rtx_code condition){
	// create basic structures
	rtx regCC = gen_rtx_REG(CCmode, CC_REGNUM);
	rtx constInt = gen_rtx_CONST_INT(VOIDmode, 0);
	// Create condition rtx
	rtx conditionRTX = NULL;
	switch(condition){
		case EQ:
			conditionRTX = gen_rtx_EQ(VOIDmode, regCC, constInt);
			break;
		case NE:
			conditionRTX = gen_rtx_NE(VOIDmode, regCC, constInt);
			break;
		case GT:
			conditionRTX = gen_rtx_GT(VOIDmode, regCC, constInt);
			break;
		case GTU:
			conditionRTX = gen_rtx_GTU(VOIDmode, regCC, constInt);
			break;
		case LT:
			conditionRTX = gen_rtx_LT(VOIDmode, regCC, constInt);
			break;
		case LTU:
			conditionRTX = gen_rtx_LTU(VOIDmode, regCC, constInt);
			break;
		case GE:
			conditionRTX = gen_rtx_GE(VOIDmode, regCC, constInt);
			break;
		case GEU:
			conditionRTX = gen_rtx_GEU(VOIDmode, regCC, constInt);
			break;
		case LE:
			conditionRTX = gen_rtx_LE(VOIDmode, regCC, constInt);
			break;
		case LEU:
			conditionRTX = gen_rtx_LEU(VOIDmode, regCC, constInt);
			break;
		default:
			break;
	}
	return conditionRTX;
}
