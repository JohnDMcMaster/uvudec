/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasciiart/ascii_art.h"
#include "uvdasciiart/plugin.h"
#include <boost/filesystem.hpp>
#include <stdlib.h>

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

/*
...........,:Z8DDDD888?.............
.........88O$$$$$$$$$$$Z8D:.........
.......8D$$$$$$$$$$$$$$$$$O8,.......
.... +8Z$$$$$$$$$$$$$$$$$$$$OD......
..,.=D$$$$$$$$$$$$$$$$$$$$$$$Z8.....
...ZD$$$$$$$$$$$$$$$$$$$$$$$$$Z8....
...D$$$$$$$$$$$$$$$$$$$$$$$$$$$OI...
..8$$$$$$$$$$$8Z...,D8$$$$$$$$$$8...
..D$$$$$$$$$88.......~D$$$$$$$$$8O..
.:D$$$$$$$$$D.........ZZ$$$$$$$$ZD..
.=D$$$$$$$$$8.........+O$$$$$$$$ZD..
..D$$$$$$$$$D:........8$$$$$$$$$O8..
..D$$$$$$$$$$D+......DZ$$$$$$$$$8~..
..IO$$$$$$$$$$ZD,,.ND$$$$$$$$$$$D...
...8Z$$$$$$$$$Z8....D$$$$$$$$$$D,...
..  87$$$$$$$$8.....8$$$$$$$$ZD+....
....,8O$$$$$$Z7......D$$$$$$$D:.....
......=8$$$$$8.......OD$$$$8D.......
........OD7$DI........8$$88,...(TM).
...........88.........,D?...........
.......Rensselaer Center For........
........Open Source Software........
*/
#define RCOS_LOGO_0 \
"...........,:Z8DDDD888?.............\n" \
".........88O$$$$$$$$$$$Z8D:.........\n" \
".......8D$$$$$$$$$$$$$$$$$O8,.......\n" \
".... +8Z$$$$$$$$$$$$$$$$$$$$OD......\n" \
"..,.=D$$$$$$$$$$$$$$$$$$$$$$$Z8.....\n" \
"...ZD$$$$$$$$$$$$$$$$$$$$$$$$$Z8....\n" \
"...D$$$$$$$$$$$$$$$$$$$$$$$$$$$OI...\n" \
"..8$$$$$$$$$$$8Z...,D8$$$$$$$$$$8...\n" \
"..D$$$$$$$$$88.......~D$$$$$$$$$8O..\n" \
".:D$$$$$$$$$D.........ZZ$$$$$$$$ZD..\n" \
".=D$$$$$$$$$8.........+O$$$$$$$$ZD..\n" \
"..D$$$$$$$$$D:........8$$$$$$$$$O8..\n" \
"..D$$$$$$$$$$D+......DZ$$$$$$$$$8~..\n" \
"..IO$$$$$$$$$$ZD,,.ND$$$$$$$$$$$D...\n" \
"...8Z$$$$$$$$$Z8....D$$$$$$$$$$D,...\n" \
"..  87$$$$$$$$8.....8$$$$$$$$ZD+....\n" \
"....,8O$$$$$$Z7......D$$$$$$$D:.....\n" \
"......=8$$$$$8.......OD$$$$8D.......\n" \
"........OD7$DI........8$$88,...(TM).\n" \
"...........88.........,D?...........\n" \
".......Rensselaer Center for........\n" \
"........Open Source Software........\n"
                         
const char *g_uvnet_ascii_art[] = 
{
UVNET_LOGO_0,
RCOS_LOGO_0,
NULL
};

uv_err_t getUVNetASCIIArt(std::vector<std::string> &out)
{
	std::string dataDir;
	
	out.clear();

	for( const char **cur = &g_uvnet_ascii_art[0]; *cur != NULL; ++cur )
	{
		out.push_back(*cur);
	}
	
	uv_assert_err_ret(g_asciiArtPlugin->getDataDir(dataDir));
	//Load from files
	for( boost::filesystem::directory_iterator iter(dataDir);
			iter != boost::filesystem::directory_iterator(); ++iter )
	{
		std::string file;
		std::string fileContents;
		
		file = dataDir + "/" + iter->path().filename();
		uv_assert_err_ret(UVDReadFileByString(file, fileContents));
		
		out.push_back(fileContents);
	}
		
	return UV_ERR_OK;
}

uv_err_t getRandomUVNetASCIIArt(std::string &out)
{
	std::vector<std::string> artGallery;
	
	uv_assert_err_ret(getUVNetASCIIArt(artGallery));
	if( !artGallery.empty() )
	{
		srand(time(NULL));
		out = artGallery[rand() % artGallery.size()];
	}
	else
	{
		out = "";
	}
	
	return UV_ERR_OK;
}

