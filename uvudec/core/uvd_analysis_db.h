/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd.h"

#ifndef UVD_ANALYSIS_DB_H
#define UVD_ANALYSIS_DB_H

#include "uvd_types.h"
#include "uvd_binary_function.h"

/*
Manages data to aid in analysis of binaries
Original purpose was to include pre-decompiled functions for CRT and related analysis
*/
class UVDAnalysisDB
{
public:
	UVDAnalysisDB();
	virtual ~UVDAnalysisDB();
	
	/*
	Possible file inputs:
	-Archive (tar, zip, tar/gzip, etc)
	-Directory
	*/
	virtual uv_err_t loadData(std::string &file) = 0;
	
	/*
	Dump entire database to given location
	Tries to guess what you are trying to do
	-If an archive extension is given, will try to save in that format
	-If no extension is given, will assume its a directory.
		-If it does not exist, it will be created as if with mkdir -p
		-If it does exist, it will assume is destination dir and files should be ovverwritten (ie an update)
	*/
	virtual uv_err_t saveData(std::string &file) = 0;
	
	/*
	Whats with the clear params here?
	Seems they aren't even used
	Given a binary, try to match possible representations
	"Best" (highest level, most expected)
	*/
	uv_err_t queryFunctionByBinary(UVDDataChunk *dataChunk, UVDBinaryFunctionShared **func);
	//All
	virtual uv_err_t queryFunctionByBinary(UVDDataChunk *dataChunk, std::vector<UVDBinaryFunctionShared *> &funcs, bool bClear = false) = 0;

	//Erase the database
	virtual uv_err_t clear();

public:
};

/*
A file format analysis DB
*/
class UVDAnalysisDBArchive : public UVDAnalysisDB
{
public:
	UVDAnalysisDBArchive();
	~UVDAnalysisDBArchive();
	
	/*
	Possible file inputs:
	-Archive (tar, zip, tar/gzip, etc)
	-Directory
	*/
	uv_err_t loadData(std::string &file);
	//Add a single function to the DB
	//This registers functions found during analysis
	uv_err_t loadFunction(UVDBinaryFunctionShared *function);
	
	/*
	Dump entire database to given location
	Tries to guess what you are trying to do
	-If an archive extension is given, will try to save in that format
	-If no extension is given, will assume its a directory.
		-If it does not exist, it will be created as if with mkdir -p
		-If it does exist, it will assume is destination dir and files should be ovverwritten (ie an update)
	*/
	uv_err_t saveData(std::string &file);
	uv_err_t saveFunctionInstanceSharedData(UVDBinaryFunctionShared *function, UVDBinaryFunctionInstance *functionCode, const std::string &outputDir, int functionIndex, std::string &config);
	uv_err_t saveFunctionData(UVDBinaryFunctionShared *function, const std::string &outputDir, std::string &config);
	
	//All
	uv_err_t queryFunctionByBinary(UVDDataChunk *dataChunk, std::vector<UVDBinaryFunctionShared *> &funcs, bool bClear = false);

	virtual uv_err_t clear();

private:
	uv_err_t shouldSaveFunction(UVDBinaryFunctionShared *functionShared);

public:
	//Function database
	std::vector<UVDBinaryFunctionShared *> m_functions;
};

/*
Collection of UVDAnalysisDB
*/
class UVDAnalysisDBConcentrator : public UVDAnalysisDB
{
public:
	UVDAnalysisDBConcentrator();
	~UVDAnalysisDBConcentrator();
	
	uv_err_t init();

	//Do a recursive load on given data
	uv_err_t loadData(std::string &file);
	//Creates a directory with sub dirs/archives
	uv_err_t saveData(std::string &file);
	//All
	uv_err_t queryFunctionByBinary(UVDDataChunk *dataChunk, std::vector<UVDBinaryFunctionShared *> &funcs, bool bClear = false);
	
	//A DB used for the binary we are currently examining
	//FIXME: what if we are examining a multi-part executable?
	//Not an issue for now since embedded are fully static
	uv_err_t getAnalyzedProgramDB(UVDAnalysisDB **db);

	virtual uv_err_t clear();

public:
	std::vector<UVDAnalysisDB *> m_dbs;
};

#endif //UVD_ANALYSIS_DB_H
