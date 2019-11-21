/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

#include "CFEDcreator.h"
#include "GeneralCFED.h"
#include "ArmISA_Functions.h"
#include "ARMv6M_Functions.h"
#include "ARMv7M_Functions.h"
#include "ARMv8M_Functions.h"

#include "RACFED.h"
#include "SEDSR.h"
#include "SCFC.h"
#include "CFCSS.h"
#include "ECCA.h"
#include "YACCA.h"
#include "YACCA_Fast.h"
#include "RSCFC.h"
#include "SIED.h"

/**
 * Function to implement the selected CFE detection technique.
 * 	1) determines the ISA of the current CPU
 * 	2) creates the selected CFE detection technique
 * 	3) calls the implementTechnique of the created CFE detection technique
 * 		to effectively implement the technique.
 */
void CFEDcreator::implementTechnique(const char *technique, bool intraBlockDet, unsigned int selectiveLevel){
	// 1) Create object for the ISA
	ARM_ISA* isa;
	switch(ARM_ISA::getISAtarget(arm_cpu_option)){
		case ARMv6M:
			isa = new ARMv6M_Functions(arm_cpu_option);
			break;
		case ARMv7M:
			isa = new ARMv7M_Functions(arm_cpu_option);
			break;
		case ARMv8M:
			isa = new ARMv8M_Functions(arm_cpu_option);
			break;
		default:
			throw "Not supported target and therefore unknown ISA!\n";
			break;
	}

	// 2) Create object for the CFE detection technique
	GeneralCFED* genCFED;
	if(!strcmp(technique, "RACFED")){
		genCFED = new RACFED(isa, 1);
	}
	else if(!strcmp(technique, "SEDSR")){
		genCFED = new SEDSR(isa, 1);
	}
	else if(!strcmp(technique, "SCFC")){
		genCFED = new SCFC(isa, 2);
	}
	else if(!strcmp(technique, "CFCSS")){
		genCFED = new CFCSS(isa, 2);
	}
	else if(!strcmp(technique, "ECCA")){
		genCFED = new ECCA(isa, 2);
	}
	else if(!strcmp(technique, "YACCA")){
		genCFED = new YACCA(isa, 3);
	}
	else if(!strcmp(technique, "YACCA_Fast")){
		genCFED = new YACCA_Fast(isa, 3);
	}
	else if(!strcmp(technique, "RSCFC")){
		genCFED = new RSCFC(isa, 2, intraBlockDet);
	}
	else if(!strcmp(technique, "SIED")){
		genCFED = new SIED(isa, 3, intraBlockDet);
	}
	else{
		throw "Unknown technique supplied to implement!\n";
	}

	// 3) Implement the selected technique
	genCFED->implementTechnique(intraBlockDet, selectiveLevel);
}
