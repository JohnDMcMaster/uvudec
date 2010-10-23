/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_OBJECT_FACTORY_H
#define UVD_OBJECT_FACTORY_H

#include "uvd/flirt/pat/pat.h"

//typedef std::pair<UVDFLIRTPatternGenerator::CanLoad, UVDFLIRTPatternGenerator::TryLoad> UVDFLIRTPatFactoryLoader;
class UVDFLIRTPatFactoryLoader
{
public:
	UVDFLIRTPatFactoryLoader();
	UVDFLIRTPatFactoryLoader(std::string name,
			UVDFLIRTPatternGenerator::CanLoad canLoad, UVDFLIRTPatternGenerator::TryLoad tryLoad,
			void *data = NULL);
	~UVDFLIRTPatFactoryLoader();

public:
	std::string m_name;
	UVDFLIRTPatternGenerator::CanLoad m_canLoad;
	UVDFLIRTPatternGenerator::TryLoad m_tryLoad;
	//User supplied data to be passed to callback
	void *m_user;
};

/*
Abstract factory for deciding on the best object to generate patterns for a given runtime
*/
class UVDFLIRTPatFactory
{
public:
	typedef std::map<std::string, UVDFLIRTPatFactoryLoader> Loaders;
	typedef std::pair<UVDFLIRTPatFactoryLoader, uvd_priority_t> Priority;
	typedef std::vector<Priority> PriorityList;
	
public:
	UVDFLIRTPatFactory();
	~UVDFLIRTPatFactory();

	//Neither canLoad or tryLoad should have been already registered (and not unreigstered)
	//name must be unique to the factory
	//user: passed to callback when called
	uv_err_t registerObject(const std::string &name, UVDFLIRTPatternGenerator::CanLoad canLoad, UVDFLIRTPatternGenerator::TryLoad tryLoad, void *user = NULL);
	uv_err_t unregisterObject(const std::string &name);
	uv_err_t unregisterObject(UVDFLIRTPatternGenerator::TryLoad tryLoad);

	//Return the best candidates
	//For now, only the tied priorty results are returned
	uv_err_t canLoad(const UVDRuntime *runtime, PriorityList &bestCandidates);
	//Return the best candidate level
	//If there is a tie, one will be chosen arbitrarily
	uv_err_t tryLoad(const UVDRuntime *runtime, UVDFLIRTPatternGenerator **out);

public:
	//std::vector<UVDFLIRTPatFactoryLoader> m_loaders;
	Loaders m_loaders;
	//std::set<UVDFLIRTPatternGenerator::CanLoad> m_canLoad;
	//std::set<UVDFLIRTPatternGenerator::TryLoad> m_tryLoad;
};

//Throw in some gray magic just to keep things interesting
#define UVDFLIRTPatFactoryRegister(factory, Class) \
		factory.registerObject(Class::canLoad, Class::tryLoad)
#define UVDFLIRTPatFactoryUnregister(factory, Class) \
		factory.unregisterObject(Class::canLoad, Class::tryLoad)

#endif

