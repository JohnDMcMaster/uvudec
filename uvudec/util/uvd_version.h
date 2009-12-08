/*
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/

#pragma once

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

class UVDSupportedVersion
{
public:
	UVDSupportedVersion();	
	UVDSupportedVersion(UVDVersion min, UVDVersion max);	
	
public:
	UVDVersion m_min;
	UVDVersion m_max;
};

typedef std::vector<UVDSupportedVersion> UVDSupportedVersions;
