/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "ArmISA_Functions.h"
#include "InstrType.h"
#include "UpdatePoint.h"


// ----------------- STATIC Section ------------------------
/**
 * Function to determine whether or not the current CPU is
 * supported. Currently only CPUs of ARMv6-M and ARMv7-M are
 * supported.
 */
bool ARM_ISA::supportedTarget(processor_type cpu){
	bool supported = false;
	switch(cpu){
		// Support all ARMv6M targets
		case TARGET_CPU_cortexm0:
		case TARGET_CPU_cortexm0plus:
		case TARGET_CPU_cortexm0plussmallmultiply:
		case TARGET_CPU_cortexm0smallmultiply:
		case TARGET_CPU_cortexm1:
		case TARGET_CPU_cortexm1smallmultiply:
		// Support all ARMv7M targets
		case TARGET_CPU_cortexm3:
		case TARGET_CPU_cortexm4:
		case TARGET_CPU_cortexm7:
			supported = true;
			break;
		// ARMv8-M is unsupported at the moment
		case TARGET_CPU_cortexm23:
		case TARGET_CPU_cortexm33:
		case TARGET_CPU_cortexm33nodsp:
		// Any other processor is not supported.
		default:
			printf("--- Unsupported target! ---\n");
			break;
	}

	return supported;
}

/**
 * Function to retrieve the ISA of the provided CPU
 */
ISAs ARM_ISA::getISAtarget(processor_type cpu){
	ISAs isa;
	switch(cpu){
		case TARGET_CPU_cortexm0:
		case TARGET_CPU_cortexm0plus:
		case TARGET_CPU_cortexm0plussmallmultiply:
		case TARGET_CPU_cortexm0smallmultiply:
		case TARGET_CPU_cortexm1:
		case TARGET_CPU_cortexm1smallmultiply:
			isa = ARMv6M;
			break;
		case TARGET_CPU_cortexm3:
		case TARGET_CPU_cortexm4:
		case TARGET_CPU_cortexm7:
			isa = ARMv7M;
			break;
		case TARGET_CPU_cortexm23:
		case TARGET_CPU_cortexm33:
		case TARGET_CPU_cortexm33nodsp:
			isa = ARMv8M;
			break;
		default:
			isa = UNKNOWN_ISA;
			break;
	}
	return isa;
}

// ----------------- PUBLIC OBJECT Section ------------------------
/**
 * Constructor, initializes
 * 	- stackPointer constant: hard coded as register r6 for the moment
 * 	- cpu: provided as argument
 * 	- pushPopStrings vector: create an empty vector of 6 places
 */
ARM_ISA::ARM_ISA(processor_type cpu)
	: stackPointer(6){
	this->cpu = cpu;
	this->pushPopStrings.reserve(6);
}

/**
 * Function to retrieve the registers that can be used
 * to implement the selected CFE detection technique.
 * For
 * 	- ARMv6-M these are r7, r5 and r4;
 * 	- ARMv7-M and ARMv8-M these are r11, r10 and r9.
 * These are hard coded for the moment.
 */
vector<unsigned int> ARM_ISA::getNecessaryRegisters(){
	vector<unsigned int> regs;
	switch(ARM_ISA::getISAtarget(cpu)){
		case ARMv6M:
			regs = {7, 5, 4};
			break;
		case ARMv7M:
			regs = {11, 10, 9};
			break;
		case ARMv8M:
			regs = {11, 10, 9};
			break;
		default:
			regs = {0, 0, 0};
			break;
	}
	return regs;
}

/**
 * Function to implement the necessary PUSH and POP instructions
 */
void ARM_ISA::insertPushPop(vector<unsigned int> regs){
	// 1) Insert Push at the start of the function
	insertPush(regs);

	// 2) Insert Pop in each exit basic block
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		if (InstrType::isExitBlock(bb)){
			rtx_insn* last = UpdatePoint::lastRealINSN(bb);
			insertPop(regs, last, bb);
		}
	}
}

