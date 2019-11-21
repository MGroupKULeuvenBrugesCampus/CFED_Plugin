/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>
#include <tree.h>

#include <stdlib.h>

#include "RACFED.h"
#include "AsmGen.h"
#include "UpdatePoint.h"
#include "InstrType.h"

/**
 * Constructor, initializes necessary variables
 * Some values dependent on the ISA of the current CPU
 */
RACFED::RACFED(ARM_ISA* isa, unsigned int nrOfRegsToUse)
	:GeneralCFED(isa, nrOfRegsToUse){
	vector<int>tempInt(n_basic_blocks_for_fn(cfun)-2, 0);
	this->intraBlockAddValues = tempInt;								// Making sure the vector 0 filled.
	this->subRanPrevValues.reserve(n_basic_blocks_for_fn(cfun)-2);		// Making sure the vector exists for all needed spaces
	switch(ARM_ISA::getISAtarget(arm_cpu_option)){
		case ARMv7M:
			this->CMPlimit = 254;
			this->subRanPrevValLimit = 1500;
			this->sigRegLowerLimit = -2341;
			this->sigRegUpperLimit = 4095;
			break;
		case ARMv6M:
		default:
			this->CMPlimit = 254;
			this->subRanPrevValLimit = 255;
			this->sigRegLowerLimit = 0;
			this->sigRegUpperLimit = 255;
	}
}

/**
 * Function to calculate / assign the
 * 	- compile-time signatures for each basic block
 * 	- subRanPrevVal values for each basic block
 */
void RACFED::calcVariables(){
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		unsigned int idBB = (bb->index) - 2;
		unsigned int tempSig = 0;
		unsigned int tempSubRanPrev = 0;
		do{
			// Assign signature
			do{
				tempSig = (rand() % CMPlimit) + 1;
			}while(!isUniqueSignature(tempSig));
			signatures[idBB] = tempSig;
			// Assign subRanPrevValue
			tempSubRanPrev = rand() % subRanPrevValLimit;
		}while(!isUniqueSum(idBB, tempSubRanPrev));
		subRanPrevValues[idBB] = tempSubRanPrev;
	}
}

/**
 * Function to determine whether or not the current
 * candidate signature is unique in the function
 */
bool RACFED::isUniqueSignature(unsigned int signature){
	bool unique = true;
	for(int i = 0; i < n_basic_blocks_for_fn(cfun)-2; i++){
		if(signature == signatures[i]){
			unique = false;
		}
	}
	return unique;
}

/**
 * Function to determine whether or not the sum of
 * the current candidate subRanPrevVal and compile-time signature
 * is unique in the function
 */
bool RACFED::isUniqueSum(unsigned int idBB, unsigned int subRanPrevValue){
	bool unique = true;
	unsigned int ownSum = signatures[idBB] + subRanPrevValue;
	for(int i = 0; i < idBB; i++){
		unsigned int existingSum = signatures[i] + subRanPrevValues[i];
		if (ownSum == existingSum){
			unique = false;
		}
	}
	if(!sigRegValueInRange(ownSum)){
		unique = false;
	}
	return unique;
}

/**
 * Function to insert the necessary intra-block CFE detection instruction
 * Inserts
 * 	ADD r11, #randomValue -> after each original instruction in the basic block
 */
