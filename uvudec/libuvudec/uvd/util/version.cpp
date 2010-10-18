/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/core/uvd.h"
#include "uvd/util/util.h"
#include "uvd/util/version.h"
#include <vector>

UVDVersion::UVDVersion()
{
}

UVDVersion::UVDVersion(std::string version)
{
	m_version = version;
}
	
std::vector<int> UVDVersion::intVec()
{
	std::vector<std::string> strs = strVec();
	std::vector<int> ret;

	for( std::vector<std::string>::size_type i = 0; i < strs.size(); ++i )
	{
		ret.push_back(atoi(strs[i].c_str()));
	}
	return ret;
}

std::vector<std::string> UVDVersion::strVec()
{
	return split(m_version, '.', true);
}

bool UVDVersion::compare(UVDVersion other)
{
	/*
	This does not currently consider lettered versions
	Rare and not important for now
	*/
	
	std::vector<int> partsThis = intVec();
	std::vector<int> partsOther = other.intVec();
	
	for( std::vector<int>::size_type i = 0; i < partsThis.size() && i < partsOther.size(); ++i )
	{
		int partThis = partsThis[i];
		int partOther = partsOther[i];
		int diffLast = partThis - partOther;
		
		if( diffLast != 0 )
		{
			return diffLast;
		}
	}
	//All matched segments equal length
	
	//Extra version parts?
	if( partsThis.size() > partsOther.size() )
	{
		return 1;
	}
	else if( partsThis.size() < partsOther.size() )
	{
		return -1;
	}
	//Completly equal
	else
	{
		return 0;
	}
}

bool UVDVersion::operator<(UVDVersion other)
{
	return compare(other) < 0;
}

bool UVDVersion::operator<=(UVDVersion other)
{
	return compare(other) <= 0;
}

bool UVDVersion::operator==(UVDVersion other)
{
	return compare(other) == 0;
}

bool UVDVersion::operator>(UVDVersion other)
{
	return compare(other) > 0;
}

bool UVDVersion::operator>=(UVDVersion other)
{
	return compare(other) >= 0;
}

UVDVersionRange::UVDVersionRange()
{
}

UVDVersionRange::UVDVersionRange(UVDVersion exact)
{
	m_min = exact;
	m_max = exact;
}

UVDVersionRange::UVDVersionRange(UVDVersion min, UVDVersion max)
{
	m_min = min;
	m_max = max;
}

bool UVDVersionRange::matches(UVDVersion &given)
{
	return given >= m_min && given <= m_max;
}

const char *UVDGetVersion()
{
	return UVUDEC_VER_STRING;
}

