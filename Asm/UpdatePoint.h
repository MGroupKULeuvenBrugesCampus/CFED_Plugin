/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/*
 * This is the header file of the UpdatePoint class.
 *
 * It contains the prototypes of the methods used to
 * determine the first real instruction, the middle real instruction
 * and the last real instruction of a basic block.
 */

#ifndef ASM_UPDATEPOINT_H_
#define ASM_UPDATEPOINT_H_


#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>

class UpdatePoint{
	public:
		static rtx_insn* firstRealINSN(basic_block bb);
		static rtx_insn* middleRealINSN(basic_block bb);
		static rtx_insn* lastRealINSN(basic_block bb);
		static rtx_insn* lastRealSafeINSN(basic_block bb);
	private:
		static unsigned int countInsnBB(basic_block bb);
		static rtx_insn* findInsn(basic_block bb, unsigned int index);
};


#endif /* ASM_UPDATEPOINT_H_ */
