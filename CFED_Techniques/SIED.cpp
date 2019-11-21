/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "SIED.h"
#include "AsmGen.h"
#include "UpdatePoint.h"
#include "InstrType.h"

/**
 * Constructor, initializes necessary variables
 * Intra-block = regsToUSe [0], inter-block = regsToUse[1],[2]
 */
SIED::SIED(ARM_ISA* isa, unsigned int nrOfRegsToUse, bool intraDet)
	:GeneralCFED(isa, nrOfRegsToUse){
	this->intraDet = intraDet;
}

/**
 * Function to calculate / assign the
 * 	- compile-time signatures for each basic block
 * 	- the Y-values for each basic block
 */
void SIED::calcVariables(){
	// intra-block part
	countNrOfVerifiableInstruction();
	// inter-block part
	for(int i = 0; i < n_basic_blocks_for_fn(cfun)-2; i++){
		signatures[i] = (i+1)*5;
	}

	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		unsigned int idBB = bb->index -2;
		calcYvalues(idBB, bb);
	}
}
/**
 * Function to insert the necessary intra-block CFE detection instructions
 * Inserts:
 * 	SUB r11, #1 -> after each original verifiable instruction
 */
void SIED::insertIntraBlockJumpDetection(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	// only if there is at least 1 verifiable instruction
	if(nrOfVerifiableInstructions[idBB] > 0){
		// First, insert the SUB operations after each instruction in the bb, unless the instruction
		// * is a debug instruction that is not translated into assembly
		// * is a USE instruction that is not translated into assembly
		// * is a jump instruction
		unsigned int instrIndex = 0;
		rtx_insn* insn;
		FOR_BB_INSNS(bb, insn){
			if((NONDEBUG_INSN_P(insn)) && (!InstrType::isUse(insn)) && (!JUMP_P(insn)) && (!CALL_P(insn)) && (instrIndex < nrOfVerifiableInstructions[idBB])  ){ //&& (!UpdatePoint::isCompare(insn))
				insn = AsmGen::emitSubRegInt(regsToUse[0], 1, insn, bb, true);
				instrIndex++;
			}
		}
		// Next, insert the Update instruction: r10 = #instr
		rtx_insn* firstInsn = UpdatePoint::firstRealINSN(bb);
		AsmGen::emitMovRegInt(regsToUse[0], nrOfVerifiableInstructions[idBB], firstInsn, bb, false);
	}
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the beginning of each basic block
 * Inserts:
 * 	CMP r11, #0 -> only if intra-block detection is enabled
 * 	BNE .codeLabel
 * 	ADD r10, #<compileTimeSignatureBasicBlock> -> From here, in all basic blocks
 * 	CMP r10, r9
 * 	BNE .codeLabel
 */
void SIED::insertBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	rtx_insn* prev = attachBefore;
	if (intraDet){
		if(idBB != 0){
			prev = AsmGen::emitCmpRegInt(regsToUse[0], 0, prev, bb, false);
			prev = AsmGen::emitBne(codeLabel, prev, bb, true);
		}
	}
	// Inter-block verification
	prev = AsmGen::emitAddRegInt(regsToUse[1], signatures[idBB], prev, bb, (intraDet && nrOfVerifiableInstructions[idBB] > 0) );
	prev = insertCMP(prev, bb);
	AsmGen::emitBne(codeLabel, prev, bb, true);
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * in the middle of each basic block
 */
void SIED::insertMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	// Nothing to do for SIED
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the end of each basic block
 * Inserts:
 * 	MOV<true> r10, #1 -> if basic block ends with conditional jump
 * 	MOV<true> r9, #<trueYvalue>
 * 	MOV<false> r10, #0
 * 	MOV<false> r9, #<falseYvalue>
 *
 * 	OR
 *
 * 	MOV r10, #0 -> other basic blocks
 * 	MOV r9, #<Yvalue>
 */
void SIED::insertEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	if (!InstrType::isExitBlock(bb)){
		// Inter-block only
		rtx_insn* last = UpdatePoint::lastRealINSN(bb);
		rtx_insn* jmp = UpdatePoint::lastRealINSN(bb);

		if(JUMP_P(last)){
			if(InstrType::isCondJump(last)){
				rtx_code trueCode = InstrType::getCondCode(last);
				rtx_code falseCode = InstrType::findContraryConditionalCode(trueCode);
				last = AsmGen::emitCondMovRegInt(trueCode, regsToUse[1], 1, last, bb, false);
				last = AsmGen::emitCondMovRegInt(trueCode, regsToUse[2], branchSigs[idBB].trueBranch, last, bb, true);
				last = AsmGen::emitCondMovRegInt(falseCode, regsToUse[1], 0, last, bb, true);
				AsmGen::emitCondMovRegInt(falseCode, regsToUse[2], branchSigs[idBB].falseBranch, last, bb, true);
			}
			else{
				last = AsmGen::emitMovRegInt(regsToUse[1], 0, last, bb, false);
				AsmGen::emitMovRegInt(regsToUse[2], branchSigs[idBB].trueBranch, last, bb, true);
			}
		}
		else{
			last = AsmGen::emitMovRegInt(regsToUse[1], 0, last, bb, true);
			AsmGen::emitMovRegInt(regsToUse[2], branchSigs[idBB].falseBranch, last, bb, true);
		}
	}
}

