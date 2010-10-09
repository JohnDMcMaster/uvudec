/*
Several options availible for javascript engine
*/

#if defined(USING_SPIDERMONKEY)
#include "uvd_javascript_spidermonkey.h"
#define UVDJavascriptInterpreter UVDJavascriptSpidermonkeyInterpreter
#elif defined(USING_SPIDERAPE)
#include "uvd_javascript_spiderape.h"
#define UVDJavascriptInterpreter UVDJavascriptSpiderapeInterpreter
#else
#error Javascript support requires specifying an engine to use
#endif

