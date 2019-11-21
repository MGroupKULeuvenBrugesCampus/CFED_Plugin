/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "ECCA.h"
#include "primeNumbers.h"
#include "AsmGen.h"
#include "UpdatePoint.h"
#include "InstrType.h"

/**
 * Constructor, initializes necessary variables
 */
ECCA::ECCA(ARM_ISA* isa, unsigned int nrOfRegsToUse)
	:GeneralCFED(isa, nrOfRegsToUse){
}

/**
 * Function to calculate / assign the
 * 	- compile-time signature for each basic block
 * 	- NEXT1 and NEXT2 for each basic block
 */
void ECCA::calcVariables(){
	for (int i = 0; i < n_basic_blocks_for_fn(cfun)-2; i++){
		signatures[i] = primeNumber[i];
	}

	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		unsigned int idBB = bb->index -2;
		createNext(idBB, bb);
	}
}

/**
 * Function to insert the necessary intra-block CFE detection instructions
 */
void ECCA::insertIntraBlockJumpDetection(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	throw "Intra-block CFE detection for ECCA not officially supported and therefore not implemented!";
}

/*
 * Function to insert the necessary inter-block CFE detection instructions
 * at the beginning of each basic block
 * Inserts:
 * 	SUB r11, #<compileTimeSignature>
 * 	SUB r10, #<compileTimeSignature>
 * 	MUL r11, r11, r10
 * 	CMP r11, #0
 * 	BNE .codelabel
 * 	MOV r10, r11, ror #31 -> from here, only if necessary
 * 	ADD r11, #1
 * 	ADD r10, #1
 * 	UDIV r11, r11, r10
 * 	MOV r10, #( <compileTimeSignature> + 1 )
 * 	UDIV r11, r10, r11
 */
void ECCA::insertBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	rtx_insn* prev = AsmGen::emitSubRegInt(regsToUse[0], signatures[idBB], attachBefore, bb, false);
	prev = AsmGen::emitSubRegInt(regsToUse[1], signatures[idBB], prev, bb, true);
	prev = insertMUL(idBB, prev, bb);
	prev = AsmGen::emitCmpRegInt(regsToUse[0], 0, prev, bb, true);
	prev = AsmGen::emitBne(codeLabel, prev, bb, true);
	if(!InstrType::isExitBlock(bb)){
		prev = insertLSL(idBB, prev, bb);
		prev = AsmGen::emitAddRegInt(regsToUse[0], 1, prev, bb, true);
		prev = AsmGen::emitAddRegInt(regsToUse[1], 1, prev, bb, true);
		prev = AsmGen::emitUdivRegRegReg(regsToUse[0], regsToUse[0], regsToUse[1], prev, bb, true);
		prev = AsmGen::emitMovRegInt(regsToUse[1], signatures[idBB]+1, prev, bb, true);
		prev = AsmGen::emitUdivRegRegReg(regsToUse[0], regsToUse[1], regsToUse[0],prev, bb, true);
	}
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * in the middle of each basic block
 */
void ECCA::insertMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	// Nothing to do for ECCA
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the end of each basic block
 * Inserts -> if necessary
 * 	SUB r11, #( <compileTimeSignature> + 1 )
 * 	ADD r10, r11, #<NEXT2>
 * 	ADD r11, #<NEXT1>
 */
void ECCA::insertEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	if(!InstrType::isExitBlock(bb)){
		rtx_insn* prev = UpdatePoint::lastRealINSN(bb);
		prev = AsmGen::emitSubRegInt(regsToUse[0], signatures[idBB]+1, prev, bb, !JUMP_P(prev));
		prev = insertADD(idBB, prev, bb);
		prev = AsmGen::emitAddRegInt(regsToUse[0], nextValues[idBB].Next1, prev, bb, true);
	}
}

/**
 * Function to insert the necessary setup code at the beginning of
 * the first basic block
 * Inserts
 * 	MOV r11, #<compileTimeSignatureFirstBasicBlock>
 */
void ECCA::insertSetup(){
	basic_block bb = BASIC_BLOCK_FOR_FN(cfun, 2);
	rtx_insn* prev = UpdatePoint::firstRealINSN(bb);
	AsmGen::emitMovRegInt(regsToUse[0], signatures[0], prev, bb, false);
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the beginning of the basic block
 */
void ECCA::insertSelBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	throw "Selective implementation for ECCA not officially supported and therefore not implemented!";
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions in the middle of the basic block
 */
void ECCA::insertSelMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	throw "Selective implementation for ECCA not officially supported and therefore not implemented!";
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the end of the basic block
 */
void ECCA::insertSelEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	throw "Selective implementation for ECCA not officially supported and therefore not implemented!";
}

/**
 * Function to determine the NEXT1 and NEXT2 values
 * for each basic block
 */
void ECCA::createNext(unsigned int idBB, basic_block bb){
	ECCAnext nextVal;
	nextVal.Next1 = 0;
	nextVal.Next2 = 0;

	if (!InstrType::isExitBlock(bb)){
		edge e;
		edge_iterator ei;
		FOR_EACH_EDGE(e, ei, bb->succs){
			unsigned int idSuccs = (e->dest)->index -2;
			if(idSuccs == (idBB+1)){
				nextVal.Next1 = signatures[idSuccs];
			}
			else if (idSuccs <= n_basic_blocks_for_fn(cfun)-2){
				nextVal.Next2 = signatures[idSuccs];
			}
		}
	}
	nextValues.push_back(nextVal);
}

/**
 * Emits MUL r11, r11, r10
 */
rtx_insn* ECCA::insertMUL(unsigned int idBB, rtx_insn* previous, basic_block bb){
	rtx destReg = gen_rtx_REG(SImode, regsToUse[0]);
	rtx operand = gen_rtx_REG(SImode, regsToUse[1]);
	rtx mult = gen_rtx_MULT(SImode, destReg,operand);
	rtx set = gen_movsi(destReg, mult);
	return emit_insn_after_noloc(set, previous, bb);
}

/**
 * Emits MOV r10, r11, ror #31
 */
rtx_insn* ECCA::insertLSL(unsigned int idBB, rtx_insn* previous, basic_block bb){
	rtx destReg = gen_rtx_REG(SImode, regsToUse[1]);
	rtx srcReg = gen_rtx_REG(SImode, regsToUse[0]);
	rtx constInt= gen_rtx_CONST_INT(VOIDmode, 1);
	rtx lsl = gen_rtx_ROTATE(SImode, srcReg, constInt);
	rtx set = gen_movsi(destReg, lsl);
	return emit_insn_after_noloc(set, previous, bb);
}

/**
 * Emits ADD r10, r11, #<NEXT2>
 */
rtx_insn* ECCA::insertADD(unsigned int idBB, rtx_insn* previous, basic_block bb){
	rtx destReg = gen_rtx_REG(SImode, regsToUse[1]);
	rtx srcReg = gen_rtx_REG(SImode, regsToUse[0]);
	rtx constInt = gen_rtx_CONST_INT(SImode, nextValues[idBB].Next2);
	rtx add = gen_rtx_PLUS(SImode, srcReg, constInt);
	rtx set = gen_movsi(destReg, add);
	return emit_insn_after_noloc(set, previous, bb);
}
