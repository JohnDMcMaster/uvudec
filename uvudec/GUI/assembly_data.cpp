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
UVDGUIAssemblyData
*/

UVDGUIAssemblyData::UVDGUIAssemblyData()
{
	printf("UVDGUIAssemblyData::UVDGUIAssemblyData()\n");
}

uv_err_t UVDGUIAssemblyData::begin(unsigned int offset, unsigned int index, UVQtDynamicTextData::iterator *out)
{
	UVDGUIAssemblyData::iterator_impl *iter_impl = new UVDGUIAssemblyData::iterator_impl(this, offset, index);
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	*out = iter;
	return UV_ERR_OK;
}

uv_err_t UVDGUIAssemblyData::end(iterator *out)
{
	UVDGUIAssemblyData::iterator_impl *iter_impl = new UVDGUIAssemblyData::iterator_impl(this,
			getNumberStrings());
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	*out = iter;
	return UV_ERR_OK;
}
	
UVD *UVDGUIAssemblyData::getUVD()
{
	if( g_mainWindow
			&& g_mainWindow->m_project
			&& g_mainWindow->m_project->m_uvd )
	{
		return g_mainWindow->m_project->m_uvd;
	}
	else
	{
		printf("uvd engine inaccessible\n");
		return NULL;
	}
}

UVDAddressSpace *UVDGUIAssemblyData::getAddressSpace()
{
	UVD *uvd = getUVD();

	if( uvd
			&& uvd->m_runtime )
	{
		UVDAddressSpace *ret = NULL;
		
		uv_assert_err_ret(uvd->m_runtime->getPrimaryExecutableAddressSpace(&ret));
		if( ret )
		{
			return ret;
		}
	}
	printf("address space inaccessible\n");
	return NULL;
}

uv_err_t UVDGUIAssemblyData::getMinOffset(unsigned int *out)
{
	*out = 0;

	if( getAddressSpace() != NULL )
	{
		uv_assert_err_ret(getAddressSpace()->getMinValidAddress(out));
	}
	//printf("set min offset: %d\n", *out);
	return UV_ERR_OK;
}

uv_err_t UVDGUIAssemblyData::getMaxOffset(unsigned int *out)
{
	*out = 0;

	if( getAddressSpace() != NULL )
	{
		uv_assert_err_ret(getAddressSpace()->getMaxValidAddress(out));
	}
	//printf("set max offset: %d\n", *out);
	return UV_ERR_OK;
}

