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
			g_mainWindow->m_project->m_uvd->m_analyzer->m_stringEngine->m_strings.size());
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	*out = iter;
	return UV_ERR_OK;
}
	
uv_err_t UVDGUIStringData::getMinOffset(unsigned int *out)
{
	*out = 0;
	return UV_ERR_OK;
}

uv_err_t UVDGUIStringData::getMaxOffset(unsigned int *out)
{
	*out = g_mainWindow->m_project->m_uvd->m_analyzer->m_stringEngine->m_strings.size() - 1;
	return UV_ERR_OK;
}

