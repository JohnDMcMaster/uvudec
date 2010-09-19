#include "uvd.h"
#include "uvd_util.h"
#include "uvd_version.h"
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

UVDSupportedVersion::UVDSupportedVersion()
{
}

UVDSupportedVersion::UVDSupportedVersion(UVDVersion min, UVDVersion max)
{
	m_min = min;
	m_max = max;
}

const char *UVDGetVersion()
{
	return UVUDEC_VER_STRING;
}
