/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_ascii_art.h"

#define UVNET_LOGO_0 \
"           ___________\n" \
"          /           /|\n" \
"         /           /|\n" \
"        /           /|\n" \
"       /   / -     /|\n" \
"      /   \\   \\   /|\n" \
"     /     - /   /|\n" \
"    /           /|\n" \
"   /           /|\n" \
"  /           /|\n" \
" /-----------/|\n" \
" | '         |\n" \
"\n" \
"   U V N E T"

const char *g_uvnet_ascii_art[] = 
{
UVNET_LOGO_0,
NULL
};

std::vector<std::string> getUVNetASCIIArt()
{
	std::vector<std::string> ret;
	
	for( const char **cur = &g_uvnet_ascii_art[0]; cur != NULL; ++cur )
	{
		ret.push_back(*cur);
	}
	
	return ret;
}

std::string getRandomUVNetASCIIArt()
{
	std::vector<std::string> artGallery;
	std::string sRet;
	
	artGallery = getUVNetASCIIArt();
	if( !artGallery.empty() )
	{
		sRet = artGallery[0];
	}
	
	return sRet;
}
