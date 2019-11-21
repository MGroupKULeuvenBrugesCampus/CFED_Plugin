/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>
#include <basic-block.h>
#include <rtl.h>
#include <tree.h>

#include "InstrType.h"

/**
 * Method to determine whether or not the provided
 * rtx_insn is a compare instruction
 */
bool InstrType::isCompare(rtx_insn* expr){
	rtx innerExpr = XEXP(expr, 3);
	return findCode(innerExpr, COMPARE);
}

/**
 * Method to determine whether or not the provided
 * rtx_insn is a conditionally executed instruction
 */
bool InstrType::isCondExec(rtx_insn* expr){
	rtx innerExpr = XEXP(expr, 3);
	return findCode(innerExpr, COND_EXEC);
}

/**
 * Method to determine whether or not the provided
 * rtx_insn is a return instruction
 */
bool InstrType::isReturn(rtx_insn* expr){
	rtx innerExpr = XEXP(expr, 3);
	bool ret =  findCode(innerExpr, RETURN);
	bool simpRet = findCode(innerExpr, SIMPLE_RETURN);
	return (ret || simpRet);
}

/**
 * Method to determine whether or not the provided
 * basic block is an exit basic block
 */
bool InstrType::isExitBlock(basic_block bb){
	unsigned int nrOfSuccs = 0;
	int idSuccs = -5;
	edge e;
	edge_iterator ei;
	FOR_EACH_EDGE(e, ei, bb->succs){
		nrOfSuccs++;
		if(nrOfSuccs == 1){
			idSuccs = e->dest->index - 2;
		}
	}
	return ( (nrOfSuccs == 1) && (idSuccs == -1) );
}

/**
 * Method to determine whether or not the provided
 * rtx_insn is a use instruction (RTL syntax)
 */
bool InstrType::isUse(rtx_insn* expr){
    if (INSN_P(expr)){
	    rtx innerExpr = XEXP(expr, 3);
	    rtx_code exprCode = (rtx_code) innerExpr->code;
	    return (exprCode == USE);
    }
    else{
        return false;
    }
}

/**
 * Method to determine whether or not the provided
 * rtx_insn is an unspec instruction (RTL syntax)
 */
bool InstrType::isUnspec(rtx_insn* expr){
	if(INSN_P(expr)){
		rtx innerExpr = XEXP(expr, 3);
		rtx_code exprCode = (rtx_code) innerExpr->code;
		return (exprCode == UNSPEC);
	}
	else{
		return false;
	}
}

/**
 * Method to determine whether or not the provided
 * rtx_insn is a clobber instruction (RTL syntax)
 */
bool InstrType::isClobber(rtx_insn* expr){
	if(INSN_P(expr)){
		rtx inner = XEXP(expr, 3);
		rtx_code innerCode = (rtx_code) inner->code;
		return (innerCode == CLOBBER);
	}
	else{
		return false;
	}
}

/**
 * Method to determine whether or not the provided
 * rtx_insn is a volatile unspec instruction (RTL syntax)
 */
bool InstrType::isUnspecVolatile(rtx_insn* expr){
	if(INSN_P(expr)){
		rtx inner = XEXP(expr, 3);
		rtx_code innerCode = (rtx_code) inner->code;
		return (innerCode == UNSPEC_VOLATILE);
	}
	else{
		return false;
	}
}

/**
 * Method to determine whether or not the provided
 * rtx_insn is a conditional jump instruction
 */
bool InstrType::isCondJump(rtx_insn* insn){
	if(isReturn(insn)){
		return false;
	}
	else{
		rtx set = XEXP(insn, 3);
		return (findCode(set, IF_THEN_ELSE) && JUMP_P(insn) );
	}
}

/**
 * Method to determine whether or not the provided
 * rtx_insn is a compare-branch zero instruction
 */
bool InstrType::isCBZ(rtx_insn* insn){
	rtx_insn* innerExpr = insn;//XEXP(insn, 4);
	bool isPar = findCode(innerExpr, PARALLEL);
	bool isIfTE = findCode(innerExpr, IF_THEN_ELSE);
	bool isConstIntZero = findConstIntWithNumber(innerExpr, 0);
	return (isPar && isIfTE && isConstIntZero && JUMP_P(insn));
}

/**
 * Method to determine the condition code of the
 * provided rtx_insn
 */
rtx_code InstrType::getCondCode(rtx_insn* condExpr){
	rtx set = XEXP(condExpr, 3);
	return getConditionalCode(set, IF_THEN_ELSE);
}

/**
 * Method to determine the contrary condition code
 * of the provided condition code.
 */
