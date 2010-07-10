/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd.h"
#include "uvd_binary_function.h"
#include "uvd_elf.h"
#include "uvd_elf_relocation.h"
#include "uvd_language.h"
#include "uvd_md5.h"
#include "uvd_util.h"

/*
UVDBinaryFunctionInstance
*/
UVDBinaryFunctionInstance::UVDBinaryFunctionInstance()
{
	m_compiler = NULL;
	m_compilerOptions = NULL;
	m_relocatableData = NULL;

	m_language = UVD_LANGUAGE_UNKNOWN;
}

UVDBinaryFunctionInstance::~UVDBinaryFunctionInstance()
{
	deinit();
}

uv_err_t UVDBinaryFunctionInstance::init()
{
	uv_assert_err_ret(UVDBinarySymbol::init());
	
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::getUVDBinaryFunctionInstance(UVDBinaryFunctionInstance **out)
{
	UVDBinaryFunctionInstance *instance = NULL;
	
	instance = new UVDBinaryFunctionInstance();
	uv_assert_ret(instance);
	
	uv_assert_err_ret(instance->init());

	uv_assert_ret(out);
	*out = instance;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::setData(UVDData *data)
{
	delete m_data;
	uv_assert_ret(data);
	uv_assert_err_ret(data->deepCopy(&m_data));

	if( m_relocatableData )
	{
		//Does deep copy
		uv_assert_err_ret(m_relocatableData->setData(data));
	}

	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::transferData(UVDData *data)
{
	delete m_data;
	m_data = data;

	if( m_relocatableData )
	{
		//Does deep copy
		uv_assert_err_ret(m_relocatableData->setData(data));
	}

	return UV_ERR_OK;
}

UVDData *UVDBinaryFunctionInstance::getData()
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

uv_err_t UVDBinaryFunctionInstance::toUVDElf(UVDElf **out)
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

		sourceFilename = g_uvd->getData()->getSource();
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

uv_err_t UVDBinaryFunctionInstance::getFromUVDElf(const UVDElf *in, UVDBinaryFunctionInstance **out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDBinaryFunctionInstance::getHash(std::string &hash)
{
	if( m_MD5.empty() )
	{
		uv_assert_err_ret(computeHash());
	}
	hash = m_MD5;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::getRelocatableHash(std::string &hash)
{
	if( m_relocatableMD5.empty() )
	{
		uv_assert_err_ret(computeRelocatableHash());
	}
	hash = m_relocatableMD5;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::getRawDataBinary(UVDData **dataRet)
{
	//This data is suppose to be inherent
	//Simply see if it looks halfway reasonable and return it
	uv_assert_ret(m_data);
	uv_assert_ret(dataRet);
	*dataRet = m_data;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::getRelocatableDataBinary(UVDData **dataRet)
{
	UVDData *data = NULL;

	uv_assert_ret(m_relocatableData);
	uv_assert_err_ret(m_relocatableData->getDefaultRelocatableData(&data));
	uv_assert_ret(dataRet);
	*dataRet = data;
	
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::computeHash()
{
	char *buff = NULL;
	uint32_t buffSize = 0;

	//Fetch our data
	uv_assert_ret(m_data);
	uv_assert_err_ret(m_data->readData(&buff));
	buffSize = m_data->size();

	//And compute an MD5
	uv_assert_err_ret(uv_md5(buff, buffSize, m_MD5));
	uv_assert_ret(!m_MD5.empty());
	free(buff);
	
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::computeRelocatableHash()
{
	char *buff = NULL;
	uint32_t buffSize = 0;
	UVDData *data = NULL;

	//Fetch our data
	uv_assert_err_ret(getRelocatableDataBinary(&data));
	uv_assert_ret(data);
	uv_assert_err_ret(data->readData(&buff));
	uv_assert_ret(buff);
	buffSize = m_data->size();

	//And compute an MD5
	uv_assert_err_ret(uv_md5(buff, buffSize, m_relocatableMD5));
	uv_assert_ret(!m_relocatableMD5.empty());
	//Finished, free buffer
	free(buff);
	
	return UV_ERR_OK;
}

/*
UVDBinaryFunctionShared
*/

UVDBinaryFunctionShared::UVDBinaryFunctionShared()
{
}

UVDBinaryFunctionShared::~UVDBinaryFunctionShared()
{
	deinit();
}

uv_err_t UVDBinaryFunctionShared::deinit()
{
	for( std::vector<UVDBinaryFunctionInstance *>::iterator iter = m_representations.begin(); iter != m_representations.end(); ++iter )
	{
		delete *iter;
	}
	m_representations.clear();
	
	return UV_ERR_OK;
}

/*
UVDBinaryFunction
*/

UVDBinaryFunction::UVDBinaryFunction()
{
	m_data = NULL;
	m_uvd = NULL;
	m_shared = NULL;
	m_instance = NULL;
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

UVDBinaryFunctionInstance *UVDBinaryFunction::getFunctionInstance()
{
	//Try to find the instance if it wasn't already cached
	if( !m_instance )
	{
		if( !m_shared )
		{
			return NULL;
		}
		if( m_shared->m_representations.size() != 1 )
		{
			return NULL;
		}
		m_instance = m_shared->m_representations[0];
	}
	return m_instance;
}

void UVDBinaryFunction::setFunctionInstance(UVDBinaryFunctionInstance *instance)
{
	m_instance = instance;
}


