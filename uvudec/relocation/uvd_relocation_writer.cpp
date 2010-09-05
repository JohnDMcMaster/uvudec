/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_relocation_writer.h"

UVDRelocationWriter::UVDRelocationWriter()
{
}

UVDRelocationWriter::~UVDRelocationWriter()
{
}

uv_err_t UVDRelocationWriter::constructBinary(UVDData **dataOut)
{
	uv_assert_ret(dataOut);
	
	//Phase 1: construct data peices to be assembled
	//These should have as much information filled in as possible with the exception of fields depending on locations in the file
	//printf_debug("\n***\nupdateForWrite()\n");
	uv_assert_err_ret(updateForWrite());

	//Phase 2: place all of the data
	//Each section should have all of the information it needs from the previous phase to be sized and placed
	//It is reccomended to construct the data here (as opposed to phase 1) as updateForWrite() call order is not
	//gauranteed and may not have all updates until end
	//getFileRelocatableData() should be ready to be called
	//printf_debug("\n***\nconstruct()\n");
	uv_assert_err_ret(construct());
	//ELF_WRITER_DEBUG(hexdump());

	//Phase 3: now that we have placed all of the data, fixup references to the final addresses
	//printf_debug("\n***\napplyRelocations()\n");
	uv_assert_err_ret(applyRelocations());
	//ELF_WRITER_DEBUG(hexdump());
	
	//Compute the final object
	//printf_debug("\n***\nApplying relocations\n");
	if( UV_FAILED(m_relocationManager.applyPatch(dataOut)) )
	{
		printf_error("Could not apply relocations\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_ret(*dataOut);

	return UV_ERR_OK;
}

uv_err_t UVDRelocationWriter::addRelocatableData(char *dataIn, uint32_t dataInSize, bool shouldFreeData)
{
	//Header decls
	UVDDataMemory *data = NULL;
	UVDRelocatableData *relocatable = NULL;

	//This must be the actual data to get relocations correct,
	//and is more efficient
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer(&data,
			dataIn, dataInSize, shouldFreeData));
	uv_assert_ret(data);
	relocatable = new UVDRelocatableData();
	uv_assert_ret(relocatable);
	uv_assert_err_ret(relocatable->transferData(data, true));
	m_relocationManager.addRelocatableData(relocatable);

	return UV_ERR_OK;
}

uv_err_t UVDRelocationWriter::addRelocatableData(UVDData *data, bool shouldFreeData)
{
	UVDRelocatableData *relocatable = NULL;

	uv_assert_ret(data);
	relocatable = new UVDRelocatableData();
	uv_assert_ret(relocatable);
	uv_assert_err_ret(relocatable->transferData(data, shouldFreeData));
	m_relocationManager.addRelocatableData(relocatable);

	return UV_ERR_OK;
}

