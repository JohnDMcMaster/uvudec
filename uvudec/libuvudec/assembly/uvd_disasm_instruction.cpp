/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/


#include "uvd.h"
#include "uvd_instruction.h"
#include "uvd_types.h"
#include "uvd_format.h"
#include "uvd_util.h"
#include "main.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <string.h>

UVDDisasmInstructionShared::UVDDisasmInstructionShared()
{
	m_opcode_length = 0;
	memset(m_opcode, 0, sizeof(m_opcode));
	m_total_length = 0;
	m_opcode_range_offset = 0;
	m_cpi = 0;
	m_cpi_low = 0;
	m_cpi_hi = 0;
	m_inst_class = UVD_INSTRUCTION_CLASS_UNKNOWN;
	m_config_line_syntax = 0;
	m_config_line_usage = 0;
	m_isImmediateOnlyFunction = UV_ERR_GENERAL;
}

UVDDisasmInstructionShared::~UVDDisasmInstructionShared()
{
	deinit();
}

uv_err_t UVDDisasmInstructionShared::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDDisasmInstructionShared::deinit()
{
	for( std::vector<UVDDisasmOperandShared *>::iterator iter = m_operands.begin(); iter != m_operands.end(); ++iter )
	{
		delete *iter;
	}
	m_operands.clear();
	return UV_ERR_OK;
}

std::string UVDDisasmInstructionShared::getHumanReadableUsage()
{
	std::string sRet;

	sRet += m_memoric;

	for( unsigned int i = 0; i < m_operands.size(); ++i )
	{	
		UVDDisasmOperandShared *op = m_operands[i];
		if( !op )
		{
			sRet += "<ERROR>";
		}
		else
		{
			sRet += " ";
			sRet += op->m_name;
		}
		
		if( i + 1 < m_operands.size() )
		{
			sRet += ",";
		}
	}
	
	return sRet;
}

uv_err_t UVDDisasmInstructionShared::analyzeAction()
{
	if( m_action.find("CALL") != std::string::npos )
	{
		m_inst_class = UVD_INSTRUCTION_CLASS_CALL;
	}
	else if( m_action.find("GOTO") != std::string::npos )
	{
		m_inst_class = UVD_INSTRUCTION_CLASS_JUMP;
	}
	else
	{
		m_inst_class = UVD_INSTRUCTION_CLASS_UNKNOWN;
	}
	
	m_isImmediateOnlyFunction = isImmediateOnlyFunctionCore();
	
	return UV_ERR_OK;
}

uv_err_t UVDDisasmInstructionShared::isImmediateOnlyFunction()
{
	/*
	FIXME:
	Make this cached or something
	There is no reason we should recompute this every time
	Ideal situation is should be computed when action is set
	
	Should be suitable to do during analyzeAction
	*/
	
	return m_isImmediateOnlyFunction;
}

uv_err_t UVDDisasmInstructionShared::getImmediateOnlyFunctionAttributes(/*std::string &func,
		std::string &identifier,*/ uint32_t *identifierSizeBitsOut, uint32_t *immediateOffsetOut)
{
	uint32_t identifierSizeBits = 0;
	uint32_t immediateOffset = 0;
	std::string func;
	std::string identifier;
			

	//printf("m_action: %s\n", m_action.c_str());
	uv_assert_err_ret(isImmediateOnlyFunction());
	
	//Split it up a bit	
	//If this fails it doesn't meet the criteria
	uv_assert_err_ret(parseFunc(m_action, func, identifier));

	//identifiers type is encoded as a prefix
	//<var> := <type>_<name>
	//<type> := <sign char><size in bits>
	//u8_0
	if( identifier.find("8") != std::string::npos )
	{
		identifierSizeBits = 8;
	}
	else if( identifier.find("16") != std::string::npos )
	{
		identifierSizeBits = 16;
	}
	else if( identifier.find("32") != std::string::npos )
	{
		identifierSizeBits = 32;
	}
	else
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	//If we have a single immediate, its offset should be the size of the opcode
	immediateOffset = m_opcode_length;
	
	uv_assert_ret(identifierSizeBitsOut);
	*identifierSizeBitsOut = identifierSizeBits;
	
	uv_assert_ret(immediateOffsetOut);
	*immediateOffsetOut = immediateOffset;

	return UV_ERR_OK;
}

