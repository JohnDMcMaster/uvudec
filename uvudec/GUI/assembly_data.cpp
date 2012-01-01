/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "GUI/GUI.h"
#include "GUI/assembly_data.h"
#include "uvd/core/runtime.h"
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
	UVDPrintIterator endIter;
	UVDGUIAssemblyData::iterator_impl *iter_impl = NULL;
	
	if( getUVD() )
	{
		uv_assert_err_ret(getUVD()->end(endIter));
		iter_impl = new UVDGUIAssemblyData::iterator_impl(this, endIter);
	}
	else
	{
		iter_impl = new UVDGUIAssemblyData::iterator_impl(this, 0, 0);
	}
	uv_assert_ret(iter_impl);
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
		//printf("uvd engine inaccessible\n");
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
		
		UV_DEBUG(uvd->m_runtime->getPrimaryExecutableAddressSpace(&ret));
		if( ret )
		{
			return ret;
		}
	}
	//printf("address space inaccessible\n");
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

