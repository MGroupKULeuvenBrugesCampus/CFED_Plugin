/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/*
 * Header file for the RACFED class, which represents our in-house developed RASM and RACFED
 * CFE detection techniques.
 * RASM = inter-block CFE detection only; RACFED = both inter-block and intra-block detection
 * The RACFED class implements the virtual methods of the GeneralCFED class
 *
 * Contains the prototypes of the overridden functions and some private functions
 * necessary to implement the RASM / RACFED techniques
 */

#ifndef CFED_TECHNIQUES_RACFED_H_
#define CFED_TECHNIQUES_RACFED_H_

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "GeneralCFED.h"


class RACFED : public GeneralCFED{
	public:
		RACFED(ARM_ISA* isa, unsigned int nrOfRegsToUse);
		~RACFED(){}

	private:
		void calcVariables();

		void insertIntraBlockJumpDetection(unsigned int idBB, basic_block bb, rtx_insn* codeLabel);

		void insertBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore);
		void insertMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter);
		void insertEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel);
		void insertSetup();

		// Selective methods
		void insertSelBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore);
		void insertSelMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter);
		void insertSelEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel);

		vector<unsigned int> subRanPrevValues;
		vector<int> intraBlockAddValues;

		bool isUniqueSignature(unsigned int signature);
		bool isUniqueSum(unsigned int idBB, unsigned int subRanPrevValue);
		int assignIntraBlockValue(unsigned int idBB);

		rtx_insn* getPrevInsn(rtx_insn* insn);

		// Cortex-M3
		rtx_insn* insertCondAdjust(unsigned int idBB, unsigned int idSuccs, rtx_code condition, rtx_insn* lastInsn, basic_block bb);
		// Cortex-M0
		int insertTrueAdjust(unsigned int idBB, unsigned int idSuccs, rtx_insn* condJumpInsn, basic_block bb);
		void insertFalseAdjust(unsigned int idBB, unsigned int idSuccs, int adjustValTrue, rtx_insn* condJumpInsn, basic_block bb);
		// All
		rtx_insn* insertAdjust(unsigned int idBB, unsigned int idSuccs, rtx_insn* lastInsn, basic_block bb, bool after);
		rtx_insn* insertAdjustEnd(unsigned int idBB, unsigned int returnVal, rtx_insn* lastInsn, basic_block bb);

		// Some limits for the random process
		unsigned int CMPlimit;
		unsigned int subRanPrevValLimit;
		int sigRegLowerLimit;
		int sigRegUpperLimit;
		bool sigRegValueInRange(int sigRegValue);
};


#endif /* CFED_TECHNIQUES_RACFED_H_ */
