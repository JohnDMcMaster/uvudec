/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/util/config_section.h"

/*
UVDConfigLine
*/

UVDConfigLine::UVDConfigLine()
{
	m_lineNumber = 0;
}

UVDConfigLine::UVDConfigLine(const std::string &line, UVDConfigSection *section, unsigned int lineNumber)
{
	m_line = line;
	m_section = section;
	m_lineNumber = lineNumber;
}

UVDConfigLine::~UVDConfigLine()
{
}

/*
UVDConfigSection
*/

UVDConfigSection::UVDConfigSection()
{
	m_lineNumber = 0;
	m_file = NULL;
}

UVDConfigSection::~UVDConfigSection()
{
}

/*
For splitting up config sections
Sections must start with delim and output will include it
*/
//uv_err_t UVDSplitConfigLinesVector(const std::vector<std::string> &in, const std::string &delim, std::vector< std::vector<std::string> > &out)
uv_err_t UVDConfigSection::subdivide(const std::string &delim, std::vector<UVDConfigSection> &out)
{	
	UVDConfigSection next;

	out.clear();
	next.m_file = m_file;
	next.m_lineNumber = m_lineNumber;
	
	for( std::vector<UVDConfigLine>::size_type i = 0; i < m_lines.size(); ++i )
	{
		UVDConfigLine curLine = m_lines[i];
		std::string partRaw = curLine.m_line;
		std::string part;
		
		// FIXME: this should be preprocessed out before this
		uv_assert_err_ret(uvdPreprocessLine(partRaw, part));
		if( part.empty() )
		{
			continue;
		}

		//Did we find delim?
		if( part.find(delim) == 0 )
		{
			//Add last entry, if present
			if( !next.m_lines.empty() )
			{
				out.push_back(next);
				next.m_lines.clear();
			}
		}
		else
		{
			//We should have an entry on first usable line, early error indicator
			if( next.m_lines.empty() )
			{
				printf_debug("Got: %s, expected: %s\n", part.c_str(), delim.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
		}
		
		//Build current item
		next.m_lines.push_back(curLine);
	}
	
	//Add final entry, if present (none if blank input)
	if( !next.m_lines.empty() )
	{
		out.push_back(next);
	}
	
	return UV_ERR_OK;
}

/*
UVDSectionConfigFile
*/

UVDSectionConfigFile::UVDSectionConfigFile()
{
}

UVDSectionConfigFile::~UVDSectionConfigFile()
{
}

uv_err_t UVDSectionConfigFile::fromFileNameByDot(const std::string &configFileName, UVDSectionConfigFile &sectionsOut)
{
	std::string configFileData;
	std::vector<std::string> lines;
	unsigned int start_index = 0;
	
	printf_debug("Reading file...\n");
	uv_assert_err_ret(UVDReadFileByString(configFileName, configFileData));
	
	//Find out how many sections we got
	lines = UVDSplitLines(configFileData);
	sectionsOut.m_sections.clear();
		
	for( std::vector<std::string>::size_type i = 0; i < lines.size(); ++i )
	{
		//Trigger on falling edges of sections
		if( lines[i][0] == '.' || i == lines.size() - 1 )
		{
			UVDConfigSection cur_section;
			unsigned int nLines = 0;

			//Initialize where the section starts
			if( start_index == 0 )
			{
				start_index = i;
				continue;
			}

			cur_section.m_file = &sectionsOut;
			cur_section.m_lineNumber = start_index;
			
			//Skip the .
			cur_section.m_name = lines[start_index].c_str() + 1;
			printf_debug("Reading section: %s\n", cur_section.m_name.c_str());
			printf_debug("Start: %d, end: %d\n", start_index, i);
			++start_index;
			
			//Copy lines
			nLines = (unsigned int)(i - start_index);
			printf_debug("Copying lines: %d\n", nLines);
			for( unsigned int j = 0; j < nLines; ++j )
			//while( start_index < i )
			{
				cur_section.m_lines.push_back(UVDConfigLine(lines[start_index], &cur_section, start_index));
				++start_index;
			}
			start_index = i;
			sectionsOut.m_sections.push_back(cur_section);
			printf_debug("Section read\n");
		}
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDSectionConfigFile::fromFileNameByDelim(const std::string &fileNameIn, const std::string &delim,
		UVDSectionConfigFile &out)
{
	std::vector<UVDConfigSection> sections;
	
	//Start out with a single section
	uv_assert_err_ret(UVDSectionConfigFile::fromFileNameSingleSection(fileNameIn, out));
	//Get a divided copy
	uv_assert_ret(out.m_sections.size() == 1);
	uv_assert_err_ret(out.m_sections[0].subdivide(delim, sections));
	//Replace the single section with the divided list
	out.m_sections = sections;
	
	return UV_ERR_OK;
}

uv_err_t UVDSectionConfigFile::fromFileNameSingleSection(const std::string &fileNameIn,
		UVDSectionConfigFile &out)
{
	UVDConfigSection section;
	std::vector<std::string> lines;
	std::string fileData;

	uv_assert_err_ret(readFile(fileNameIn, fileData));
	lines = split(fileData, '\n', false);

	section.m_file = &out;
	section.m_lineNumber = 0;
	
	out.m_sections.clear();
	for( unsigned int j = 0; j < lines.size(); ++j )
	{
		section.m_lines.push_back(UVDConfigLine(lines[j], &section, j));
	}
	out.m_sections.push_back(section);

	return UV_ERR_OK;
}

