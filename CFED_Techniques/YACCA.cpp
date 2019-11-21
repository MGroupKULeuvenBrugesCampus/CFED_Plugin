/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "YACCA.h"
#include "primeNumbers.h"
#include "AsmGen.h"
#include "UpdatePoint.h"
#include "InstrType.h"

/**
 * Constructor, initializes necessary variables
 */
YACCA::YACCA(ARM_ISA* isa, unsigned int nrOfRegsToUse)
	:GeneralCFED(isa, nrOfRegsToUse){
	// previousValues has to be filled with '1'
	for (int i = 0; i < n_basic_blocks_for_fn(cfun)-2; i++){
		previousValues.push_back(1);
	}
}

/**
 * Function to calculate / assign the
 * 	- compile-time signatures for each basic block
 * 	- PREVIOUS values for each basic block
 * 	- M1 values for each basic block
 * 	- M2 values for each basic block
 */
void YACCA::calcVariables(){
	for(int i = 0; i < n_basic_blocks_for_fn(cfun)-2; i++){
		signatures[i] = primeNumber[i];
	}

	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		unsigned int idBB = bb->index -2;
		calcPrevious(idBB, bb);
		calcM1(idBB, bb);
		calcM2(idBB, bb);
	}
	previousValues[0] = 0;
	M2Values[0] = 4;
}

/**
 * Function to insert the necessary intra-block CFE detection instructions
 */
void YACCA::insertIntraBlockJumpDetection(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	throw "Intra-block CFE detection for YACCA not officially supported and therefore not implemented!";
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the beginning of each basic block
 * Inserts:
 * 	MOV r10, #<previousValueBasicBlock>
 * 	generateTest instructions
 */
void YACCA::insertBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	rtx_insn* prev = AsmGen::emitMovRegInt(regsToUse[1], previousValues[idBB], attachBefore, bb, false);
	generateTest(idBB, bb, codeLabel, prev);
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * in the middle of each basic block
 */
void YACCA::insertMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	// Nothing to do for YACCA
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the end of each basic block
 * If the basic block is not an exit basic block and it has more than 1 instruction
 * then this function inserts:
 * 	generateTest instructions -> only if necessary
 * 	AND r11, #<M1valueBasicBlock> -> only if necessary
 * 	EOR r11, #<M2valueBasicBlock>
 */
void YACCA::insertEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	if( !(InstrType::isExitBlock(bb) && nrOfOrigInstr[idBB] == 1 ) ) {
		rtx_insn* prev = UpdatePoint::lastRealSafeINSN(bb);
		if(insertTestEnd(prev)){
			prev = generateTest(idBB, bb, codeLabel, prev);
		}
		if(M1Values[idBB] != -1){
			prev = AsmGen::emitAndRegInt(regsToUse[0], M1Values[idBB], prev, bb, true);
		}
		AsmGen::emitEorRegInt(regsToUse[0], M2Values[idBB], prev, bb, true);
	}
}

/*
 * Function to insert the necessary setup code at the beginning of the first basic block
 * Inserts
 * 	MOV r11, #1
 */
void YACCA::insertSetup(){
	basic_block bb = BASIC_BLOCK_FOR_FN(cfun, 2);
	rtx_insn* prev = UpdatePoint::firstRealINSN(bb);
	AsmGen::emitMovRegInt(regsToUse[0], 1, prev, bb, false);
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the beginning of the basic block
 */
void YACCA::insertSelBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	throw "Selective implementation for YACCA not officially supported and therefore not implemented!";
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions in the middle of the basic block
 */
void YACCA::insertSelMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	throw "Selective implementation for YACCA not officially supported and therefore not implemented!";
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the end of the basic block
 */
void YACCA::insertSelEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	throw "Selective implementation for YACCA not officially supported and therefore not implemented!";
}

/**
 * Function to calculate the PREVIOUS value of the provided basic block
 * and push it into the previousValues vector
 */
void YACCA::calcPrevious(unsigned int idBB, basic_block bb){
	edge e;
	edge_iterator ei;
	FOR_EACH_EDGE(e, ei,bb->preds){
		unsigned int idOrig = (e->src)->index - 2;
		if (idOrig <= n_basic_blocks_for_fn(cfun)-2 ){
			previousValues[idBB] *= signatures[idOrig];
		}
	}
}

/**
 * Function to calculate the M1 value of the provided basic block
 * and push it into the M1Values vector
 */
void YACCA::calcM1(unsigned int idBB, basic_block bb){
	edge e;
	edge_iterator ei;
	vector<unsigned int> predIds;
	FOR_EACH_EDGE(e, ei, bb->preds){
		unsigned int predId = (e->src)->index -2;
		predIds.push_back(predId);
	}
	if(predIds.size() == 1){
		M1Values.push_back(-1);
	}
	else{
		unsigned int m1 = signatures[predIds[0]];
		for(int x=1; x < predIds.size(); x++){
			m1 ^= signatures[predIds[x]];
			unsigned int mask = calcXORmask(m1);
			m1 ^= mask;
		}
		M1Values.push_back(m1);
	}
}

/**
 * Function to calculate the M2 value of the provided basic block
 * and push it into the M2Values vector
 */
void YACCA::calcM2(unsigned int idBB, basic_block bb){
	unsigned char predId = 0;

	edge e;
	edge_iterator ei;
	FOR_EACH_EDGE(e, ei, bb->preds){
		predId = (e->src)->index - 2;
		break;
	}
	unsigned int m2 = signatures[predId] & M1Values[idBB];
	M2Values.push_back(m2 ^ signatures[idBB]);
}

/**
 * Function to calculate the necessary mask to perform an XOR
 * operation with while calculating an M1 value
 */
unsigned int YACCA::calcXORmask(unsigned int m1){
	unsigned int count0 = __builtin_clz(m1); //
	unsigned int nec1 = 32 - count0;
	unsigned int mask = 1;
	for(int i = 1; i < nec1; i++){
		mask |= 1 << i;
	}
	return mask;
}

/**
 * Function to emit the Test instructions
 * Inserts:
 * 	UDIV r9, r10, r11
 * 	MUL r9, r11
 * 	CMP r9, r10
 * 	BNE .codeLabel
 */
rtx_insn* YACCA::generateTest(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachRTX){
	rtx_insn* prev = AsmGen::emitUdivRegRegReg(regsToUse[2], regsToUse[1], regsToUse[0], attachRTX, bb, true);
	prev = AsmGen::emitMulRegReg(regsToUse[2], regsToUse[2], regsToUse[0], prev, bb, true);
	prev = AsmGen::emitCmpRegReg(regsToUse[2], regsToUse[1], prev, bb, true);
	return AsmGen::emitBne(codeLabel, prev, bb, true);
}

/**
 * Function to determine whether or not the generateTest function
 * has to be called during the insertEnd function
 */
bool YACCA::insertTestEnd(rtx_insn* testInsn){
	if (JUMP_P(testInsn) ){ //|| InstrType::isReturn(testInsn)
		return false;
	}
	else{
		return true;
	}
}
