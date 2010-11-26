/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/core/analysis_db.h"
#include "uvd/util/config_section.h"
#include "uvd/util/util.h"
//#include "uvdelf/object.h"
#include <vector>
#include <stdio.h>

UVDAnalysisDB::UVDAnalysisDB(UVDAnalyzer *analyzer)
{
	m_analyzer = analyzer;
}

UVDAnalysisDB::~UVDAnalysisDB()
{
}

uv_err_t UVDAnalysisDB::clear()
{
	//No way to clear without internal knowledge of DB
	return UV_DEBUG(UV_ERR_GENERAL);
}

UVDAnalysisDBArchive::UVDAnalysisDBArchive(UVDAnalyzer *analyzer) : UVDAnalysisDB(analyzer)
{
}

UVDAnalysisDBArchive::~UVDAnalysisDBArchive()
{
}

uv_err_t UVDAnalysisDBArchive::clear()
{
	//Just clear the function list
	m_functions.clear();
	return UV_ERR_OK;
}

uv_err_t UVDAnalysisDB::queryFunctionByBinary(UVDDataChunk *dataChunk, UVDBinaryFunction **func)
{
	std::vector<UVDBinaryFunction *> funcs;
	
	uv_assert_err_ret(queryFunctionByBinary(dataChunk, funcs));
	uv_assert_ret(!funcs.empty());
	
	//Don't be picky for now, get what we can
	//Most should only have one impl registered anyway
	uv_assert_ret(func);
	*func = funcs[0];
	
	return UV_ERR_OK;
}

