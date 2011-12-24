/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/util/curses.h"
#include "uvd/config/config.h"

std::string UVDCurse(const std::string &in, const std::string &color, bool ignore_tty, bool ignore_config) {
	if ((!ignore_tty) && !isatty(fileno(stdout))) {
		return in;
	}
	if( (!ignore_config) && (!g_config || !g_config->m_curse)) {
		return in;
	}
	return UVDDoCurse(in, color);
}

std::string UVDDoCurse(const std::string &in, const std::string &color) {
	return color + in + UVD_CURSE_END;
}