rtx_code InstrType::findContraryConditionalCode(enum rtx_code condition){
	switch(condition){
		case EQ:
			return NE;
			break;
		case NE:
			return EQ;
			break;
		case GT:
			return LE;
			break;
		case LE:
			return GT;
			break;
		case GTU:
			return LEU;
			break;
		case LEU:
			return GTU;
			break;
		case LT:
			return GE;
			break;
		case GE:
			return LT;
			break;
		case LTU:
			return GEU;
			break;
		case GEU:
			return LTU;
			break;
		default:
			printf("False Conditional code, returned the code");
			return condition;
			break;
	}
}

// ----------------------- Private Section -------------------------- \\

/**
 * Method to find the provided rtx_code in the given
 * rtx. Is a recursive method to make sure all fields of
 * the provided rtx is examined.
 */
bool InstrType::findCode(rtx expr, rtx_code code){
	if( expr == 0x00 ){
		return false;
	}

	rtx_code exprCode = (rtx_code) expr->code;			// Get the code of the expression
	const char* format = GET_RTX_FORMAT(exprCode);		// Get the format of the expression, tells what operands are expected

	if(exprCode == code){					// Test if expression is a CODE expression
		return true;
	}
	else if(exprCode == ASM_OPERANDS){
		return false;
	}
	else{
		for (int x=0; x < GET_RTX_LENGTH(exprCode); x++){	// Loop over all characters in the format
			if(format[x] == 'e'){							// Test if they are an expression
				rtx subExpr = XEXP(expr,x);					// Get the expression
				if (findCode(subExpr, code)){				// Recursive call to this function
					return true;
				}
			}
			else if(format[x] == 'E'){						// Test if a Vector
				for(int i=0; i<XVECLEN(expr,0);i++){		// Loop over all expressions in the vector
					rtx subExpr = XVECEXP(expr, 0, i);		// Get the expression
					if(findCode(subExpr, code)){			// Recursive call to this function
						return true;
					}
				}
			}
		}
	}
	return false;
}

/**
 * Method to find a CONST_INT rtx with the provided number in the given
 * rtx. Is a recursive method to make sure all fields of
 * the provided rtx is examined.
 */
bool InstrType::findConstIntWithNumber(rtx expr, unsigned int number){
	if( expr == 0x00){
		return false;
	}
	rtx_code exprCode = (rtx_code) expr->code;			// Get the code of the expression
	const char* format = GET_RTX_FORMAT(exprCode);		// Get the format of the expression, tells what operands are expected

	if(exprCode == CONST_INT){								// Test if expression is a Const_Int expression
		if(XINT(expr,0) == number){
			return true;
		}
	}
	else if(exprCode == ASM_OPERANDS){
			return false;
		}
	else{
		for (int x=0; x < GET_RTX_LENGTH(exprCode); x++){	// Loop over all characters in the format
			if(format[x] == 'e'){							// Test if they are an expression
				rtx subExpr = XEXP(expr,x);					// Get the expression
				if (findConstIntWithNumber(subExpr, number)){				// Recursive call to this function
					return true;
				}
			}
			else if(format[x] == 'E'){						// Test if a Vector
				for(int i=0; i<XVECLEN(expr,0);i++){		// Loop over all expressions in the vector
					rtx subExpr = XVECEXP(expr, 0, i);		// Get the expression
					if(findConstIntWithNumber(subExpr, number)){			// Recursive call to this function
						return true;
					}
				}
			}
		}
	}
	return false;
}

/**
 * Method to find the provided conditional rtx_code in the given
 * rtx. Is a recursive method to make sure all fields of
 * the provided rtx is examined.
 */
rtx_code InstrType::getConditionalCode(rtx expr, rtx_code code){
	if( expr == 0x00){
			return (rtx_code)-1;
		}
		rtx_code exprCode = (rtx_code) expr->code;			// Get the code of the expression
		const char* format = GET_RTX_FORMAT(exprCode);		// Get the format of the expression, tells what operands are expected

		if(exprCode == code){
			rtx cond = XEXP(expr, 0);
			return GET_CODE(cond);// Test if expression is a CODE expression
		}
		else{
			for (int x=0; x < GET_RTX_LENGTH(exprCode); x++){	// Loop over all characters in the format
				if(format[x] == 'e'){							// Test if they are an expression
					rtx subExpr = XEXP(expr,x);					// Get the expression
					rtx_code condCode = getConditionalCode(subExpr, code);
					if (condCode!= -1){				// Recursive call to this function
						return condCode;
					}
				}
				else if(format[x] == 'E'){						// Test if a Vector
					for(int i=0; i<XVECLEN(expr,0);i++){		// Loop over all expressions in the vector
						rtx subExpr = XVECEXP(expr, 0, i);		// Get the expression
						rtx_code condCode = getConditionalCode(subExpr, code);
						if(condCode != -1){			// Recursive call to this function
							return condCode;
						}
					}
				}
			}
		}
		return (rtx_code)-1;
}