/**
 * Function to insert the necessary setup code at the beginning
 * of the first basic block
 * Inserts:
 * 	MOV r10, #0
 * 	MOV r9, #<compileTimeSignatureFirstBasicBlock>
 */
void SIED::insertSetup(){
	basic_block bb = BASIC_BLOCK_FOR_FN(cfun,2);
	rtx_insn* prev = UpdatePoint::firstRealINSN(bb);
	prev = AsmGen::emitMovRegInt(regsToUse[1], 0, prev, bb, false);
	AsmGen::emitMovRegInt(regsToUse[2], signatures[0], prev, bb, true);
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the beginning of the basic block
 * Inserts in exit blocks only:
 * 	CMP r11, #0 -> only if intra-block detection is enabled
 * 	BNE .codeLabel
 * 	ADD r10, #<compileTimeSignatureBasicBlock> -> From here, in all basic blocks
 * 	CMP r10, r9
 * 	BNE .codeLabel
 */
void SIED::insertSelBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	if(InstrType::isExitBlock(bb)){
		rtx_insn* prev = attachBefore;
		if (intraDet){
			if(idBB != 0){
				prev = AsmGen::emitCmpRegInt(regsToUse[0], 0, prev, bb, false);
				prev = AsmGen::emitBne(codeLabel, prev, bb, true);
			}
		}
		// Inter-block verification
		prev = AsmGen::emitAddRegInt(regsToUse[1], signatures[idBB], prev, bb, intraDet);
		prev = insertCMP(prev, bb);
		AsmGen::emitBne(codeLabel, prev, bb, true);
	}
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions in the middle of the basic block
 */
void SIED::insertSelMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	// Nothing to do for S-SIED
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the end of the basic block
 * Is equal to the full implementation, so just calls that method
 */
void SIED::insertSelEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	insertEnd(idBB, bb, codeLabel);
}

/**
 * Function to count the number of original instructions in each basic
 * block that must be verified by the intra-block CFE detection instructions
 * So only non-debug, non-use, non-jump and non-call instructions.
 */
void SIED::countNrOfVerifiableInstruction(){
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		unsigned int idBB = (bb->index) -2;
		unsigned int nr = 0;
		rtx_insn* insn;
		FOR_BB_INSNS(bb, insn){
			if((NONDEBUG_INSN_P(insn)) && (!InstrType::isUse(insn)) && (!JUMP_P(insn)) && (!CALL_P(insn)) ){ //&& (!UpdatePoint::isCompare(insn))
				nr++;
			}
		}
		nrOfVerifiableInstructions.push_back(nr);
	}
}

/**
 * Function to calculate the true and false Y values
 * for each basic block
 */
void SIED::calcYvalues(unsigned int idBB, basic_block bb){
	SIEDbranch yBranch;
	if (!InstrType::isExitBlock(bb)){
		unsigned int trueId = -1;
		unsigned int falseId = -1;

		edge e;
		edge_iterator ei;
		FOR_EACH_EDGE(e, ei, bb->succs){
			unsigned int idDest = (e->dest)->index -2;
			if(idDest == idBB+1){
				falseId = idDest;
			}
			else{
				trueId = idDest;
			}
		}

		if(falseId != -1 && trueId ==-1){	// Fallthrough
			yBranch.trueBranch = 0;
			yBranch.falseBranch = signatures[falseId];
		}
		else if(falseId == -1 && trueId != -1){
			yBranch.trueBranch = signatures[trueId];
			yBranch.falseBranch = 0;
		}
		else if (falseId == -1 && trueId == -1){
			yBranch.trueBranch = 0;
			yBranch.falseBranch = 0;
		}
		else{
			yBranch.trueBranch = signatures[trueId] + 1;
			yBranch.falseBranch = signatures[falseId];
		}
	}
	else{
		yBranch.trueBranch = 0;
		yBranch.falseBranch = 0;
	}
	branchSigs.push_back(yBranch);
}

/**
 * Emits CMP r10, r9
 */
rtx_insn* SIED::insertCMP(rtx_insn* previous, basic_block bb){
	rtx sigReg = gen_rtx_REG(SImode, regsToUse[1]);
	rtx branchReg = gen_rtx_REG(SImode, regsToUse[2]);
	rtx cmp = gen_rtx_COMPARE(CCmode, sigReg, branchReg);
	rtx regCC = gen_rtx_REG(CCmode, CC_REGNUM);
	rtx set = gen_rtx_SET(regCC, cmp);
	return emit_insn_after_noloc(set, previous, bb);
}
