/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/*
 * Header file for the ARMv6M_Functions class, which
 * implements the pure virtual functions of the ARM_ISA class
 *
 * Contains the prototypes of the overridden functions.
 */

#ifndef TARGETS_ARMV6M_FUNCTIONS_H_
#define TARGETS_ARMV6M_FUNCTIONS_H_

#include "ArmISA_Functions.h"

class ARMv6M_Functions : public ARM_ISA{
	public:
		ARMv6M_Functions(processor_type cpu);
		~ARMv6M_Functions(){}

		void changeCBZ();

	private:
		void insertPush(vector<unsigned int> regs);
		void insertPop(vector<unsigned int> regs, rtx_insn* last, basic_block bb);
};


#endif /* TARGETS_ARMV6M_FUNCTIONS_H_ */
