/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>
#include <tree.h>

#include <stdio.h>

#include "Printer.h"


/**
 * Function that writes the RTL statements to
 * the file with the given fileName
 * @param fileName The name of the file to write to
 */
void Printer::printRTL(char* fileName){
	char tempName[512];
	createFileName(tempName, fileName);
	FILE *fp = fopen(tempName, "w");
	if (fp != NULL){
		basic_block bb;
		FOR_ALL_BB_FN(bb, cfun){					// Loop over all Basic Blocks in the function, cfun = current function
			fprintf(fp,"BB: %d\n", bb->index-2);
			rtx_insn* insn;
			FOR_BB_INSNS(bb, insn){
				// Loop over all rtx statements in the Basick Block
				if( NONDEBUG_INSN_P(insn) ){		// Filter all actual code statements
					//print_simple_rtl(fp, insn);
					print_rtl_single(fp, insn);		// print to file
				}
			}
			fprintf(fp,"\n----------------------------------------------------------------\n\n");
		}
		fclose(fp);
	}
}

/**
 * Function to print the correct Edges file that is used
 * in various tools to validate the CFG build in those tools
 */
void Printer::printEdges(char* fileName){
	char tempName[512];
	createFileName(tempName, fileName);
	FILE *fp = fopen(tempName, "w");
	if (fp != NULL){
		basic_block bb;
		FOR_EACH_BB_FN(bb, cfun){
			unsigned int idBB = bb->index-2;
			fprintf(fp, "BB: %d\n", idBB);
			edge e;
			edge_iterator ei;
			char counter = 0;
			FOR_EACH_EDGE(e, ei, bb->succs){
				counter++;
			}
			unsigned int idDests[counter];
			char index = 0;
			FOR_EACH_EDGE(e, ei, bb->succs){
				unsigned int idDest = (e->dest)->index -2;
				idDests[index] = idDest;
				index ++;
			}
			if(counter == 2){
				if(idDests[1] < idDests[0]){
					unsigned int temp  = idDests[0];
					idDests[0] = idDests[1];
					idDests[1] = temp;
				}
			}
			for(char x = 0; x < counter; x++){
				fprintf(fp, "\t%i --> %i\n", idBB, idDests[x]);
			}
			fprintf(fp,"\n");
		}
		fclose(fp);
	}
}

/**
 * Function to print the structure of the current function
 * to a file with the provided name.
 *
 * Structure = What edges exist in the CFG and what basic blocks
 * exist in the CFG.
 */
void Printer::printAnalysis(char* fileName){
	char tempName[256];
	createFileName(tempName, fileName);
	edgeAnalysis(tempName);
	blockAnalysis(tempName);
}

// -------------------------------------- Private Section ------------------------------
/**
 * Function to create the necessary file name
 */
void Printer::createFileName(char *name, char* fileName){
	char* dirName = (char*)IDENTIFIER_POINTER (DECL_NAME (current_function_decl) );
	strcpy(name, "GCC_Plugin_Output/");
	strcat(name, dirName);
	strcat(name,"/");
	strcat(name,fileName);
}

/**
 * Function to print the existing edges to file.
 */
void Printer::edgeAnalysis(char* fileName){
	// Variables to analyze the edges current function
	unsigned int totalEdges = 0;
	unsigned int uncondEdges = 0;
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		edge e;
		edge_iterator ei;
		char counter = 0;
		FOR_EACH_EDGE(e, ei, bb->succs){
			totalEdges++;
			counter++;
		}
		if (counter == 1){
			uncondEdges ++;
		}
	}
	unsigned int condEdges = totalEdges - uncondEdges;

	// Print result of edge analysis
	FILE* fp = fopen(fileName, "w");
	if( fp != NULL){
		fprintf(fp, "Edge Analysis:\n");
		fprintf(fp, "\tTotal amount of edges: %i\n", totalEdges);
		fprintf(fp, "\tNumber of unconditional edges: %i\n", uncondEdges);
		fprintf(fp, "\tNumber of conditional edges: %i\n", condEdges);
		fprintf(fp, "------------------------------------------------------\n");
		fclose(fp);
	}
}

/**
 * Function to print the number of basic blocks
 * and the length of each basic block to the file
 */
void Printer::blockAnalysis(char* fileName){
	unsigned int totalBlocks = n_basic_blocks_for_fn(cfun)-2;

	FILE* fp = fopen(fileName, "a");
	if (fp != NULL){
		fprintf(fp, "Block Analysis:\n");
		fprintf(fp, "\tTotal amount of basic blocks: %i\n", totalBlocks);

		basic_block bb;
		FOR_EACH_BB_FN(bb, cfun){
			unsigned int index = bb->index -2;
			unsigned int length = 0;
			rtx_insn* insn;
			FOR_BB_INSNS(bb, insn){
				if(NONDEBUG_INSN_P(insn)){
					length++;
				}
			}
			fprintf(fp, "\tLength of basic block %i: %i\n", index, length);
		}
		fclose(fp);
	}
}
