/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/**
 * Header file for the CFEDcreator class.
 *
 * Has only one method 'implementTechnique' which
 * 	1) determines the ISA of the current CPU
 * 	2) creates the selected CFE detection technique
 * 	3) calls the implementTechnique of the created CFE detection technique
 * 		to effectively implement the technique.
 */

#ifndef CFED_TECHNIQUES_CFEDCREATOR_H_
#define CFED_TECHNIQUES_CFEDCREATOR_H_

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

class CFEDcreator{
	public:
		// Function to create the correct technique and implement it
		void implementTechnique(const char* technique, bool intraBlockDet, unsigned int selectiveLevel);
};


#endif /* CFED_TECHNIQUES_CFEDCREATOR_H_ */
