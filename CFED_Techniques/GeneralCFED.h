/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/*
 * Header file for the GeneralCFED class
 *
 * Contains the prototype of teh implementTechnique which effectively
 * implements the selected CFE detection technique.
 *
 * Contains pure virtual functions of the methods that must be implemented
 * by each supported CFE detection technique (= interface)
 */

#ifndef CFED_TECHNIQUES_GENERALCFED_H_
#define CFED_TECHNIQUES_GENERALCFED_H_

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include <vector>

#include "ArmISA_Functions.h"

using namespace std;

class GeneralCFED{
	public:
		GeneralCFED(ARM_ISA* isa, unsigned int nrOfRegsToUse);
		virtual ~GeneralCFED(){}

		void implementTechnique(bool intraBlockDet, unsigned int selectiveLevel);

	protected:
		vector<unsigned int> signatures;
		vector<unsigned int> regsToUse;
		vector<unsigned int> nrOfOrigInstr;
		ARM_ISA* isa;

	private:
		/**
		 * Function to calculate all necessary variables
		 * E.g. signatures
		 */
		virtual void calcVariables() = 0;

		/**
		 * Function to insert extra rtx's to detect intra block jumps
		 */
		virtual void insertIntraBlockJumpDetection(unsigned int idBB, basic_block bb, rtx_insn* codeLabel) = 0;

		/**
		 * Function to insert extra rtx's as first rtx's of the basic block
		 */
		virtual void insertBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore) = 0;

		/**
		 * Selective form of insertBegin
		 */
		virtual void insertSelBegin(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachBefore) = 0;

		/**
		 * Function to insert extra rtx's in the middle of the basic block
		 */
		virtual void insertMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter) = 0;

		/**
		 * Selective form of insertMiddle
		 */
		virtual void insertSelMiddle(unsigned int idBB, basic_block bb, rtx_insn* codeLabel, rtx_insn* attachAfter) = 0;

		/**
		 * Function to insert extra rtx's at the end of the basic block
		 */
		virtual void insertEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel) = 0;

		/**
		 * Selective form of insertEnd
		 */
		virtual void insertSelEnd(unsigned int idBB, basic_block bb, rtx_insn* codeLabel) = 0;

		/**
		 * Function to insert the setup functionality needed for the specific technique
		 */
		virtual void insertSetup() = 0;

		/**
		 * Function to insert the infinite while loop as CFE detection indicator
		 * returns the created codeLabel
		 */
		rtx_insn* insertError();

		void countNrOfOrigInstr();

		unsigned int insnID;

		// Functions to clearly separate the functionality of the different selective levels
		void fullyImplementInAllBB(bool intraBlockDet, rtx_insn* codeLabel);
		void selectiveImplementInAllBB(bool intraBlockDet, rtx_insn* codeLabel);
};


#endif /* CFED_TECHNIQUES_GENERALCFED_H_ */
