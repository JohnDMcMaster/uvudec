/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_ARG_PROPERTY_H
#define UVD_ARG_PROPERTY_H

//Action
#define UVD_PROP_ACTION_HELP					"action.help"
#define UVD_PROP_ACTION_VERSION					"action.version"
#define UVD_PROP_ACTION_USELESS_ASCII_ART		"action.print_ascii_art"
//Debug
#define UVD_PROP_DEBUG_LEVEL					"debug.level"
#define UVD_PROP_DEBUG_INIT						"debug.init"
#define UVD_PROP_DEBUG_PROCESSING				"debug.processing"
#define UVD_PROP_DEBUG_ANALYSIS					"debug.analysis"
#define UVD_PROP_DEBUG_PRINTING					"debug.printing"
#define UVD_PROP_DEBUG_FILE						"debug.file"
//Config
#define UVD_PROP_CONFIG_LANGUAGE				"config.language"
//Target
#define UVD_PROP_TARGET_ADDRESS_INCLUDE_MIN		"target.address_include.min"
#define UVD_PROP_TARGET_ADDRESS_INCLUDE_MAX		"target.address_include.max"
#define UVD_PROP_TARGET_ADDRESS_EXCLUDE_MIN		"target.address_exclude.min"
#define UVD_PROP_TARGET_ADDRESS_EXCLUDE_MAX		"target.address_exclude.max"
#define UVD_PROP_TARGET_FILE					"target.file"
#define UVD_PROP_TARGET_ADDRESS					"target.address"
//Analysis
#define UVD_PROP_ANALYSIS_DIR					"analysis.dir"
#define UVD_PROP_ANALYSIS_ONLY					"analysis.only"
#define UVD_PROP_ANALYSIS_FLOW_TECHNIQUE		"analysis.flow_technique"
//Output
#define UVD_PROP_OUTPUT_OPCODE_USAGE			"output.opcode_usage"
#define UVD_PROP_OUTPUT_JUMPED_ADDRESSES		"output.jumped_addresses"
#define UVD_PROP_OUTPUT_CALLED_ADDRESSES		"output.called_addresses"
#define UVD_PROP_OUTPUT_USELESS_ASCII_ART		"output.useless_ascii_art"
#define UVD_PROP_OUTPUT_ADDRESS_COMMENT			"output.address_comment"
#define UVD_PROP_OUTPUT_ADDRESS_LABEL			"output.address_label"
#define UVD_PROP_OUTPUT_FILE					"output.file"

#endif
