/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "SEDSR.h"
#include "AsmGen.h"
#include "UpdatePoint.h"
#include "InstrType.h"

/**
 * Constructor, initializes necessary variables
 */
SEDSR::SEDSR(ARM_ISA* isa, unsigned int nrOfRegsToUse)
	:GeneralCFED(isa, nrOfRegsToUse){
}

/**
 * Function to calculate / assign the
 * 	- compile-time signatures for each basic block
 */
void SEDSR::calcVariables(){
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		unsigned int si = 0;
		edge e;
		edge_iterator ei;
		FOR_EACH_EDGE(e, ei, bb->succs){
			if (e->src == bb){
				unsigned int leftShift = (e->dest)->index - 2;
				if(leftShift < (last_basic_block_for_fn(cfun))){
					si |= 1 << leftShift;
				}
			}
		}
		this->signatures[(bb->index) - 2] = si;
	}
}

/**
 * Function to insert the necessary intra-block CFE detection instructions
 */
void SEDSR::insertIntraBlockJumpDetection(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	throw "Intra-block CFE detection for SEDSR not officially supported and therefore not implemented!";
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the beginning of each basic block
 * Inserts:
 * 	AND r11, #<mask> -> mask = 1 << idBasicBlock
 * 	CMP r11, #0
 * 	BEQ .codeLabel
 */
void SEDSR::insertBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	//rtx_insn* prev = emitLSR(idBB, attachBefore, bb);
	rtx_insn* prev = AsmGen::emitAndRegInt(regsToUse[0], (1<<idBB), attachBefore, bb, false);
	prev = AsmGen::emitCmpRegInt(regsToUse[0], 0, prev, bb, true);
	AsmGen::emitBeq(codeLabel, prev, bb, true);
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * in the middle of each basic block
 * Inserts:
 * 	MOV r11, #<compileTimeSignatureBasicBlock>
 */
void SEDSR::insertMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	if(! InstrType::isExitBlock(bb)){
		if(this->nrOfOrigInstr[idBB] != 1){
			AsmGen::emitMovRegInt(regsToUse[0], signatures[idBB], attachAfter, bb, true);
		}
		else{
			AsmGen::emitMovRegInt(regsToUse[0], signatures[idBB], attachAfter, bb, false);
		}
	}
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the end of each basic block
 */
void SEDSR::insertEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	// Nothing to do for SEDSR
}

/**
 * Function to insert the necessary setup code at the beginning
 * of the first basic block
 * Inserts:
 * 	MOV r11, #1
 */
void SEDSR::insertSetup(){
	basic_block bb = BASIC_BLOCK_FOR_FN(cfun, 2);
	rtx_insn* prev = UpdatePoint::firstRealINSN(bb);
	AsmGen::emitMovRegInt(regsToUse[0], 1, prev, bb, false);
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the beginning of the basic block
 */
void SEDSR::insertSelBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	throw "Selective implementation for SEDSR not officially supported and therefore not implemented!";
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions in the middle of the basic block
 */
void SEDSR::insertSelMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	throw "Selective implementation for SEDSR not officially supported and therefore not implemented!";
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the end of the basic block
 */
void SEDSR::insertSelEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	throw "Selective implementation for SEDSR not officially supported and therefore not implemented!";
}

/**
 * Emits LSR r11, r10
 * Not used anymore
 */
rtx_insn* SEDSR::emitLSR(unsigned int idBB, rtx_insn* next, basic_block bb){
	rtx regSig = gen_rtx_REG(SImode, regsToUse[0]);
	rtx constInt = gen_rtx_CONST_INT(VOIDmode, idBB);
	return emit_insn_before_noloc(gen_rotrsi3(regSig, regSig, constInt), next,bb);
}
