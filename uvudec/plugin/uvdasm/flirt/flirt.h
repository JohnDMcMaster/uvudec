/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_UVD_H
#define UVD_FLIRT_PATTERN_UVD_H

#ifdef UVD_FLIRT_PATTERN_UVD

#include "flirt/pat/pat.h"

/*
A single function we have found
*/
class UVDLibrary;
class UVDFLIRTPatternAnalysis;
class UVDFLIRTPatternEntry
{
public:
	UVDFLIRTPatternEntry();
	~UVDFLIRTPatternEntry();
	uv_err_t deinit();

public:
	//The entire function data, mapped to our UVDLibrary's m_data
	//We own this
	UVDData *m_data;
	//The analysis object we belong to
	UVDFLIRTPatternAnalysis *m_analysis;
	/*
	We abstract the info into a FLIRT entry, but its essentially a symbol with relocs and such
	Symbol usage locations: addresses are in the analyzed library
	m_symbol->m_relocatableData contains all of the relocations in this entry
	*/
	UVDBinarySymbol *m_symbol;
};

/*
Using libuvudec to get symbols and relocations
*/
class UVDFLIRTPatternGeneratorUVD
{
public:
	UVDFLIRTPatternGeneratorUVD();
	~UVDFLIRTPatternGeneratorUVD();
	uv_err_t deinit();
	
	/*
	Save .pat to given file
	*/
	uv_err_t saveToFile(const std::string &file);
	//Core version providing a text dump of the .pat file
	uv_err_t saveToString(std::string &output);
	
	virtual uv_err_t canGenerate(const std::string &file);
	
	static uv_err_t getPatternGenerator(UVDFLIRTPatternGeneratorBFD **generatorOut);
	
private:
	uv_err_t getPatLeadingSignature(UVDFLIRTPatternEntry *entry, std::string &out);
	uv_err_t getPatCRC(UVDFLIRTPatternEntry *entry, std::string &out);
	uv_err_t getPatPublicNames(UVDFLIRTPatternEntry *entry, std::string &out);
	uv_err_t getPatReferencedNames(UVDFLIRTPatternEntry *entry, std::string &out);
	uv_err_t getPatTailBytes(UVDFLIRTPatternEntry *entry, std::string &out);
	
public:
	//List of functions, we own this
	std::vector<UVDFLIRTPatternEntry *> m_entries;
	//Library that generated the entries, we do not own this
	UVDLibrary *m_library;
};

#endif

#endif
