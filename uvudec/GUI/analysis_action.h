/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

#ifndef UVD_ANALYSIS_ACTION
#define UVD_ANALYSIS_ACTION

class UVDAnalysisAction
{
public:
	UVDAnalysisAction();
	virtual ~UVDAnalysisAction();

public:
};

class UVDAnalysisActionBegin : public UVDAnalysisAction
{
};

#endif

