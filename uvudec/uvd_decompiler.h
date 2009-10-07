/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/

#pragma once

#include "uvd.h"
#include "uvd_types.h"
#include "uvd_compiler.h"
#include "uvd_language.h"

//Extra information generated from decompiling
//Primary purpose is to decide if the code is better off as assembly
class UVDDecompileNotes
{
public:
	UVDDecompileNotes();
	
	uv_err_t getOptimalLanguage(int &langauge);
	
public:
	int m_optimalLangauge;
};

//Given disassembled instruction structures, produce code in given language
class UVDDecompiler
{
public:
	UVDDecompiler();
	virtual ~UVDDecompiler();

	//Class specific init function
	virtual uv_err_t init();
	//Class specific deinit function
	virtual uv_err_t deinit();

	virtual uv_err_t decompile(std::vector<UVDInstruction *> disassembledCode, std::string &highLevelCode, UVDDecompileNotes *notes) = NULL;

	//Get the best matching decompiler for given compiler
	static uv_err_t getDecompiler(UVDCompiler *compiler, UVDDecompiler **decompiler);

public:
	UVD *m_uvd;
	//The specific compiler we are targetting
	UVDCompiler *m_compiler;
};