void RACFED::insertIntraBlockJumpDetection(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	// If the number of instructions is larger than 2, insert protective instructions
	if(nrOfOrigInstr[idBB] > 2){
		rtx_insn* rtl;
		FOR_BB_INSNS(bb, rtl){
			// Insert an intra block check after each instruction in the BB, unless
			// * the instruction is a debug instruction that is not translated into assembly
			// * the instruction is a jump instruction
			// * the instructions successor is a jump instruction
			// * the instruction is the last one of the basic block
			// * the instruction is a USE instruction
            // * the instruction is a CALL instruction
			if(NONDEBUG_INSN_P(rtl) && !JUMP_P(rtl) && !JUMP_P(NEXT_INSN(rtl)) && (BB_END(bb) != rtl) &&
					!InstrType::isUse(rtl) && !CALL_P(rtl) && !InstrType::isUnspec(rtl) &&
					!InstrType::isClobber(rtl) && !InstrType::isUnspecVolatile(rtl)){
				int intraBlockValue = assignIntraBlockValue(idBB);
				rtl = AsmGen::emitAddRegInt(regsToUse[0], intraBlockValue, rtl, bb, true);
				this->intraBlockAddValues[idBB] += intraBlockValue;
			}
		}
    }
}

/**
 * Function to determine the value to use in the current
 * intra-block CFE detection instructions
 * Uses some checks to determine the correct value
 */
int RACFED::assignIntraBlockValue(unsigned int idBB){
	int intraBlockValue = 0;
	int currVal = signatures[idBB] + intraBlockAddValues[idBB];
	int currValCandidate = 0;
	int candidate = 0;
	do{
		candidate = (rand() % (sigRegUpperLimit + abs(sigRegLowerLimit))) -(sigRegUpperLimit + abs(sigRegLowerLimit))/2; // Random value between [-2000 and 4000[
		currValCandidate = currVal + candidate;
	}while(!sigRegValueInRange(currValCandidate) || (candidate == 0) );
	// Validate that the overall signature value remains within [-2341, 4095] because of limits of ADDW and SUBW instructions
	return candidate;
}

/**
 * Function to check whether or not the current candidate for the
 * intra-block CFE detection instruction is in the necessary range
 */
bool RACFED::sigRegValueInRange(int sigRegValue){
	return((sigRegValue > sigRegLowerLimit) && (sigRegValue < sigRegUpperLimit));
}

/**
 * Function to get the first non-debug previous rtx_insn
 * of the provided rtx_insn
 */
rtx_insn* RACFED::getPrevInsn(rtx_insn* insn){
	rtx_insn* prev = PREV_INSN(insn);
	while(!NONDEBUG_INSN_P(prev)){
		prev = PREV_INSN(prev);
	}
	return prev;
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the beginning of each basic block
 * Inserts
 * 	SUB r11, #<subRanPrevVal>
 * 	CMP r11, #<compileTimeSignature>
 * 	BNE .codeLabel
 */
void RACFED::insertBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	if (nrOfOrigInstr[idBB] == 0){
		attachBefore = NEXT_INSN(BB_HEAD(bb));
	}
	rtx_insn* prev = AsmGen::emitAddRegInt(regsToUse[0], (0-subRanPrevValues[idBB]), attachBefore, bb, false);
	// filter special case for which the system flags cannot be changed.
	if( (nrOfOrigInstr[idBB] != 1) || !(InstrType::isCondJump(attachBefore)) ){   // was &&
		switch(ARM_ISA::getISAtarget(arm_cpu_option)){
			case ARMv7M:
				prev = AsmGen::emitCmpRegInt(regsToUse[0], signatures[idBB], prev, bb, true);
				AsmGen::emitBne(codeLabel, prev, bb, true);
				break;
			case ARMv6M:
			default:
				AsmGen::emitBne(regsToUse[0], signatures[idBB], codeLabel, prev, bb, true);
				break;
			}
	}
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the beginning of the basic block
 * Inserts
 * 	SUB r11, #<subRanPrevVal>
 * 	CMP r11, #<compileTimeSignature> -> From here only in exit basic block
 * 	BNE .codeLabel
 */
void RACFED::insertSelBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore){
	rtx_insn* prev = AsmGen::emitAddRegInt(regsToUse[0], (0-subRanPrevValues[idBB]), attachBefore, bb, false);
	// Insert CMP and BNE in exit basic blocks
	if(InstrType::isExitBlock(bb)){
		switch(ARM_ISA::getISAtarget(arm_cpu_option)){
			case ARMv7M:
				prev = AsmGen::emitCmpRegInt(regsToUse[0], signatures[idBB], prev, bb, true);
				AsmGen::emitBne(codeLabel, prev, bb, true);
				break;
			case ARMv6M:
			default:
				AsmGen::emitBne(regsToUse[0], signatures[idBB], codeLabel, prev, bb, true);
				break;
		}
	}
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * in the middle of each basic block
 */
void RACFED::insertMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter){
	// Nothing to do for RACFED
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions in the middle of the basic block
 */
void RACFED::insertSelMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel ,rtx_insn* attachAfter){
	RACFED::insertMiddle(idBB, bb, codeLabel, attachAfter);
}

