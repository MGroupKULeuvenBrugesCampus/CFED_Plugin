/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>
#include <tree.h>

#include "UpdatePoint.h"
#include "InstrType.h"

/**
 * Function that returns the first real INSN of
 * the basic block bb.
 * (No Debug or Note insn)
 */
rtx_insn* UpdatePoint::firstRealINSN(basic_block bb){
	rtx_insn* next = BB_HEAD(bb);
	while(!NONDEBUG_INSN_P(next)){
		next = NEXT_INSN(next);
	}
	return next;
}

/**
 * Function that returns the middle real INSN
 * of the basic block bb.
 * (No Debug or Note insn)
 * To insert instructions in the middle of the basic block
 * attach them AFTER this INSN
 */
rtx_insn* UpdatePoint::middleRealINSN(basic_block bb){
	// Learn how many INSNs this block contains
	unsigned int totalINSN = countInsnBB(bb);
	// Find the middle INSN
	if(totalINSN != 1){			// Most cases
		unsigned int middleINSNindex = totalINSN/2;
		rtx_insn* middleINSN = findInsn(bb, middleINSNindex);
		// Make it safe to use by filtering out jump_insns and cond_exec insns
		while( JUMP_P(middleINSN)|| InstrType::isCondExec(middleINSN) ){
			do{
				middleINSN = PREV_INSN(middleINSN);
			}
			while(!NONDEBUG_INSN_P(middleINSN));
		}
		if(InstrType::isCompare(middleINSN)){
			middleINSN = PREV_INSN(middleINSN);
		}
		return middleINSN;
	}
	else{
		rtx_insn* middleINSN = findInsn(bb, 1);
		// filter out Return statement
		if (InstrType::isReturn(middleINSN)){
			middleINSN = PREV_INSN(middleINSN);
		}
		return middleINSN;
	}
}

/**
 * Function that returns the last real INSN
 * of the basic block
 * (No Debug or Note insn)
 */
rtx_insn* UpdatePoint::lastRealINSN(basic_block bb){
	rtx_insn* lastInsn = BB_END(bb);
	while(!NONDEBUG_INSN_P(lastInsn)){
		lastInsn = PREV_INSN(lastInsn);
	}
	return lastInsn;
}

rtx_insn* UpdatePoint::lastRealSafeINSN(basic_block bb){
	rtx_insn* lastInsn = lastRealINSN(bb);
	if(InstrType::isReturn(lastInsn)){
		lastInsn = PREV_INSN(lastInsn);
	}
	else if(JUMP_P(lastInsn) && !InstrType::isCondJump(lastInsn)){
			lastInsn = PREV_INSN(lastInsn);
		}
	else if (JUMP_P(lastInsn) && InstrType::isCondJump(lastInsn)){
			do{
				lastInsn = PREV_INSN(lastInsn);
			}while(!InstrType::isCompare(lastInsn));
			lastInsn = PREV_INSN(lastInsn);
		}
	return lastInsn;
}

/**
 * Function to count the number of real instructions in the basic block
 */
unsigned int UpdatePoint::countInsnBB(basic_block bb){
	rtx_insn* insn;
	unsigned int totalINSN = 0;
	FOR_BB_INSNS(bb,insn){
		if(NONDEBUG_INSN_P(insn)){
			totalINSN++;
		}
	}
	return totalINSN;
}

/**
 * Function to find the rtx_insn at the provided index
 * in the given basic block
 */
rtx_insn* UpdatePoint::findInsn(basic_block bb, unsigned int index){
	rtx_insn* insn = BB_HEAD(bb);
	int count = 0;
	while(count < index){
		insn = NEXT_INSN(insn);
		if(NONDEBUG_INSN_P(insn)){
			count++;
		}
	}
	return insn;
}

