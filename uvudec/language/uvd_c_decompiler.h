/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_decompiler.h"
#include "uvd_types.h"

//Given disassembled instruction structures, produce code in given language
class UVDCDecompiler : public UVDDecompiler
{
public:
	UVDCDecompiler();
	~UVDCDecompiler();

	//Class specific init function
	uv_err_t init();

	virtual uv_err_t decompile(std::vector<UVDInstruction *> disassembledCode, std::string &highLevelCode, UVDDecompileNotes *notes);

public:
};