uv_err_t UVDDisasmInstructionShared::isImmediateOnlyFunctionCore()
{
	std::string name;
	std::string content;
	
	//Split it up a bit	
	//If this fails it doesn't meet the criteria
	if( UV_FAILED(parseFunc(m_action, name, content)) )
	{
		return UV_ERR_GENERAL;
	}

	//Simple, only immediate
	//ACTION=CALL(u16_0)
	if( UV_SUCCEEDED(isConfigIdentifier(content)) )
	{
		return UV_ERR_OK;
	}
	//Complex, some ugly expression
	//ACTION=CALL(%PC&0x1F00+u8_0+0x6000)
	else
	{
		return UV_ERR_GENERAL;
	}
}

/*
UVDDisasmInstruction
*/

UVDDisasmInstruction::UVDDisasmInstruction()
{
	m_shared = NULL;
	m_offset = 0;
	m_inst_size = 0;
	m_uvd = NULL;
	memset(m_inst, 0, sizeof(m_inst));
}

UVDDisasmInstruction::~UVDDisasmInstruction()
{
	deinit();
}

uv_err_t UVDDisasmInstruction::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDDisasmInstruction::deinit()
{
	//printf("this UVDDisasmInstruction under deinit: 0x%08X, operands: 0x%02X\n", (int)this, m_operands.size());
	//fflush(stdout);
	/*
	//m_shared is not owned by this as there can be n to 1, as is obviously not m_uvd
	for( std::vector<UVDOperand *>::iterator iter = m_operands.begin(); iter != m_operands.end(); ++iter )
	{
		delete *iter;
	}
	*/
	m_operands.clear();

	return UV_ERR_OK;
}

UVDDisasmInstructionShared *UVDDisasmInstruction::getShared()
{
	return (UVDDisasmInstructionShared *)m_shared;
}

uv_err_t UVDDisasmInstruction::print_disasm(std::string &out)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string operand_pad = "";
	UVDConfig *config = g_config;
	char buff[256];
	
	uv_assert_ret(config);
	

	printf_debug("inst size: %d\n", m_inst_size);
	uv_assert(m_inst_size);

	if( m_operands.size() )
	{
		operand_pad = " ";
	}

	printf_debug("beginning print\n");
	
	if( config->m_asm_instruction_info )
	{
		snprintf(buff, sizeof(buff), "%s (0x%.2X/%s)%s", getShared()->m_memoric.c_str(), ((unsigned int)m_inst[0]) & 0xFF, getShared()->m_desc.c_str(), operand_pad.c_str());
		out += buff;
	}
	else
	{
		out += getShared()->m_memoric;
		out += operand_pad;
	}
	if( !m_operands.empty() )
	{
		printf_debug("Has operand\n");
		printf_debug("Pre operand buff: <%s>\n", buff);
		for( unsigned int i = 0; i < m_operands.size(); ++i )
		{
			UVDDisasmOperand *operand = (UVDDisasmOperand *)m_operands[i];

			uv_assert_err_ret(operand->printDisassemblyOperand(out));

			if( i + 1 < m_operands.size() )
			{
				/* note that strncpy would return dest pointer, not num copied */
				out += PRINT_OPERAND_SEPERATOR;
			}

		}
	}
	else
	{
		printf_debug("No operand\n");
	}
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

