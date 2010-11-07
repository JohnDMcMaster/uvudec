/*
UVNet Universal Decompiler (uvudec)
Copyright 2009 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDASCIIART_CONFIG_H
#define UVDASCIIART_CONFIG_H

#include "uvd/config/config.h"

#define UVD_PROP_ACTION_USELESS_ASCII_ART				"plugin.uvdasciiart.action.print"
#define UVD_PROP_OUTPUT_USELESS_ASCII_ART				"plugin.uvdasciiart.output.useless_ascii_art"
#define UVD_PROP_OUTPUT_USELESS_ASCII_ART_DEFAULT		false

class UVDASCIIArtConfig
{
public:
	UVDASCIIArtConfig();
	~UVDASCIIArtConfig();
	
	uv_err_t init(UVDConfig *config);
	uv_err_t deinit();

	uv_err_t setConfigInterpreterLanguageInterface(const std::string &in);
	uv_err_t setConfigInterpreterLanguage(const std::string &in);
	uv_err_t getDefaultArchitectureFile(std::string &ret);

public:
	uvd_bool_t m_outputASCIIArt;
	int m_uselessASCIIArt;
};

extern UVDASCIIArtConfig *g_asciiArtConfig;

#endif

