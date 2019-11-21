/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

#include <gcc-plugin.h>

#include "CFED_Plugin.h"
#include "CFEDcreator.h"
#include "ArmISA_Functions.h"
#include "Printer.h"


/**
 * Necessary structure, provides all data
 * about the pass.
 */
const struct pass_data CFED_PLUGIN_pass_data =
{
		.type = RTL_PASS,
		.name = "myPlugin",
		.optinfo_flags = OPTGROUP_NONE,
		.tv_id = TV_TREE_CLEANUP_CFG,
		.properties_required = 0,//(PROP_rtl | PROP_cfglayout),
		.properties_provided = 0,
		.properties_destroyed = 0,
		.todo_flags_start = 0,
		.todo_flags_finish = 0,
};

// ---------------------------------- Public Area ------------------------------------------------------------- \\

/**
 * Constructor of the pass
 * @param ctxt This is the gcc context (necessary to construct super class)
 * @param arguments Contains all arguments that were passed through the command line
 * @param argcounter Tells how many arguments that were passed
 */
CFED_PLUGIN::CFED_PLUGIN(gcc::context *ctxt, struct plugin_argument *arguments, int argcounter)
		: rtl_opt_pass(CFED_PLUGIN_pass_data, ctxt)
{
		argc = argcounter;			// nr. of arguments
		args = arguments;			// array containing arrguments (key,value)
}

/**
 * Overrides the gate-method of opt_pass class
 * Mostly used to perform some tests to see if the
 * pass is allowed to run
 * @ return bool Indicates if the pass may run or not
 */
bool CFED_PLUGIN::gate(function *fun)
{
	const char *name = IDENTIFIER_POINTER (DECL_NAME (current_function_decl) );		// Use macro's to get to the name

	if(ARM_ISA::supportedTarget(arm_cpu_option)){		// First check if the current ARM ISA is supported
		return isAllowedToRun(name);						// Then only execute if the current function must be protected
	}
	else{
		return false;
	}
}

/**
 * Overrides execute-function of opt_pass class
 * contains the actual code that must be executed
 * by the plugin
 * @return int Returns 0 if success
 */
unsigned int CFED_PLUGIN::execute(function *fun)
{
	// 1) Find the name of the function
	char* funName = (char*)IDENTIFIER_POINTER (DECL_NAME (current_function_decl) );

	// 2) Create the name of the directory to put all ouput-files in
	// Takes the form of GCC_Plugin_Output/<function_name>
	char dirName[512];
	strcpy(dirName, "GCC_Plugin_Output/");
	strcat(dirName, funName);

	printf("\x1b[92m GCC Plugin executing for function \x1b[92;1m %s \x1b[0m\n",funName);

	// 3)Create the needed directories
	mkdir("GCC_Plugin_Output", 0766);
	int status = mkdir(dirName, 0766);
	if (status == -1){
		int counter = rand() % 1000;
		char renameName[512];
		strcpy(renameName,dirName);
		strcat(renameName, (to_string(counter)).c_str());
		rename(dirName, renameName);
		mkdir(dirName, 0766);
		counter++;
	}

	// 4) Print out the desired info
	Printer::printRTL((char *)"RTL.txt");			// Print the RTL statements to file
	Printer::printEdges((char *)"Edges.txt");		// Print the CFE in edge form
	Printer::printAnalysis((char *)"Analysis.txt");	// Print the analyses for all basic blocks

	// 5) Implement the technique
	try{
		const char *techType = findArgumentValue("techniqueType");
		if(!strcmp(techType, "SigMon")){
			implementDetectionTechnique(false);
		}
		else if(!strcmp(techType, "fullCFED")){
			implementDetectionTechnique(true);
		}
		else{
			throw "Wrong technique type provided!\n";
		}

		Printer::printRTL((char*)"RTL_Protected.txt");

		printf("\x1b[92m--------------------- Plugin fully ran -----------------------\n\x1b[0m");
		return 0;
	}
	catch (const char* e){
		printf("\x1b[91m--------------------- Plugin did not execute completely!  -----------------------\x1b[0m\n\t%s\n", e);
		return 1;
	}

}

// ---------------------------------------------------- Private Area ------------------------------------- \\

/**
 * Generic function to get the value out of the
 * argv array, matching the given key
 * @param Key The key of the argument to get the value for
 * @return Value The const char* representation of the value
 */
const char* CFED_PLUGIN::findArgumentValue(const char* key){
	for (int i=0; i< argc; i++){
		if(!strcmp(args[i].key, key)){
			return args[i].value;
		}
	}
	char msg[1024];
	snprintf(msg, 1024, "Argument %s not found!\n\tRead %s to find out which arguments are needed!\n",
			key, "https://svs.icts.kuleuven.be/projects/svs_project052/wiki");
	throw (const char*) msg;
}

/**
 * Function that analyses all arguments and searches if the
 * current function was given as argument. Only then the pass
 * may execute.
 * If the argument is empty, execute for each function encounterd.
 * @param funName Constant Char* that holds the name of the function
 * @return bool Returns if the pass may be executed
 */
bool CFED_PLUGIN::isAllowedToRun(const char* funName){
	try{
		const char* f = findArgumentValue("function");
		if(lookup_attribute("noProtection", DECL_ATTRIBUTES(current_function_decl))){
			printf("\x1b[32m GCC Plugin not executing for function %s \x1b[0m\n",funName);
			printf("\t\x1b[96mFunction specified as not needing protection\x1b[0m\n");
			return false;
		}
		else if( (!strcmp(f, funName)) || (strlen(f) == 0 ) ){
			return true;
		}
		else{
			return false;
		}
	}
	catch (const char* e){
		printf("x1b[91mArgument x1b[91;1m'function' x1b[91m not found!\x1b[0m\n");
		return false;
	}
}

void CFED_PLUGIN::implementDetectionTechnique(bool intraBlockDet){
	try{
		const char* technique = findArgumentValue("techniqueSpecific");
		unsigned int selectiveLevel = atoi(findArgumentValue("selectiveLevel"));
		CFEDcreator* cfedCreator = new CFEDcreator();
		cfedCreator->implementTechnique(technique, intraBlockDet, selectiveLevel);
	} catch (const char* e){
		printf("\x1b[91mCFE Detection Technique was not implemented:\x1b[0m\n\t%s\n", e);
	}
}