//Given an identified instruction operand, parse the next operand out of the remaining binary (data)
uv_err_t UVDDisasmInstruction::parseOperands(UVDIteratorCommon *uvdIter,
		std::vector<UVDDisasmOperandShared *> ops_shared, std::vector<UVDOperand *> &operands)
{
	UVDData *data = NULL;;
	uv_assert_ret(m_uvd);
	data = m_uvd->getData();
	uv_assert_ret(data);
	
	//uv_assert(instruction);
	//shared = instruction->m_shared;
	//uv_assert(shared);
	//We should be initializing this, should not have been touched yet
	operands.clear();
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
	for( unsigned int i = 0; i < ops_shared.size(); ++i )
	{
		printf_debug("Loop op\n");
		UVDDisasmOperandShared *op_shared = ops_shared[i];
		UVDDisasmOperand *op = NULL;
		uv_err_t rcParse = UV_ERR_GENERAL;
		
		uv_assert_ret(op_shared);
		
		op = new UVDDisasmOperand();
		uv_assert_ret(op);
		op->m_instruction = this;
		
		op->m_shared = op_shared;
		
		rcParse = op->parseOperand(uvdIter);
		uv_assert_err_ret(rcParse);
		//Truncated analysis?
		if( rcParse == UV_ERR_DONE )
		{
			return UV_ERR_DONE;
		}
		
		uv_assert_ret(op->m_shared);
		//printf_debug("Linked %s\n", op->m_shared->m_name.c_str());
		
		operands.push_back(op);
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDDisasmInstruction::collectVariables(UVDVariableMap &environment)
{
	uv_err_t rc = UV_ERR_GENERAL;

	environment.clear();
	
	for( std::vector<UVDOperand *>::size_type i = 0; i < m_operands.size(); ++i )
	{
		UVDDisasmOperand *operand = (UVDDisasmOperand *)m_operands[i];
		std::string sKey;
		UVDVarient vValue;
		
		uv_assert(operand);
		uv_assert_err(operand->getVariable(sKey, vValue));
		if( !sKey.empty() )
		{
			environment[sKey] = vValue;
		}
	}
	
	rc = UV_ERR_OK;	

error:
	return UV_DEBUG(rc);
}

uv_err_t UVDDisasmInstruction::analyzeControlFlow()
{
	std::string action;
	uint32_t followingPos = 0;
	UVDDisasmArchitecture *architecture = NULL;

	followingPos = m_offset + m_inst_size;
	architecture = (UVDDisasmArchitecture *)m_uvd->m_architecture;

	action = getShared()->m_action;

	//printf_debug("Next instruction (start: 0x%.8X, end: 0x%.8X): %s\n", startPos, followingPos, getShared()->m_memoric.c_str());

	/*
	We must extract the symbols from this
	A human can see that the function call contained a PC relative jump + 
	
	Assumptions for now
	-All jumps are local symbols to a function, they should be created for completeness
	-PC relative jumps may not need tinkering with for now if need to cut corners since they still work
	
	First goal would be to get easy symbols like so
	NAME=LCALL
	DESC=Long Call
	USAGE=0x12,u16_0
	SYNTAX=u16_0
	ACTION=CALL(u16_0)


	NAME=LJMP
	DESC=Long Jump
	USAGE=0x02,u16_0
	SYNTAX=u16_0
	ACTION=GOTO(u16_0)

	LJMP seems like a rare instruction that current analysis would probably mess up anyway 
	(unless you are using a bad compiler anyway)
	Haha I wrote the above and not too much to my surprise
	I checked the Candela file and their assembler/compiler doesn't use AJMP
	So I have the easy case

	ACALL is also rare, but not uneard of
	Suspect these to be assembly routines, defintly seems clustered
	Or maybe spinning?
	[mcmaster@gespenst uvudec]$ cat candela.asm |fgrep ACALL
	ACALL #0x1111
	ACALL #0x1108
	ACALL #0x1100
	ACALL #0x1402
	ACALL #0x1502
	ACALL #0x0818
	ACALL #0x0824
	ACALL #0x0830
	ACALL #0x0835
	ACALL #0xB110
	ACALL #0x3131
	ACALL #0xB124
	ACALL #0xB132
	ACALL #0xB100
	ACALL #0xB101
	ACALL #0xB104
	ACALL #0xB101
	ACALL #0xB105
	ACALL #0xB140
	ACALL #0xB141
	ACALL #0xB152
	ACALL #0x0EC3
	ACALL #0xCED0
	ACALL #0x5500
	*/

	//printf("Action: %s, type: %d\n", action.c_str(), getShared()->m_inst_class);
	//See if its a call instruction
	if( getShared()->m_inst_class == UVD_INSTRUCTION_CLASS_CALL )
	{
		/*
		NAME=ACALL
		DESC=Absolute Call (page 3)
		USAGE=0x71,u8_0
		SYNTAX=u8_0
		ACTION=CALL(%PC&0x1F00+u8_0+0x6000)
		*/

		UVDVariableMap environment;
		UVDVariableMap mapOut;

		uv_assert_err_ret(collectVariables(environment));
					
		/*
		Add iterator specific environment
		*/

		//Register environment
		//PC/IP is current instruction location
		environment["PC"] = UVDVarient(followingPos);

		//About 0.03 sec per exec...need to speed it up
		//Weird...cast didn't work to solve pointer incompatibility
		uv_assert_ret(architecture->m_interpreter);
		uv_assert_err_ret(architecture->m_interpreter->interpretKeyed(action, environment, mapOut));
		
		uv_assert_err_ret(analyzeCall(followingPos, mapOut));
		
	}
	else if( getShared()->m_inst_class == UVD_INSTRUCTION_CLASS_JUMP )
	{
		/*
		NAME=JNB
		DESC=Jump if Bit Not Set
		USAGE=0x30,u8_0,u8_1
		SYNTAX=u8_0,u8_1
		ACTION=GOTO(%PC+u8_1)
		*/

		UVDVariableMap environment;
		UVDVariableMap mapOut;

		uv_assert_err_ret(collectVariables(environment));
					
		/*
		Add iterator specific environment
		*/

		//Register environment
		//PC/IP is current instruction location
		environment["PC"] = UVDVarient(followingPos);

		//About 0.03 sec per exec...need to speed it up
		//Weird...cast didn't work to solve pointer incompatibility
		uv_assert_ret(architecture->m_interpreter);
		uv_assert_err_ret(architecture->m_interpreter->interpretKeyed(action, environment, mapOut));

		uv_assert_err_ret(analyzeJump(followingPos, mapOut));
	}
	return UV_ERR_OK;
}

#define BASIC_SYMBOL_ANALYSIS			

uv_err_t UVDDisasmInstruction::analyzeCall(uint32_t startPos, const UVDVariableMap &attributes)
{
	std::string sAddr;
	uint32_t targetAddress = 0;
	UVDDisasmArchitecture *architecture = NULL;
 	
 	architecture = (UVDDisasmArchitecture *)g_uvd->m_architecture;
	uv_assert_ret(attributes.find(SCRIPT_KEY_CALL) != attributes.end());
	(*attributes.find(SCRIPT_KEY_CALL)).second.getString(sAddr);
	targetAddress = (uint32_t)strtol(sAddr.c_str(), NULL, 0);
	
	uv_assert_err_ret(g_uvd->m_analyzer->insertCallReference(targetAddress, startPos));
	//uv_assert_err(insertReference(targetAddress, startPos, ));
	architecture->updateCache(startPos, attributes);

#ifdef BASIC_SYMBOL_ANALYSIS
	/*
	Only simple versions can be parsed for now
	Must have a single immediate as the target value with no calculation required
	*/
	uv_assert_ret(m_shared);
	if( UV_SUCCEEDED(getShared()->isImmediateOnlyFunction()) )
	{
		uint32_t relocatableDataSizeBits = 0;
		uint32_t relocationPos = 0;
		uint32_t immediateOffset = 0;

		uv_assert_err_ret(getShared()->getImmediateOnlyFunctionAttributes(&relocatableDataSizeBits, &immediateOffset));

		relocationPos = startPos + immediateOffset;

		//We know the location of a call symbol relocation
		//uv_assert_ret(m_symbolManager);
		uv_assert_err_ret(g_uvd->m_analyzer->m_symbolManager.addAbsoluteFunctionRelocationByBits(targetAddress,
				relocationPos, relocatableDataSizeBits));
	}
#endif

	return UV_ERR_OK;
}

#undef BASIC_SYMBOL_ANALYSIS

uv_err_t UVDDisasmInstruction::analyzeJump(uint32_t startPos, const UVDVariableMap &attributes)
{
	std::string sAddr;
	uint32_t targetAddress = 0;

	uv_assert_ret(attributes.find(SCRIPT_KEY_JUMP) != attributes.end());
	(*attributes.find(SCRIPT_KEY_JUMP)).second.getString(sAddr);
	targetAddress = (uint32_t)strtol(sAddr.c_str(), NULL, 0);
	
	uv_assert_err_ret(g_uvd->m_analyzer->insertJumpReference(targetAddress, startPos));
	((UVDDisasmArchitecture *)(g_uvd->m_architecture))->updateCache(startPos, attributes);

#ifdef BASIC_SYMBOL_ANALYSIS			
	uv_assert_ret(instruction);
	uv_assert_ret(instruction->m_shared);
	if( UV_SUCCEEDED(instruction->m_shared->isImmediateOnlyFunction()) )
	{
		uint32_t relocatableDataSizeBits = 0;
		uint32_t relocationPos = 0;
		uint32_t immediateOffset = 0;

		uv_assert_err_ret(instruction->m_shared->getImmediateOnlyFunctionAttributes(&relocatableDataSizeBits, &immediateOffset));

		relocationPos = startPos + immediateOffset;

		//We know the location of a jump symbol relocation
		//uv_assert_ret(m_symbolManager);
		uv_assert_err_ret(m_symbolManager.addAbsoluteLabelRelocationByBits(targetAddress,
				relocationPos, relocatableDataSizeBits));
	}
#endif

	return UV_ERR_OK;
}

uv_err_t UVDDisasmInstruction::parseCurrentInstruction(UVDIteratorCommon &iterCommon)
{
	/*
	Gets the next logical print group
	These all should be associated with a small peice of data, such as a single instruction
	Ex: an address on line above + call count + disassembled instruction
	Note that we can safely assume the current address is valid, but no addresses after it
	*/
	uv_err_t rcTemp = UV_ERR_GENERAL;
	UVDDisasmInstructionShared *inst_shared = NULL;
	uv_addr_t startPosition = 0;
	//UVDData *data = m_uvd->m_data;
	//UVDData *data = m_data;
	UVDDisasmInstructionShared *element = NULL;
	uint8_t opcode = 0;
	UVD *uvd = NULL;
	uv_addr_t absoluteMaxAddress = 0;
	UVDDisasmArchitecture *architecture = NULL;
		
	printf_debug("\n");
	printf_debug("m_nextPosition: 0x%.8X\n", iterCommon.m_nextPosition);
		
	uvd = g_uvd;
	m_uvd = uvd;
	uv_assert_ret(uvd);
	architecture = (UVDDisasmArchitecture *)uvd->m_architecture;
	
	//We are starting a new instruction, reset
	iterCommon.m_currentSize = 0;
	uv_assert_err_ret(g_uvd->m_analyzer->getAddressMax(&absoluteMaxAddress));
	
	//Hmm seems we should never branch here
	//Lets see if we can prove it or at least comment to as why
	//printf("m_nextPosition 0x%04X <= absoluteMaxAddress 0x%04X\n", m_nextPosition, absoluteMaxAddress);
	uv_assert_ret(iterCommon.m_nextPosition <= absoluteMaxAddress);
	/*
	if( m_nextPosition > absoluteMaxAddress )
	{
		*this = m_uvd->end();
		return UV_ERR_DONE;
	}
	*/
	
	//Used to get delta for copying the data we just iterated over
	startPosition = iterCommon.m_nextPosition;

	//We should be garaunteed a valid address at current position by definition
	rcTemp = iterCommon.consumeCurrentExecutableAddress(&opcode);
	uv_assert_err_ret(rcTemp);
	uv_assert_ret(rcTemp != UV_ERR_DONE);
	//uv_assert_err_ret(data->readData(iterCommon.m_nextPosition, (char *)&opcode));	
	printf_debug("Just read (now pos 0x%.8X, size: 0x%02X) 0x%.2X\n", iterCommon.m_nextPosition, iterCommon.m_currentSize, opcode);
	
	/*
	//Go to next position
	uv_err_t rcNextAddress = UV_ERR_GENERAL;
	rcNextAddress = nextValidExecutableAddress();
	uv_assert_err_ret(rcNextAddress);
	This would trucate printing the last instruction
	if( rcNextAddress == UV_ERR_DONE )
	{
		return UV_ERR_DONE;
	}
	*/
	//printf_debug("post nextValidExecutableAddress: start: 0x%.4X, next time: 0x%.4X and is end: %d\n", startPosition, iterCommon.m_nextPosition, m_isEnd);
	uv_assert_ret(iterCommon.m_nextPosition > startPosition/* || m_isEnd*/);
	//If we get UV_ERR_DONE,
	//Of course, if also we require operands, there will be issues
	//But this doesn't mean we can't analyze the current byte
	
	uv_assert_ret(architecture->m_opcodeTable);
	uv_assert_err_ret(architecture->m_opcodeTable->getElement(opcode, &element));

	if( element == NULL )
	{
		if( m_uvd->m_config->m_haltOnTruncatedInstruction )
		{
			printf_debug("Undefined instruction: 0x%.2X\n", opcode);
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		//XXX add something more descriptive
		//uv_assert_err_ret(uvd->addWarning("Undefined instruction"));
		return UV_ERR_OK;
	}
	//XXX: this may change in the future to be less direct
	inst_shared = element;

	m_offset = startPosition;
	m_shared = inst_shared;

	/*
	Setup extension structs
	There should be a perfect matching between each of these and the shared structs
	*/
	/* Since operand and shared operand structs are linked list, we can setup the entire structure by passing in the first elements */
	rcTemp = parseOperands(&iterCommon, getShared()->m_operands, m_operands);
	uv_assert_err_ret(rcTemp);
	//Truncated analysis?
	if( rcTemp == UV_ERR_DONE )
	{
		//Did we request a half or should we comment and continue to best of our abilities?
		if( m_uvd->m_config->m_haltOnTruncatedInstruction )
		{
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		//FIXME: add a more usefule error message with offsets, how many bytes short, etc
		//uv_assert_err_ret(addWarning("Insufficient data to process instruction"));
		return UV_ERR_OK;
	}

	printf_debug("nextPosition, initial: %d, final: %d\n", startPosition, iterCommon.m_nextPosition);

	uv_assert_ret(iterCommon.m_currentSize);
	m_inst_size = iterCommon.m_currentSize;
	//For now these should match
	//XXX However, things like intel opcode extensions bytes will make this check invalid in the future
	if( m_inst_size != getShared()->m_total_length )
	{
		printf_error("Instruction size: %d, shared size: %d\n", m_inst_size, getShared()->m_total_length);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	//We now know the actual size, read the data we just iterated over
	//This will always be valid since we are just storing a shadow copy of the instruction bytes
	//We could alternativly create a UVDData object referring to the source binary file
	//uv_assert_ret(m_data);
	uv_assert_err_ret(g_uvd->m_data->readData(m_offset, m_inst, m_inst_size));

	printf_debug("m_nextPosition about to rollback, initial: %d, final: %d\n", startPosition, iterCommon.m_nextPosition);
	//This is suppose to be parse only, do not actually advance as we use to do
	iterCommon.m_nextPosition = startPosition;

	return UV_ERR_OK;
}	

