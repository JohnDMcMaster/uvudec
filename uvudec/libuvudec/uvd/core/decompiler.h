/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CORE_DECOMPILER_H
#define UVD_CORE_DECOMPILER_H

#include "uvd/core/uvd.h"
#include "uvd/util/types.h"
#include "uvd/compiler/compiler.h"
#include "uvd/language/language.h"

//Extra information generated from decompiling
//Primary purpose is to decide if the code is better off as assembly
class UVDDecompileNotes
{
public:
	UVDDecompileNotes();
	
	uv_err_t getOptimalLanguage(int &Language);
	
public:
	int m_optimalLanguage;
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

	virtual uv_err_t decompile(std::vector<UVDInstruction *> disassembledCode, std::string &highLevelCode, UVDDecompileNotes *notes) = 0;

	//Get the best matching decompiler for given compiler
	static uv_err_t getDecompiler(UVDCompiler *compiler, UVDDecompiler **decompiler);

public:
	UVD *m_uvd;
	//The specific compiler we are targetting
	UVDCompiler *m_compiler;
};

#endif