/**
 * Function to insert the necessary inter-block CFE detection instructions
 * at the end of each basic block
 * Inserts
 * 	ADD r11, #<value> -> can be SUB, can be conditional depending on the situation
 */
void RACFED::insertEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
    if(bb->succs->length() != 0){
	    int trueId = -1;
	    int falseId = -1;

	    edge e;
	    edge_iterator ei;
	    FOR_EACH_EDGE(e, ei, bb->succs){
		    unsigned int idSuccs = (e->dest)->index -2;
		    if (idSuccs == (idBB+1)){
			    falseId = idSuccs;
		    }
		    else{
			    trueId = idSuccs;
		    }
	    }

	    rtx_insn* lastInsn = UpdatePoint::lastRealINSN(bb);
        unsigned int returnVal = 0;
	    if (nrOfOrigInstr[idBB] == 0 && !InstrType::isExitBlock(bb)){
		    lastInsn == BB_END(bb);
		    insertAdjust(idBB, falseId, lastInsn, bb, true);
	    }
	    else if(InstrType::isCondJump(lastInsn)){
	    	switch(ARM_ISA::getISAtarget(arm_cpu_option)){
	    		case ARMv7M:
	    			{
	    			rtx_code trueCode = InstrType::getCondCode(lastInsn);
					rtx_code falseCode = InstrType::findContraryConditionalCode(trueCode);
					insertCondAdjust(idBB, trueId, trueCode, lastInsn, bb);
					insertCondAdjust(idBB, falseId, falseCode, lastInsn, bb);
					break;
	    			}
	    		case ARMv6M:
	    		default:
	    			{
	    			int adjustValTrue = insertTrueAdjust(idBB, trueId, lastInsn, bb);
	    			insertFalseAdjust(idBB, falseId, adjustValTrue, lastInsn, bb);
	    			break;
	    			}
	    	}
	    }
	    else if (InstrType::isExitBlock(bb)){
			// Only insert the next instructions if there are more than 2 instructions in the BB
			// or if the second instructions is not a 'use' rtx
			//if((nrOfOrigInstr[idBB] > 2)||((nrOfOrigInstr[idBB] == 2) && (!UpdatePoint::isUse(getPrevInsn(lastInsn))))){
	    	if( nrOfOrigInstr[idBB] > 1 ){
				returnVal = rand() % 254;
				rtx_insn* prev = insertAdjustEnd(idBB, returnVal, lastInsn, bb);
				switch(ARM_ISA::getISAtarget(arm_cpu_option)){
					case ARMv7M:
						prev = AsmGen::emitCmpRegInt(regsToUse[0], returnVal, prev, bb, true);
						AsmGen::emitBne(codeLabel, prev, bb, true);
						break;
					case ARMv6M:
					default:
						AsmGen::emitBne(regsToUse[0], returnVal, codeLabel, prev, bb, true);
						break;
				}

			}
		}
        else if(JUMP_P(lastInsn)){
			insertAdjust(idBB, trueId, lastInsn, bb, false);
		}
		else if(CALL_P(lastInsn)){
			insertAdjust(idBB, falseId, lastInsn, bb, false);
		}
		else{
			insertAdjust(idBB, falseId, lastInsn, bb, true);
		}
    }
}

