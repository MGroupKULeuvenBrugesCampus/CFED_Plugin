/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/*
 * Header file for the ARM_ISA class.
 *
 * Contains the ISAs enum
 *
 * Contains the prototypes of the methods to determine
 * whether or not the current CPU is supported and
 * what the ISA is of the current CPU.
 *
 * Contains pure virtual functions related to the
 * necessary PUCH POP operation.
 */

#ifndef TARGETS_ARMISA_FUNCTIONS_H_
#define TARGETS_ARMISA_FUNCTIONS_H_


#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include <vector>
#include <string>

using namespace std;

enum ISAs{
	ARMv6M, ARMv7M, ARMv8M, UNKNOWN_ISA
};

class ARM_ISA{
	public:
		static bool supportedTarget(processor_type cpu);
		static ISAs getISAtarget(processor_type cpu);

		ARM_ISA(processor_type cpu);
		virtual ~ARM_ISA(){}

		vector<unsigned int> getNecessaryRegisters();

		void insertPushPop(vector<unsigned int> regs);

		virtual void changeCBZ() = 0;

	protected:
		vector<string> pushPopStrings;
		const unsigned char stackPointer;

	private:
		processor_type cpu;

		virtual void insertPush(vector<unsigned int> regs) = 0;
		virtual void insertPop(vector<unsigned int> regs, rtx_insn* last, basic_block bb) = 0;
};


#endif /* TARGETS_ARMISA_FUNCTIONS_H_ */
