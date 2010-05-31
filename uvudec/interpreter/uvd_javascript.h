/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

/*
Several options availible for javascript engine
*/

#include "config.h"

#if defined(USING_SPIDERMONKEY)
#include "uvd_javascript_spidermonkey.h"
#define UVDJavascriptInterpreter UVDJavascriptSpidermonkeyInterpreter
#elif defined(USING_SPIDERAPE)
#include "uvd_javascript_spiderape.h"
#define UVDJavascriptInterpreter UVDJavascriptSpiderapeInterpreter
#else
//Or maybe should just not include something
#error Javascript support requires specifying an engine to use
#endif