/**
 * Function to insert the necessary selective form of the inter-block
 * CFE detection instructions at the end of the basic block
 * Is equal to the full implementation, so just calls that method
 */
void RACFED::insertSelEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel){
	RACFED::insertEnd(idBB, bb, codeLabel);
}

/**
 * Emits ADD r11, #<AdjustValue>
 */
rtx_insn* RACFED::insertAdjust(unsigned int idBB, unsigned int idSuccs, rtx_insn* lastInsn, basic_block bb, bool after){
	int currSigVal = signatures[idBB] + intraBlockAddValues[idBB];
	unsigned int nextVal = signatures[idSuccs] + subRanPrevValues[idSuccs];
	int adjustVal = nextVal - currSigVal;
	return AsmGen::emitAddRegInt(regsToUse[0], adjustVal, lastInsn, bb, after);
}

/**
 * Emits ADD r11, #<AdjustValue>
 * For use in exit basic blocks only
 */
rtx_insn* RACFED::insertAdjustEnd(unsigned int idBB, unsigned int returnVal, rtx_insn* lastInsn, basic_block bb){
	int currSigVal = signatures[idBB] + intraBlockAddValues[idBB];
	int adjustVal = returnVal - currSigVal;
	return AsmGen::emitAddRegInt(regsToUse[0], adjustVal, lastInsn, bb, false);
}

/**
 * Emits ADD<cond> r11, #<AdjustValue>
 */
rtx_insn* RACFED::insertCondAdjust(unsigned int idBB, unsigned int idSuccs, enum rtx_code condition, rtx_insn* lastInsn, basic_block bb){
	int currSigVal = signatures[idBB] + intraBlockAddValues[idBB];
	unsigned int nextVal = signatures[idSuccs] + subRanPrevValues[idSuccs];
	int adjustVal = nextVal - currSigVal;
	return AsmGen::emitCondAddRegInt(regsToUse[0], adjustVal, condition, lastInsn, bb, false);
}

/**
 * Emits ADD r11, #<AdjustValue>
 * For use with ARMv6-M only
 */
int RACFED::insertTrueAdjust(unsigned int idBB, unsigned int idSuccs, rtx_insn* condJumpInsn, basic_block bb){
	int currSigVal = signatures[idBB] + intraBlockAddValues[idBB];
	unsigned int nextVal = signatures[idSuccs] + subRanPrevValues[idSuccs];
	int adjustVal = nextVal - currSigVal;
	AsmGen::emitAddRegInt(regsToUse[0], adjustVal, condJumpInsn, bb, false);
	return adjustVal;
}

/**
 * Emits ADD r11, #<AdjustValue>
 * For use with ARMv6-M only
 */
void RACFED::insertFalseAdjust(unsigned int idBB, unsigned int idSuccs, int adjustValTrue, rtx_insn* condJumpInsn, basic_block bb){
	int currSigVal = signatures[idBB] + intraBlockAddValues[idBB];
	unsigned int nextVal = signatures[idSuccs] + subRanPrevValues[idSuccs];
	int adjustVal = nextVal - currSigVal - adjustValTrue;
	AsmGen::emitAddRegInt(regsToUse[0], adjustVal, condJumpInsn, bb, true);
}

/**
 * Function to insert the necessary setup code at the beginning of the first basic block
 * Inserts
 * 	MOV r11, #( <compileTimeSignatureFirstBasicBlock> + <subRanPrevValFirstBasicBlock> )
 */
void RACFED::insertSetup(){
	basic_block bb = BASIC_BLOCK_FOR_FN(cfun, 2);
	rtx_insn* prev = UpdatePoint::firstRealINSN(bb);
	unsigned int val = signatures[0] + subRanPrevValues[0];
	prev = AsmGen::emitMovRegInt(regsToUse[0], val, prev, bb, false);
}
