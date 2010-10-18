/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_VERSION_H
#define UVD_VERSION_H

#include <string>
#include <vector>

class UVDVersion
{
public:
	UVDVersion();
	UVDVersion(std::string version);
	
	//Deliminate by dots
	std::vector<int> intVec();
	std::vector<std::string> strVec();

	//Scalar magnitude does not matter
	bool compare(UVDVersion Other);
	bool operator<(UVDVersion other);
	bool operator<=(UVDVersion other);
	bool operator==(UVDVersion other);
	bool operator>(UVDVersion other);
	bool operator>=(UVDVersion other);
	
public:
	std::string m_version;
};

class UVDVersionRange
{
public:
	UVDVersionRange();
	UVDVersionRange(UVDVersion exact);	
	UVDVersionRange(UVDVersion min, UVDVersion max);	
	
	bool matches(UVDVersion &given);
	
public:
	//Inclusive
	UVDVersion m_min;
	//Exclusive, unless specified in min
	UVDVersion m_max;
};

/*
To verify we have linked to a good library version if we are using dlsym() style loading
*/
const char *UVDGetVersion();

typedef std::vector<UVDVersionRange> UVDSupportedVersions;

#endif

