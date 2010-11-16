/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_UTIL_CONFIG_SECTION_H
#define UVD_UTIL_CONFIG_SECTION_H

#include "uvd/util/types.h"

class UVDConfigSection;
class UVDConfigLine
{
public:
	UVDConfigLine();
	UVDConfigLine(const std::string &line, UVDConfigSection *section, unsigned int lineNumber);
	~UVDConfigLine();
	
public:
	std::string m_line;
	//Don't own this
	UVDConfigSection *m_section;
	//Actual file line number
	unsigned int m_lineNumber;
	std::string m_key;
	std::string m_value;
};

class UVDSectionConfigFile;
class UVDConfigSection
{
public:
	UVDConfigSection();
	~UVDConfigSection();

public:
	uv_err_t subdivide(const std::string &delim, std::vector<UVDConfigSection> &out);

public:
	//Name of the section
	std::string m_name;
	//Where the section starts
	unsigned int m_lineNumber;
	
	//std::vector<std::string> m_lines;
	//TODO: these are here more of as a placeholder
	//Should replace these vectors with an entry ConfigEntry to track this properly
	//This would allow automatic removal of duplicate entries and maybe even hash map lookups
	//std::vector<uint32_t> m_lineNumbers;
	std::vector<UVDConfigLine> m_lines;
	//Don't own this
	UVDSectionConfigFile *m_file;
};

/*
WARNING: do not do a shallow copy of this
*/
class UVDSectionConfigFile
{
public:
	UVDSectionConfigFile();
	~UVDSectionConfigFile();

	//static uv_err_t readSections(const std::string config_file, std::vector<UVDConfigSection> &sectionsIn);
	//Splits on .SECTIOs type names
	static uv_err_t fromFileNameByDot(const std::string &fileNameIn, UVDSectionConfigFile &out);
	//NAME= type delim
	static uv_err_t fromFileNameByDelim(const std::string &fileNameIn, const std::string &delim,
			UVDSectionConfigFile &out);
	static uv_err_t fromFileNameSingleSection(const std::string &fileNameIn,
			UVDSectionConfigFile &out);

public:
	std::vector<UVDConfigSection> m_sections;
};

#endif

