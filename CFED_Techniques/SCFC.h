/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/*
 * Header file for the SCFC class, which represents the SCFC CFE detection technique
 * The SCFC class implements the virtual methods of the GeneralCFED class
 *
 * Contains the prototypes of the overridden functions and some private functions
 * necessary to implement the SCFC technique
 */

#ifndef CFED_TECHNIQUES_SCFC_H_
#define CFED_TECHNIQUES_SCFC_H_

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "GeneralCFED.h"

class SCFC : public GeneralCFED{
	public:
		SCFC(ARM_ISA* isa, unsigned int nrOfRegsToUse);
		~SCFC(){}

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

		rtx_insn* emitLSR(unsigned int idBB, rtx_insn* attachRtx, basic_block bb);
};

#endif /* CFED_TECHNIQUES_SCFC_H_ */
