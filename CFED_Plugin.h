/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/*
 * Header file of the CFED_PLUGIN class, which is the actual RTL pass.
 * This class implements the necessary gate and execute functions.
 */

#ifndef CFED_PLUGIN_H_
#define CFED_PLUGIN_H_

#include <gcc-plugin.h>
#include <context.h>
#include <basic-block.h>
#include <rtl.h>
#include <tree-pass.h>
#include <tree.h>

#include <stdio.h>


class CFED_PLUGIN : public rtl_opt_pass{
	public:
		CFED_PLUGIN(gcc::context *ctxt, struct plugin_argument *arguments, int argcounter);

		bool gate(function *fun);

		unsigned int execute(function *fun);

	private:
		const char* findArgumentValue(const char* key);

		bool isAllowedToRun(const char* funName);

		void implementDetectionTechnique(bool intraBlockDet);

		int argc;
		struct plugin_argument *args;

};

#endif /* CFED_PLUGIN_H_ */
