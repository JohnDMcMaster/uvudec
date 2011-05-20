/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/config/raw_arg.h"


/*
UVDRawArg
*/

UVDRawArg::UVDRawArg() {
}

UVDRawArg::~UVDRawArg() {
}
	
/*
UVDCommandLineArg
*/
UVDCommandLineArg::UVDCommandLineArg() {
}

UVDCommandLineArg::~UVDCommandLineArg() {
}

unsigned int UVDCommandLineArg::getSource() {
	return UVD_ARG_FROM_COMMAND_LINE;
}

uv_err_t UVDCommandLineArg::fromStringVector( const std::vector<std::string> in, UVDRawArgs &out, bool skipFirst ) {
	for( unsigned int i = skipFirst ? 1 : 0; i < in.size(); ++i ) {
		UVDCommandLineArg *arg = NULL;
		
		arg = new UVDCommandLineArg();
		arg->m_token = in[i];
		out.m_args.push_back(arg);
	}
	return UV_ERR_OK;
}

/*
UVDConfigFileArg
*/

UVDConfigFileArg::UVDConfigFileArg() {
	//m_fileName;
	m_fileLine = 0;
}

UVDConfigFileArg::~UVDConfigFileArg() {
}
	
unsigned int UVDConfigFileArg::getSource() {
	return UVD_ARG_FROM_CONFIG;
}

/*
UVDRawArgs
*/

UVDRawArgs::UVDRawArgs() {
}

UVDRawArgs::~UVDRawArgs() {
	for( std::vector<UVDRawArgs *>::size_type i = 0; i < m_args.size(); ++i ) {
		delete m_args[i];
	}
}

