/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/core/uvd.h"
#include "uvdasm/architecture.h"
#include "uvdasm/instruction.h"
#include "uvd/util/types.h"
#include "uvd/core/runtime.h"
#include "uvd/language/format.h"
#include "uvd/util/util.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <string.h>

/*
UVDGBInstruction
*/

UVDGBInstruction::UVDGBInstruction()
{
	m_architecture = NULL;
}

UVDGBInstruction::~UVDGBInstruction()
{
	deinit();
}

uv_err_t UVDGBInstruction::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDGBInstruction::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDGBInstruction::print_disasm(std::string &out)
{
	out = "<insert instruction here...>";
	return UV_ERR_OK;
}

uv_err_t UVDGBInstruction::analyzeControlFlow()
{
	std::string action;
	uint32_t followingPos = 0;
	UVDDisasmArchitecture *architecture = NULL;

	followingPos = m_offset + m_inst_size;
	architecture = (UVDDisasmArchitecture *)m_uvd->m_runtime->m_architecture;

	action = getShared()->m_action;

	//printf("Action: %s, type: %d\n", action.c_str(), getShared()->m_inst_class);
	//See if its a call instruction
	if( getShared()->m_inst_class == UVD_INSTRUCTION_CLASS_CALL )
	{
		uv_assert_err_ret(g_uvd->m_analyzer->insertCallReference(targetAddress, startPos));
	}
	else if( getShared()->m_inst_class == UVD_INSTRUCTION_CLASS_JUMP )
	{
		uv_assert_err_ret(g_uvd->m_analyzer->insertJumpReference(targetAddress, startPos));
	}
	return UV_ERR_OK;
}

uv_err_t UVDGBInstruction::parseCurrentInstruction(UVDIteratorCommon &iterCommon)
{
	/*
	printf_debug("m_nextPosition: 0x%.8X\n", iterCommon.m_nextPosition);
		
	//Used to get delta for copying the data we just iterated over
	startPosition = iterCommon.m_nextPosition;

	//We should be garaunteed a valid address at current position by definition
	rcTemp = iterCommon.consumeCurrentExecutableAddress(&opcode);
	uv_assert_err_ret(rcTemp);
	uv_assert_ret(rcTemp != UV_ERR_DONE);
	//uv_assert_err_ret(data->readData(iterCommon.m_nextPosition, (char *)&opcode));	
	printf_debug("Just read (now pos 0x%.8X, size: 0x%02X) 0x%.2X\n", iterCommon.m_nextPosition, iterCommon.m_currentSize, opcode);	
	*/
	
	return UV_ERR_OK;
}	

