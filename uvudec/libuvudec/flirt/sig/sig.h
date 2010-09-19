/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_SIGNATURE_H
#define UVD_FLIRT_SIGNATURE_H

#include "uvd_types.h"
#include "uvd_data.h"
#include <vector>

#define UVD__FLIRT__CONFLICT__RESOLUTION__UNKNOWN		0
//No action has been taken against it
//Default policies will be used
#define UVD__FLIRT__CONFLICT__RESOLUTION__NONE			1
//It should be added to the output signature file
#define UVD__FLIRT__CONFLICT__RESOLUTION__KEEP			2
//For one reason or another, it should be discarded and will not be in the output file
#define UVD__FLIRT__CONFLICT__RESOLUTION__DISCARD		3


class UVDFLIRTSignatureTreeLeafNode;
class UVDFLIRTSignatureDBConflictSource
{
public:
	UVDFLIRTSignatureDBConflictSource();
	~UVDFLIRTSignatureDBConflictSource();	

public:
	//We do not own this
	UVDFLIRTSignatureTreeLeafNode *m_node;
	std::string m_patFileName;
	//Starting at 1.  0 means unknown/invalid
	uint32_t m_patFileLine;
	//If any action has been taken against it
	uint32_t m_resolution;
};

class UVDFLIRTSignatureDBConflict
{
public:
	UVDFLIRTSignatureDBConflict();
	~UVDFLIRTSignatureDBConflict();
	
public:
	//hmm I'd like file sources as well
	std::vector<UVDFLIRTSignatureDBConflictSource> m_conflictingNodes;
};

class UVDFLIRTSignatureDBConflicts
{
public:
	UVDFLIRTSignatureDBConflicts();
	~UVDFLIRTSignatureDBConflicts();
	
	/*
	Write out a human editable conflict file to resolve conflicts
	*/
	uv_err_t writeToFile(const std::string &fileNameIn);
	uv_err_t writeToString(std::string &out);
	
	static uv_err_t readFromFile(const std::string &fileNameIn, UVDFLIRTSignatureDBConflicts **out);
	static uv_err_t readFromString(const std::string &in, UVDFLIRTSignatureDBConflicts **out);
	
public:
	//Move to pointers if needed
	std::vector<UVDFLIRTSignatureDBConflict> m_conflicts;
};

/*
For working with .sig files
*/
class UVDFLIRTPatternAnalysis;
class UVDFLIRT;
class UVDFLIRTSignatureTreeLeadingNode;
class UVDFLIRTFunction;
class UVDFLIRTSignatureDB
{
public:
	UVDFLIRTSignatureDB();
	~UVDFLIRTSignatureDB();
	uv_err_t init();
	uv_err_t deinit();
	
	/*
	Using signatures
	*/
	//uv_err_t pat2sig(UVDFLIRTPatternAnalysis *pat);
	
	/*
	Save .sig to given file
	*/
	uv_err_t writeToFile(const std::string &file);
	uv_err_t writeToData(UVDData **out);
	
	/*
	Load from a .sig file
	Work in progress
	*/
	uv_err_t loadFromFile(const std::string &file);
	
	/*
	Will not clear the DB
	m_conflicts set: list and report conflicts.  Node will be forcefully added
	m_conflicts clear: conflicts fatal
	*/
	uv_err_t loadFromPatFile(const std::string &file);
	uv_err_t loadFromPatFileString(const std::string &patternFileContents);

	uv_err_t insert(UVDFLIRTFunction *function);

	//How many total patterns in the DB
	//"number modules"
	uv_err_t size(uint32_t *size);

	//Debug dump of the DB like (but not exactly like) dumpsig would give
	uv_err_t debugDump();

public:
	//The UVD FLIRT engine
	//We do not own it
	UVDFLIRT *m_flirt;

	/*
	Our .sig file, if we created through file read
	We own it
	*/
	UVDData *m_data;
	
	//The actual signatures
	//We own this
	//Dynamically allocated because all other tree nodes are
	//careful accessing this if tree is empty
	//XXX: later, we will make the engine configurable
	//This engine should be kept for dumping and such though as its simple
	UVDFLIRTSignatureTreeLeadingNode *m_tree;
	
	//If any conflicts are forcefully added, set to indicate they need to be resolved
	//uint32_t m_isConsistent;
	//Don't bother, use m_conflicts.empty() or error
	
	/*
	Determintes mode of operations
	We own this
	If present, we will record conflicts and allow an inconsistant DB state
	*/
	UVDFLIRTSignatureDBConflicts *m_conflicts;

	std::string m_libraryName;
};

#define UVD_FLIRT_SIG_LEADING_LENGTH				0x20

#endif