uv_err_t UVDAnalysisDBArchive::loadData(std::string &file)
{
	//This code isn't being used, its likely to get replaced by JSON config
	return UV_DEBUG(UV_ERR_GENERAL);
#if 0	
	uv_err_t rc = UV_ERR_GENERAL;
	std::vector<std::string> lines;
	UVDSectionConfigFile dbParts;
	std::string fileData;
	
	UV_ENTER();

	printf_debug("Reading DB file...\n");
	uv_assert_err_ret(UVDSectionConfigFile::fromFileNameByDelim(file, "NAME=", dbParts));
	printf_debug("DB parts: %d\n", dbParts.size());
	//Loop for each function
	for( std::vector< std::vector<std::string> >::size_type dbPartsIndex = 0; dbPartsIndex < dbParts.size(); ++dbPartsIndex )
	{
		std::vector<std::string> dbPart = dbParts[dbPartsIndex];
		std::vector<std::string> implLines;

		std::string valueName;
		std::string valueDesc;
		
		UVDBinaryFunction *functionShared = NULL;
	
		//Go until we hit function specific impl
		for( std::vector<std::string>::size_type dbPartIndex = 0; dbPartIndex < dbPart.size(); ++dbPartIndex )
		{
			std::string key;
			std::string value;
			std::string line = dbPart[dbPartIndex];
			uv_err_t rc_temp = UV_ERR_GENERAL;

			printf_debug("Line: <%s>\n", line.c_str());
			rc_temp = uvdParseLine(line, key, value);
			if( UV_FAILED(rc_temp) )
			{
				return UV_ERR(rc);
			}
			else if( rc_temp == UV_ERR_BLANK )
			{
				continue;
			}

			if( key == "NAME" )
			{
				valueName = value;
			}
			else if( key == "DESC" )
			{
				valueDesc = value;
			}
			else if( key == "IMPL" )
			{
				//Transfer remaining to impl vector
				while( dbPartIndex < dbPart.size() )
				{
					implLines.push_back(line);
				}
				break;
			}
			else
			{
				printf_error("Unrecognized function line: %s\n", line.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
		}

		functionShared = new UVDBinaryFunction();
		uv_assert_ret(functionShared);
		
		functionShared->m_name = valueName;
		functionShared->m_description = valueDesc;
		
	
		std::vector< std::vector<std::string> > implParts;
		uv_assert_err_ret(UVDSplitConfigLinesVector(implLines, "BINARY_RAW=", implParts));
		//Loop for each function impl
		for( std::vector< std::vector<std::string> >::size_type implPartsIndex = 0; implPartsIndex < implParts.size(); ++implPartsIndex )
		{
			std::vector<std::string> implPart = implParts[implPartsIndex];

			std::string valueImpl;
			std::string valueSrc;

			for( std::vector<std::string>::size_type implPartIndex = 0; implPartIndex < implPart.size(); ++implPartIndex )
			{
				std::string key;
				std::string value;
				std::string line = implPart[implPartIndex];
				uv_err_t rc_temp = UV_ERR_GENERAL;

				printf_debug("Line: <%s>\n", line.c_str());
				rc_temp = uvdParseLine(line, key, value);
				if( UV_FAILED(rc_temp) )
				{
					return UV_ERR(rc);
				}
				else if( rc_temp == UV_ERR_BLANK )
				{
					continue;
				}

				if( key == "IMPL" )
				{
					valueImpl = value;
				}
				else if( key == "SRC" )
				{
					valueSrc = value;
				}
				else
				{
					printf_error("Unrecognized function implementation line: %s\n", line.c_str());
					return UV_DEBUG(UV_ERR_GENERAL);
				}
			}
			
			UVDBinaryFunctionInstance *codeShared = NULL;
			uv_assert_err_ret(UVDBinaryFunctionInstance::getUVDBinaryFunctionInstance(&codeShared));
			uv_assert_ret(codeShared);
			
			UVDData *data = NULL;
			uv_assert_err_ret(UVDDataFile::getUVDDataFile(&data, valueImpl));
			/*
			//This looks unecessary and would also leak memory
			UVDDataChunk *dataChunk = NULL;
			dataChunk = new UVDDataChunk();
			uv_assert_err_ret(dataChunk->init(data));
			uv_assert_err_ret(codeShared->transferData(dataChunk));
			*/
			uv_assert_err_ret(codeShared->transferData(data));
			
			//Ignore valueSrc for now
			
			
			functionShared->m_representations.push_back(codeShared);
		}
	}

	return UV_ERR_OK;
#endif
}

uv_err_t UVDAnalysisDBArchive::loadFunction(UVDBinaryFunction *function)
{
	uv_assert_ret(function);
	printf_debug("loadFunction: 0x%.8X\n", (unsigned int)function);
	m_functions.push_back(function);
	return UV_ERR_OK;
}

uv_err_t UVDAnalysisDBArchive::saveFunctionInstanceSharedData(
		UVDBinaryFunction *function,
		const std::string &outputDir, int functionIndex, std::string &out)
{
//printf("output dir: %s\n", outputDir.c_str());
	char buff[256];
	UVDData *data = NULL;
	UVDConfig *config = m_analyzer->m_uvd->m_config;
	UVDBinaryFunction *functionInstance = function;
	
	//uv_assert_ret(functionInstance);
	data = functionInstance->getData();
	uv_assert_ret(data);
		
	std::string sOutputFilePrefix;
	std::string symbolName;
	uv_assert_err_ret(functionInstance->getSymbolName(symbolName));
	//snprintf(buff, sizeof(buff), "%s/%s", outputDir.c_str(), symbolName.c_str());
	sOutputFilePrefix = symbolName;
	
	printf_debug_level(UVD_DEBUG_SUMMARY, "Saving function analysis data: %s\n", symbolName.c_str());
	
	if( config->m_writeRawBinary )
	{
		//Raw binary
		std::string binRawFile;
		snprintf(buff, sizeof(buff), "%s/%s%s", outputDir.c_str(), sOutputFilePrefix.c_str(), config->m_rawFileSuffix.c_str());
		binRawFile = buff;
		printf_debug_level(UVD_DEBUG_VERBOSE, "Writting raw binary to file: %s\n", binRawFile.c_str());
		out += "BINARY_RAW=" + binRawFile + "\n";
		uv_assert_err_ret(data->saveToFile(binRawFile));
	}

#if RELOCATABLE_WRITE_BINRARIES
	if( config->m_writeRelocatableBinary )
	{
		//Relocatable fixed binary
		std::string binRelocatableFile;
		snprintf(buff, sizeof(buff), "%s%s", sOutputFilePrefix.c_str(), config->m_relocatableFileSuffix.c_str());
		printf_debug_level(UVD_DEBUG_VERBOSE, "Writting relocatable binary to file: %s\n", binRelocatableFile.c_str());
		binRelocatableFile = buff;
		out += "BINARY_RELOCATABLE=" + binRelocatableFile + "\n";
		uv_assert_err_ret(data->saveToFile(outputDir + "/" + binRelocatableFile));
	
		//TODO: add config for relocations
		std::string binRelocatableConfigFile;
		snprintf(buff, 256, "%s/%s_%d_relocatable.cfg", outputDir.c_str(), symbolName.c_str());
		binRelocatableConfigFile = buff;
		out += "BINARY_RELOCATABLE_CONFIG=" + binRelocatableConfigFile + "\n";
		//Get relocations here and save file...
	}
#endif

	//Code is optional, sometimes we just have binary and know its, say, printf
#if 0
	if( !functionInstance->m_code.empty() )
	{
		std::string srcFile;
		
		snprintf(buff, 256, "%s/%s_%d.c", outputDir.c_str(), sOutputFilePrefix.c_str(), functionIndex);
		srcFile = buff;
		
		out += "SRC=" + srcFile + "\n";
		uv_assert_err_ret(writeFile(srcFile, functionInstance->m_code));
	}
#endif

	return UV_ERR_OK;
}

uv_err_t UVDAnalysisDBArchive::saveFunctionData(UVDBinaryFunction *function, const std::string &outputDir, std::string &config)
{
	std::string name;
	std::string description;
	
	uv_assert_ret(function);
		
	uv_assert_err_ret(function->getSymbolName(name));
	//Should have auto named even if real is unknown
	uv_assert_ret(!name.empty());
	//Reminder: this can be empty as it is defined as the real function name or empty if unknown
	config += "NAME=" + name + "\n";
	config += "DESC=" + description + "\n";
		
	uv_assert_err_ret(saveFunctionInstanceSharedData(function, outputDir, 0, config));
	//config += "VER_MIN=" + 1 + "\n";
	//config += "VER_MAX=" + 2 + "\n";
	config += "\n";

	return UV_ERR_OK;
}

uv_err_t UVDAnalysisDBArchive::shouldSaveFunction(UVDBinaryFunction *functionShared)
{
	UVDConfig *config = m_analyzer->m_uvd->m_config;
	UVDBinaryFunction *function = functionShared;
	uint32_t iFunctionAddress = 0;
		
	uv_assert_ret(config);	
	if( config->m_analysisOutputAddresses.empty() )
	{
		return UV_ERR_OK;
	}
	
	uv_assert_ret(function);
	iFunctionAddress = function->m_offset;

	if( config->m_analysisOutputAddresses.find(iFunctionAddress) != config->m_analysisOutputAddresses.end() )
	{
		return UV_ERR_OK;
	}
	else
	{
		return UV_ERR_GENERAL;
	}
}

uv_err_t UVDAnalysisDBArchive::saveData(const std::string &outputDbFile)
{
	std::string outputDir;
	UVDConfig *uvdConfig = m_analyzer->m_uvd->m_config;
	
	printf_analysis_debug("saving data\n");
	
	//Assume directory
	if( UV_FAILED(isDir(outputDbFile)) )
	{
		uv_assert_err_ret(createDir(outputDbFile, false));
	}
	//May be temp dir later to create archive
	outputDir = outputDbFile;
	
	//Do one conglamerate file for now, but allow breaking up if required
	std::string configFile = outputDbFile + "/" + uvdConfig->m_functionIndexFilename;
	std::string config;
	
	printf_debug_level(UVD_DEBUG_SUMMARY, "Iterating over functions: %d\n", m_functions.size());
	printf_analysis_debug("Iterating over functions: %d\n", m_functions.size());
	
	//Loop for each function
	for( std::vector<UVDBinaryFunction *>::size_type i = 0; i < m_functions.size(); ++i )
	{
		UVDBinaryFunction *function = m_functions[i];
		
		uv_assert_ret(function);
		
		if( UV_FAILED(shouldSaveFunction(function)) )
		{
			continue;
		}

		printf_debug("m_functions[%d] = %0x%.8X\n", i, (unsigned int)function);
		uv_assert_err_ret(saveFunctionData(function, outputDir, config));

		if( i < m_functions.size() )
		{
			config += "\n";
			config += "\n";
		}
	}	

	uv_assert_err_ret(writeFile(configFile, config));	

	return UV_ERR_OK;
}

uv_err_t UVDAnalysisDBArchive::queryFunctionByBinary(UVDDataChunk *dataChunk, std::vector<UVDBinaryFunction *> &funcs, bool bClear)
{
	if( bClear )
	{
		funcs.clear();
	}
	for( std::vector<UVDBinaryFunction *>::size_type i = 0; i < m_functions.size(); ++i )
	{
		UVDBinaryFunction *function = m_functions[i];
		
		uv_assert_ret(function);
		
		UVDBinaryFunction *functionShared = function;
		uv_assert_ret(functionShared);
		
		//Match?
		if( dataChunk == functionShared->getData() )
		{
			//Yipee!
			funcs.push_back(function);
			//We already have a match for this function
			break;
		}
	}
	
	return UV_ERR_OK;
}

