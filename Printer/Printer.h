/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/*
 * Header file of the Pinter class.
 *
 * It contains the prototypes of methods to print
 * certain aspects of the current function to .txt files
 */

#ifndef PRINTER_PRINTER_H_
#define PRINTER_PRINTER_H_


#include <gcc-plugin.h>

class Printer{
	public:
		static void printRTL(char* fileName);
		static void printEdges(char* fileName);
		static void printAnalysis(char* fileName);
	private:
		static void createFileName(char *name, char* fileName);
		static void edgeAnalysis(char* fileName);
		static void blockAnalysis(char* fileName);
};


#endif /* PRINTER_PRINTER_H_ */
