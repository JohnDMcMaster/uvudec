/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/core/uvd.h"
#include "uvd/assembly/function.h"
#include "uvd/elf/elf.h"
#include "uvd/elf/relocation.h"
#include "uvd/language/language.h"
#include "uvd/hash/md5.h"
#include "uvd/util/util.h"
#include "uvd/core/runtime.h"

uv_err_t UVDBinaryFunction::setData(UVDData *data)
{
	delete m_data;
	uv_assert_ret(data);
	uv_assert_err_ret(data->deepCopy(&m_data));

	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::transferData(UVDData *data)
{
	delete m_data;
	m_data = data;

	return UV_ERR_OK;
}

UVDData *UVDBinaryFunction::getData()
{
	return m_data;
}

/*
During the symbol mapping process, the symbol names remain the same, they are the PK link on both sides
*/

//
static uv_err_t relocationFixupToElfRelocationFixup(UVDElf *elf, UVDRelocationFixup *analysisRelocation)
{
	//analysisRelocation param forms analysis place
	//Offsets are assumed for now to be for the single symbol we have in the file that these apply to
	//Analysis fixup value
	UVDRelocatableElement *analysisSymbol = NULL;
	//ELF fixup value
	UVDElfSymbol *elfSymbol = NULL;
	//ELF place to apply fixup
	UVDElfRelocation *elfRelocation = NULL;

	std::string symbolName;


	//Setup related
	
	uv_assert_ret(elf);
	uv_assert_ret(analysisRelocation);
	
	//Analysis symbol
	analysisSymbol = analysisRelocation->m_symbol;
	uv_assert_ret(analysisSymbol);

	//Symbol name
	uv_assert_err_ret(analysisSymbol->getName(symbolName));
	uv_assert_ret(!symbolName.empty());
	
	//ELF symbol
	uv_assert_err_ret(elf->getFunctionSymbol(symbolName, &elfSymbol));
	uv_assert_ret(elfSymbol);

	//ELF relocation
	uv_assert_err_ret(elfSymbol->getRelocation(&elfRelocation));
	uv_assert_ret(elfRelocation);

	//Transfer the relocation
	//Assume are relative to the symbol (assumed function) for now
	
	//Size
	uv_assert_err_ret(elfRelocation->updateRelocationTypeByBits(analysisRelocation->getSizeBits()));
	//Where
	//elfRelocation->setSectionOffset(analysisRelocation->m_offset);
	elfRelocation->m_offset = analysisRelocation->m_offset;
	//Note value is the external symbol value

	
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::toUVDElf(UVDElf **out)
{
	UVDElf *elf = NULL;
	std::string symbolName;
	
	uv_assert_ret(m_relocatableData);
	//uv_assert_ret(m_relocatableData->m_data);
	//Get a base representation
	//printf("symbol: %s, symbol relocations: %d\n", name.c_str(), m_relocatableData->m_fixups.size()); 
	uv_assert_err_ret(getSymbolName(symbolName));
	uv_assert_ret(!symbolName.empty());
	uv_assert_err_ret(UVDElf::getFromRelocatableData(m_relocatableData, symbolName, &elf));

	/*
	Add in the relocations
	For each relocation, register the symbol if it doesn't exist
	Then, add it as an instance of relocation for that symbol
	*/
	uv_assert_ret(m_relocatableData);
	for( std::set<UVDRelocationFixup *>::iterator iter = m_relocatableData->m_fixups.begin();
			iter != m_relocatableData->m_fixups.end(); ++iter )
	{
		UVDRelocationFixup *fixup = *iter;
		
		uv_assert_ret(fixup);
		uv_assert_ret(fixup->m_symbol);
		/*
		{
			std::string name;
			uv_assert_err_ret(fixup->m_symbol->getName(name));
			printf("Fixup name: %s\n", name.c_str());
			uv_assert_ret(!name.empty());
		}
		*/
		uv_assert_err_ret(relocationFixupToElfRelocationFixup(elf, fixup));
	}
	
	{
		std::string sourceFilename;
		std::string mangledSourceFilename;

		sourceFilename = g_uvd->m_runtime->m_object->m_data->getSource();
		uv_assert_ret(!sourceFilename.empty());
		//TODO: add option here for absolute path save
		mangledSourceFilename = uv_basename(sourceFilename);
		uv_assert_ret(!mangledSourceFilename.empty());
		uv_assert_err_ret(elf->setSourceFilename(mangledSourceFilename));
	}

	uv_assert_ret(out);
	*out = elf;
	
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::getFromUVDElf(const UVDElf *in, UVDBinaryFunction **out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
UVDBinaryFunction
*/

UVDBinaryFunction::UVDBinaryFunction()
{
	m_data = NULL;
	m_uvd = NULL;
	m_offset = 0;
}

UVDBinaryFunction::~UVDBinaryFunction()
{
	deinit();
}

uv_err_t UVDBinaryFunction::deinit()
{
	delete m_data;
	m_data = NULL;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::getMin(uint32_t *out)
{
	uv_assert_ret(out);
	*out = m_offset;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::getMax(uint32_t *out)
{
	uv_assert_ret(m_data);
	uv_assert_ret(out);
	uv_assert_ret(m_data->size(out));
	*out += m_offset;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::getUVDBinaryFunctionInstance(UVDBinaryFunction **out)
{
	UVDBinaryFunction *instance = NULL;
	
	instance = new UVDBinaryFunction();
	uv_assert_ret(instance);
	
	uv_assert_err_ret(instance->init());

	uv_assert_ret(out);
	*out = instance;
	return UV_ERR_OK;
}

