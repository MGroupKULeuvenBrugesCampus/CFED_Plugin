/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/**
 * Header file containing some structs used in various techniques
 */

#ifndef CFED_TECHNIQUES_STRUCTSHOLDER_H_
#define CFED_TECHNIQUES_STRUCTSHOLDER_H_

/**
 * Struct to create paths used in CFCSS
 * Constains:
 * 	- Id of the start basic block
 * 	- Id of the end basic block
 * 	- The differential signature corresponding to this path
 */
struct CFCSSpath{
	unsigned int startBBId;
	unsigned int endBBId;
	unsigned int upD;
};

/**
 * Struct used to save the two NEXT values used in ECCA
 */
struct ECCAnext{
	unsigned int Next1;
	unsigned int Next2;
};

/**
 * Struct used to save the true and false Yvalue used in SIED
 */
struct SIEDbranch{
	unsigned int trueBranch;
	unsigned int falseBranch;
};

#endif /* CFED_TECHNIQUES_STRUCTSHOLDER_H_ */
