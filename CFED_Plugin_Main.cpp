/*
 * This GCC Plugin has been developed during a research grant from the Baekeland program of the Flemish Agency for Innovation and Entrepreneurship (VLAIO) in cooperation with Televic Healthcare NV, under grant agreement IWT 150696.
 * Copyright (c) 2019 Jens Vankeirsbilck & KU Leuven LRD & Televic Healthcare NV.
 * Distributed under the MIT "Expat" License. (See accompanying file LICENSE.txt)
 */

/*
 * This is the main-file for the GCC plugin.
 * It defines necessary structs, our defined noProtection function attribute,
 * and adds the plugin to the PASS_MANAGER.
 */

#include <gcc-plugin.h>
#include <context.h>
#include <basic-block.h>
#include <rtl.h>
#include <tree-pass.h>
#include <tree.h>
#include <plugin.h>

#include <stdio.h>

#include "CFED_Plugin.h"


// Mandatory variable, indicates that a GPL compatible license is applied to this GCC plugin.
int plugin_is_GPL_compatible = 1;

static struct plugin_info myPlugin_info =
{
		.version = "3",
		.help = "GCC plugin for arm-none-eabi-7.3",
};

static struct plugin_gcc_version myPlugin_ver =
{
		.basever = "7.3",
};

// specify own noProtection attribute
static struct attribute_spec noProtection_attr =
{
		"noProtection", 0, 0, false, false, false, NULL, false
};

// Register all self-specified attributes
static void register_attributes(void *event_data, void *data){
	register_attribute(&noProtection_attr);
}


// Start point of the plugin
int plugin_init(struct plugin_name_args *info, struct plugin_gcc_version *ver){
	if(strncmp(ver->basever,myPlugin_ver.basever, strlen("7.3"))){
		return -1;
	}

	struct register_pass_info pass;
	pass.pass = new CFED_PLUGIN(g, info->argv, info->argc);
	pass.reference_pass_name = "*free_cfg";
	pass.ref_pass_instance_number = 1;
	pass.pos_op = PASS_POS_INSERT_AFTER;

	register_callback("myPlugin", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
	register_callback("myPlugin", PLUGIN_ATTRIBUTES, register_attributes, NULL);

	return 0;
}
