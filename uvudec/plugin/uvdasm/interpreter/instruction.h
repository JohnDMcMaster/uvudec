/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_INTERPRETED_INSTRUCTION_H
#define UVD_INTERPRETED_INSTRUCTION_H

#include "uvd_types.h"

/*
For extracting results after running an instruction through the interpreter
*/

class UVDInterpretedInstruction
{
public:
	UVDInterpretedInstruction();
	virtual ~UVDInterpretedInstruction();
	
	//Parse interpreted output
	virtual uv_err_t parse(const UVDVariableMap &attributes);
	
protected:
	uv_err_t parseRequired(const UVDVariableMap &attributes, const std::string &attribute, uint32_t *val);
	uv_err_t parseRequired(const UVDVariableMap &attributes, const std::string &attribute, std::string *val);
	uv_err_t parseOptional(const UVDVariableMap &attributes, const std::string &attribute, uint32_t *val, uint32_t valDefault);
	uv_err_t parseOptional(const UVDVariableMap &attributes, const std::string &attribute, std::string *val, const std::string &valDefault);
	uv_err_t parseCore(const UVDVariableMap &attributes, const std::string &key, uint32_t *val);
	uv_err_t parseCore(const UVDVariableMap &attributes, const std::string &key, std::string *val);
	
public:
	std::string m_symbol;
};

class UVDInterpretedCall : public UVDInterpretedInstruction
{
public:
	UVDInterpretedCall();
	virtual ~UVDInterpretedCall();

	virtual uv_err_t parse(const UVDVariableMap &attributes);

public:
	uint32_t m_callTarget;
};

class UVDInterpretedBranch : public UVDInterpretedInstruction
{
public:
	UVDInterpretedBranch();
	virtual ~UVDInterpretedBranch();

	virtual uv_err_t parse(const UVDVariableMap &attributes);

public:
	uint32_t m_branchTarget;
};

#endif
