/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdflirt/pat/factory.h"
#include "uvdflirt/flirt.h"

UVDFLIRTPatFactoryLoader::UVDFLIRTPatFactoryLoader()
{
	m_canLoad = NULL;
	m_tryLoad = NULL;
	m_user = NULL;
}

UVDFLIRTPatFactoryLoader::UVDFLIRTPatFactoryLoader(std::string name,
			UVDFLIRTPatternGenerator::CanLoad canLoad, UVDFLIRTPatternGenerator::TryLoad tryLoad,
			void *user)
{
	m_name = name;
	m_canLoad = canLoad;
	m_tryLoad = tryLoad;
	m_user = user;
}

UVDFLIRTPatFactoryLoader::~UVDFLIRTPatFactoryLoader()
{
}

/*
UVDFLIRTPatFactory
*/

UVDFLIRTPatFactory::UVDFLIRTPatFactory()
{
}

UVDFLIRTPatFactory::~UVDFLIRTPatFactory()
{
}

uv_err_t UVDFLIRTPatFactory::registerObject(const std::string &name, UVDFLIRTPatternGenerator::CanLoad canLoad, UVDFLIRTPatternGenerator::TryLoad tryLoad, void *data)
{
	uv_assert_ret(this);
	uv_assert_ret(m_loaders.find(name) == m_loaders.end());
	m_loaders[name] = UVDFLIRTPatFactoryLoader(name, canLoad, tryLoad, data);
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatFactory::unregisterObject(UVDFLIRTPatternGenerator::TryLoad tryLoadIn)
{
	for( Loaders::iterator iter = m_loaders.begin();
			iter != m_loaders.end(); ++iter )
	{
		//Try load is always needed, canLoad isn't
		UVDFLIRTPatternGenerator::TryLoad tryLoadCurrent = ((*iter).second).m_tryLoad;
		if( tryLoadCurrent == tryLoadIn )
		{
			m_loaders.erase((*iter).first);
			return UV_ERR_OK;
		}
	}
	return UV_ERR_NOTFOUND;	
}

uv_err_t UVDFLIRTPatFactory::unregisterObject(const std::string &name)
{
	if( m_loaders.find(name) == m_loaders.end() )
	{
		return UV_ERR_NOTFOUND;	
	}

	m_loaders.erase(m_loaders.find(name));

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatFactory::canLoad(const UVDRuntime *runtime, PriorityList &bestCandidates)
{
	uvd_priority_t confidence = UVD_MATCH_NONE;

	printf_flirt_debug("canLoad(), n loaders: %d\n", m_loaders.size());
	bestCandidates = PriorityList();
	
	for( Loaders::iterator iter = m_loaders.begin();
			iter != m_loaders.end(); ++iter )
	{
		const UVDFLIRTPatFactoryLoader &loader = ((*iter).second);
		UVDFLIRTPatternGenerator::CanLoad canLoad = loader.m_canLoad;
		uvd_priority_t confidenceTemp = UVD_MATCH_NONE;
		
		uv_assert_ret(canLoad);
		if( UV_SUCCEEDED(canLoad(runtime, &confidenceTemp, loader.m_user)) )
		{
			printf_flirt_debug("can load confidence: %d\n", confidenceTemp);
			//Better value?
			if( confidenceTemp < confidence )
			{
				confidence = confidenceTemp;
				bestCandidates.clear();
			}
			//A valid value?
			if( confidenceTemp <= confidence )
			{
				Priority priority = Priority(loader, confidenceTemp);
				bestCandidates.push_back(priority);
			}
		}
	}
	
	//If all of the matches were bad, don't return them
	if( confidence >= UVD_MATCH_NONE )
	{
		bestCandidates.clear();
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatFactory::tryLoad(const UVDRuntime *runtime, UVDFLIRTPatternGenerator **out)
{
	PriorityList bestCandidates;
	UVDFLIRTPatternGenerator::TryLoad tryLoadCallback = NULL;

	uv_assert_err_ret(canLoad(runtime, bestCandidates));
	if( bestCandidates.empty() )
	{
		printf_warn("no valid pattern generators for given runtime\n");
		return UV_ERR_NOTSUPPORTED;
	}

	//Arbitrarily chose the first
	UVDFLIRTPatFactoryLoader firstCandidate = (*bestCandidates.begin()).first;
	tryLoadCallback = firstCandidate.m_tryLoad;
	uv_assert_err_ret(tryLoadCallback(runtime, out, firstCandidate.m_user));
	
	return UV_ERR_OK;
}

