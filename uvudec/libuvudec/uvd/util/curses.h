/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CURSES_H
#define UVD_CURSES_H

#include <string>

/*
COLOR_BLACK   0
COLOR_RED     1
COLOR_GREEN   2
COLOR_YELLOW  3
COLOR_BLUE    4
COLOR_MAGENTA 5
COLOR_CYAN    6
COLOR_WHITE   7
*/
#define UVD_CURSE_RED		"\x1b[0;31m"
#define UVD_CURSE_GREEN		"\x1b[0;32m"
#define UVD_CURSE_BLUE		"\x1b[0;34m"
#define UVD_CURSE_END		"\x1b[0;39m"

//Take config preferences into account
std::string UVDCurse(const std::string &in, const std::string &color, bool ignore_tty = false, bool ignore_pref = false);
//Force the operation
std::string UVDDoCurse(const std::string &in, const std::string &color);

#endif

