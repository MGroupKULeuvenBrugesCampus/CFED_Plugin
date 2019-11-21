/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "RSCFC.h"
#include "primeNumbers.h"
#include "AsmGen.h"
#include "UpdatePoint.h"
#include "InstrType.h"

/**
 * Constructor, initializes necessary variables
 */
RSCFC::RSCFC(ARM_ISA* isa, unsigned int nrOfRegsToUse, bool intraBlockDet)
	:GeneralCFED(isa, nrOfRegsToUse){
	this->intraBlockDet = intraBlockDet;
}

/**
 * Function to calculate / assign the
 *  - compile-time signatures for each basic block
 *  - CFG Locator for each basic block
 */
void RSCFC::calcVariables(){
	// intra block part
	countNrOfVerifiableInstruction();
	// Signature monitoring part
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		unsigned int si = 1 << (n_basic_blocks_for_fn(cfun)-2);
		edge e;
		edge_iterator ei;
		FOR_EACH_EDGE(e, ei, bb->succs){
			unsigned int leftShift = (e->dest)->index-2;
			if(leftShift < (last_basic_block_for_fn(cfun))){
				si |= 1 << leftShift;
			}
		}
		signatures[(bb->index)-2] = si;
		CFGLocator.push_back( 1 << (bb->index)-2);
	}
}

/**
 * Function to insert the necessary intra-block CFE detection instructions
 * Inserts
 * 	EOR r10, #<exorVal> -> after each original instruction
 *
 * 	MOV r10, #<mask> -> before the first original instruction
 */
void RSCFC::insertIntraBlockJumpDetection(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	// only if there is at least 1 verifiable instruction
	if(nrOfVerifiableInstructions[idBB] > 0){
		// First, insert the EOR operations after each instruction in the bb, unless the instruction
		// * is a debug instruction that is not translated into assembly
		// * is a USE instruction that is not translated into assembly
		// * is a jump instruction
		unsigned int instrIndex = 0;
		rtx_insn* insn;
		FOR_BB_INSNS(bb, insn){
			if((NONDEBUG_INSN_P(insn)) && (!InstrType::isUse(insn)) && (!JUMP_P(insn)) && (!CALL_P(insn)) && (instrIndex < nrOfVerifiableInstructions[idBB]) && (instrIndex < 32) ){ //&& (!UpdatePoint::isCompare(insn))
				unsigned int exorVal = (1 << instrIndex);
				insn = AsmGen::emitEorRegInt(regsToUse[1], exorVal, insn, bb, true);
				instrIndex++;
			}
		}
		// Next, insert the Update instruction: r12 = #instr
		insertInstructionCounterUpdate(idBB, bb);
	}
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the beginning of each basic block
 * Inserts:
 * 	AND r11, #<CFGlocator>
 * 	CMP r11, #0
 * 	BEQ .codeLabel
 */
void RSCFC::insertBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	rtx_insn* andR = AsmGen::emitAndRegInt(regsToUse[0], CFGLocator[idBB], attachBefore, bb, false);
	rtx_insn* cmp = AsmGen::emitCmpRegInt(regsToUse[0], 0, andR, bb, true);
	AsmGen::emitBeq(codeLabel, cmp, bb, true);
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * in the middle of each basic block
 */
void RSCFC::insertMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	// Nothing to do for RSCFC
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the end of each basic block
 * Inserts
 * 	EOR r11, #<CFGlocator>
 * 	MVN r11, r11
 * 	AND r11, #<compileTimeSignature>
 *
 * 	or
 *
 * 	EOR r11, #<CFGlocator>
 * 	MVN r11, r11
 * 	MVN r10, r10
 * 	AND r11, r10
 * 	AND r11, #<compileTimeSignature>
 */
void RSCFC::insertEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	rtx_insn* prev = UpdatePoint::lastRealINSN(bb);
	// the updates are only necessary if there is a successor that will validate them.
	// Therefore we do not insert them in in return BBs
	if( !InstrType::isExitBlock(bb) ){
		// Decide whether to add the instruction in front or after the last insn
		if(JUMP_P(prev)){
			prev = AsmGen::emitEorRegInt(regsToUse[0], CFGLocator[idBB], prev, bb, false);
		}
		else{
			prev = AsmGen::emitEorRegInt(regsToUse[0], CFGLocator[idBB], prev, bb, true);
		}
		prev = emitMVN(regsToUse[0], prev, bb);
		// r12 must only be used if it has been updated throughout the BB
		if( intraBlockDet == true && nrOfVerifiableInstructions[idBB] != 0 ){
			prev = emitMVN(regsToUse[1], prev, bb);
			prev = emitAnd(prev, bb);
		}
		AsmGen::emitAndRegInt(regsToUse[0], signatures[idBB], prev, bb, true);
	}
}

