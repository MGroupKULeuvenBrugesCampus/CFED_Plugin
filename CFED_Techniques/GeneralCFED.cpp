/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>
#include <emit-rtl.h>

#include <stdlib.h>

#include "GeneralCFED.h"
#include "ArmISA_Functions.h"
#include "UpdatePoint.h"
#include "AsmGen.h"
#include "InstrType.h"

/**
 * Constructor, initializes the necessary variables.
 */
GeneralCFED::GeneralCFED(ARM_ISA* isa, unsigned int nrOfRegsToUse){
	this->isa = isa;
	for(int i = 0; i < nrOfRegsToUse; i++){
		this->regsToUse.push_back(this->isa->getNecessaryRegisters()[i]);
	}
	this->signatures.reserve(n_basic_blocks_for_fn(cfun)-2);
	this->nrOfOrigInstr.reserve(n_basic_blocks_for_fn(cfun)-2);
	this->insnID = get_max_uid();
	srand(time(NULL));
}

/**
 * Function which implements the selected CFE detection technique.
 * Determines in which order the pure virtual functions are executed.
 */
void GeneralCFED::implementTechnique(bool intraBlockDet, unsigned int selectiveLevel){
	// 1) Change the CBZ instructions
	isa->changeCBZ(); // Was after insterError();

	// 2) Count the number of original instructions -> needed by some techniques
    countNrOfOrigInstr();

    // 3) Calculate the necessary variables, such as signatures, etc.
	calcVariables();

	// 4) Insert the jump the CFED_Detected
	rtx_insn* codeLabel = insertError();

	// 5) Implement the technique, based on which selective level is provided
	if(selectiveLevel == 0){
		fullyImplementInAllBB(intraBlockDet, codeLabel);
	}
	else if(selectiveLevel == 1){
		selectiveImplementInAllBB(intraBlockDet, codeLabel);
	}
	else{
		throw "Wrong selectiveLevel provided. Values are 0 or 1";
	}

	// 6) Insert the setup code for the technique
	insertSetup();

	// 7) Insert the necessary Push and Pop of the signature register
	isa->insertPushPop(this->regsToUse);
}

/**
 * Implements the full form of the selected CFE detection technique.
 * For each basic block:
 * 	1) Insert the intra-block CFE detection instructions if necessary
 * 	2) Determine the middle rtx_insn and insert the CFE detection
 * 		instructions in the middle of the basic block
 *	3) Determine the first rtx_insn and insert the CFE detection
 *		instructions at the beginning of the basic block
 *	4) Insert the CFE detection instructions at the end of the basic block
 */
void GeneralCFED::fullyImplementInAllBB(bool intraBlockDet, rtx_insn* codeLabel){
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		unsigned int idBB = (bb->index) - 2;
		if(intraBlockDet){
			insertIntraBlockJumpDetection(idBB, bb, codeLabel);
		}
		rtx_insn* middleInsn = UpdatePoint::middleRealINSN(bb);
		insertMiddle(idBB, bb, codeLabel, middleInsn);
		rtx_insn* firstInsn = UpdatePoint::firstRealINSN(bb);
		insertBegin(idBB, bb, codeLabel, firstInsn);
		insertEnd(idBB, bb, codeLabel);
	}
}

/**
 * Implements the selective form of the selected CFE detection technique
 * For each basic block:
 * 	1) Insert the intra-block CFE detection instructions if necessary
 * 	2) Determine the middle rtx_insn and insert the CFE detection
 * 		instructions in the middle of the basic block
 *	3) Determine the first rtx_insn and insert the CFE detection
 *		instructions at the beginning of the basic block
 *	4) Insert the CFE detection instructions at the end of the basic block
 *	But uses the selective forms of these instructions.
 */
void GeneralCFED::selectiveImplementInAllBB(bool intraBlockDet, rtx_insn* codeLabel){
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		unsigned int idBB = (bb->index) - 2;
		if(intraBlockDet){
			insertIntraBlockJumpDetection(idBB, bb, codeLabel);
		}
		rtx_insn* middleInsn = UpdatePoint::middleRealINSN(bb);
		insertSelMiddle(idBB, bb, codeLabel, middleInsn);
		rtx_insn* firstInsn = UpdatePoint::firstRealINSN(bb);
		insertSelBegin(idBB, bb, codeLabel, firstInsn);
		insertSelEnd(idBB, bb, codeLabel);
	}
}

/**
 * Function that emits a code label at the end of the current function
 * and emits the call to the CFED_Detected error handler.
 */
rtx_insn* GeneralCFED::insertError(){
	rtx_insn* prev = get_last_insn();
	basic_block bb = BASIC_BLOCK_FOR_FN(cfun,last_basic_block_for_fn(cfun)-1);
	rtx_insn* codeLabel = AsmGen::emitCodeLabel(insnID++, prev, bb, true);
	AsmGen::emitCall(codeLabel);
	return codeLabel;
}

/**
 * Function that counts the number of original instructions
 * in the basic block
 */
void GeneralCFED::countNrOfOrigInstr(){
	// First count the number of instructions in the current basic block
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		unsigned int idBB = (bb->index) - 2;
		unsigned int nrOfInstr = 0;
		rtx_insn* insn;
		FOR_BB_INSNS(bb, insn){
			if(NONDEBUG_INSN_P(insn) && !InstrType::isUse(insn) && !InstrType::isUnspec(insn) &&
					!InstrType::isClobber(insn) && !InstrType::isUnspecVolatile(insn)){
				nrOfInstr++;
			}
		}
		this->nrOfOrigInstr[idBB] = nrOfInstr;
	}
}
