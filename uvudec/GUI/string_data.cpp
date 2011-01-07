/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "GUI/GUI.h"
#include "GUI/string_data.h"
#include "uvd/string/engine.h"
#include "uvd/util/debug.h"
#include <math.h>

/*
UVDGUIStringData
*/

UVDGUIStringData::UVDGUIStringData()
{
	printf("UVDGUIStringData::UVDGUIStringData()\n");
	//Try the simple case first
	m_singleLineStrings = true;
}

uv_err_t UVDGUIStringData::begin(unsigned int offset, unsigned int index, UVQtDynamicTextData::iterator *out)
{
	UVDGUIStringData::iterator_impl *iter_impl = new UVDGUIStringData::iterator_impl(this, offset, index);
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	*out = iter;
	return UV_ERR_OK;
}

uv_err_t UVDGUIStringData::end(iterator *out)
{
	UVDGUIStringData::iterator_impl *iter_impl = new UVDGUIStringData::iterator_impl(this,
			getNumberStrings());
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	*out = iter;
	return UV_ERR_OK;
}
	
uv_err_t UVDGUIStringData::getMinOffset(unsigned int *out)
{
	*out = 0;
	return UV_ERR_OK;
}

unsigned int UVDGUIStringData::getNumberStrings()
{
	if( getStringEngine() )
	{
		return getStringEngine()->m_strings.size();
	}
	else
	{
		return 0;
	}
}

UVDStringEngine *UVDGUIStringData::getStringEngine()
{
	if( g_mainWindow
			&& g_mainWindow->m_project
			&& g_mainWindow->m_project->m_uvd
			&& g_mainWindow->m_project->m_uvd->m_analyzer
			&& g_mainWindow->m_project->m_uvd->m_analyzer->m_stringEngine )
	{
		return g_mainWindow->m_project->m_uvd->m_analyzer->m_stringEngine;
	}
	else
	{
printf("string engine inaccessible\n");
		return NULL;
	}
}

uv_err_t UVDGUIStringData::getMaxOffset(unsigned int *out)
{
	if( getNumberStrings() == 0 )
	{
		*out = 0;
	}
	else
	{
		*out = getNumberStrings() - 1;
	}
printf("set max offset: %d\n", *out);
	return UV_ERR_OK;
}