/**
 * Function to insert the necessary setup code at the beginning of the
 * first basic block
 * Inserts
 * 	MOV r11, #1
 */
void RSCFC::insertSetup(){
	basic_block bb = BASIC_BLOCK_FOR_FN(cfun ,2);
	rtx_insn* prev = UpdatePoint::firstRealINSN(bb);
	AsmGen::emitMovRegInt(regsToUse[0], 1, prev, bb, false);
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the beginning of the basic block
 * Inserts
 * 	AND r11, #<CFGlocator>
 * 	CMP r11, #0 -> from here only in exit basic blocks
 * 	BEQ .codeLabel
 */
void RSCFC::insertSelBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	rtx_insn* andR = AsmGen::emitAndRegInt(regsToUse[0], CFGLocator[idBB], attachBefore, bb, false);
	if (InstrType::isExitBlock(bb)){
		rtx_insn* cmp = AsmGen::emitCmpRegInt(regsToUse[0], 0, andR, bb, true);
		AsmGen::emitBeq(codeLabel, cmp, bb, true);
	}
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions in the middle of the basic block
 */
void RSCFC::insertSelMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	// Nothing to do for S-RSCFC
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the end of the basic block
 * Is equal to the full implementation, so just calls that method
 */
void RSCFC::insertSelEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	insertEnd(idBB, bb, codeLabel);
}

/**
 * Function to count the number of original instructions in each basic
 * block that must be verified by the intra-block CFE detection instructions
 * So only non-debug, non-use, non-jump and non-call instructions.
 */
void RSCFC::countNrOfVerifiableInstruction(){
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
 * Emits MOV r10, #<mask>
 * Is inserted before the first instruction in the basic block
 * mask = 0xff... with a 1 for each verifiable instruction in the basic block
 */
void RSCFC::insertInstructionCounterUpdate(unsigned int idBB, basic_block bb){
	// create the initial counter value
	unsigned int mask = 0;
	for(int i = 0; i < nrOfVerifiableInstructions[idBB]; i++){
		mask |= (1<<i);
	}
	// insert update instruction as first instruction in the BB
	rtx_insn* firstInsn = UpdatePoint::firstRealINSN(bb);
	AsmGen::emitMovRegInt(regsToUse[1], mask, firstInsn, bb, false);
}

/**
 * Emits MVN reg, reg using the provided regNr
 */
rtx_insn* RSCFC::emitMVN(unsigned int regNr, rtx_insn* prev, basic_block bb){
	rtx reg = gen_rtx_REG(SImode, regNr);
	rtx notRtx = gen_rtx_NOT(SImode, reg);
	rtx set = gen_rtx_SET(reg, notRtx);
	return emit_insn_after_noloc(set , prev, bb);
}

/**
 * Emits AND r11, r10
 */
rtx_insn* RSCFC::emitAnd(rtx_insn* previous, basic_block bb){
	rtx reg11 = gen_rtx_REG(SImode, regsToUse[0]);
	rtx reg12 = gen_rtx_REG(SImode, regsToUse[1]);
	rtx andRtx = gen_rtx_AND(SImode, reg11, reg12);
	rtx set = gen_rtx_SET(reg11, andRtx);
	return emit_insn_after_noloc(set, previous, bb);
}
