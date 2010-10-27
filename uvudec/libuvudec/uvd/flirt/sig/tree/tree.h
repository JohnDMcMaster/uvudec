/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_SIGNATURE_TREE_H
#define UVD_FLIRT_SIGNATURE_TREE_H

#include "uvd/util/types.h"
#include "uvd/flirt/function.h"
#include <set>

/*
For working with .sig files

Signature file stores things in roughly this tree structure (after header):
-Internal nodes
	-If a signature is less than 32 leading chars, it will end sooner
-Leaf node
	-First sorted by crc16
	-Next bucketed by equal crc16 and same prefix length

When I feel like beefing this up, my computational biology class algorithms should come in handy

There are a lot of options how this could be implemented
Eventually since I want to do this on a very large scale, I'll have to move to something more efficient
For now, something quick and dirty

Basic tree blocks
	Escaped technique
	-Use / as an escape char
	-// would mean literal /
	-/. would mean pattern byte
	Advantages
	-On average more memory efficient
	Disadvantages
	-If we need random access, it will be CPU intensive
		Seems like something we wouldn't need
		Possibly for relocations

	Aligned technique
	-Store an array of byte objects/structures
	-One member would represent data type, second part the data
		One type for actual byte, another for wildcard

Tree branches
	For now just use a simple vector
	Later consider AVL tree or something similar
*/

/*
The two subclasses are quite different, but it does help tracking what is a valid tree member
Actually, the reason for doing this no longer applies with current construction
*/

/*
The lowest level of the tree
Should be all of the information (combied with above) to 
Was UVDFLIRTSignatureElement
*/
class UVDFLIRTModule;
class UVDFLIRTSignatureTreeBasicNode
{
public:
	UVDFLIRTSignatureTreeBasicNode();
	static uv_err_t fromModule(const UVDFLIRTModule *function, UVDFLIRTSignatureTreeBasicNode **out);
	~UVDFLIRTSignatureTreeBasicNode();
	
	uv_err_t insertReference(UVDFLIRTSignatureReference reference);
	//LIke this - second
	int compare(const UVDFLIRTSignatureTreeBasicNode *second);

	uv_err_t size(uint32_t *size);

	std::string debugString();
	uv_err_t debugDump(const std::string &prefix, uint32_t basicNodeIndex);

public:
	//This admitedly can be somewhat vague
	//Its up to the definer to make sure these don't collide
	//This code should NOT map these to current code objects, this should be handeled externally
	//Or I guess if you want to agressivly report names, you just merge the collisions onto one object
	//TODO: change this to reference list or similar that can also save an offset
	std::vector<UVDFLIRTPublicName> m_publicNames;
	std::vector<UVDFLIRTSignatureReference> m_references;
	/*
	How many bytes were used to calc the crc16
	If this is 0, m_crc16 should also be 0
	So in .pat file crc16 length is given, but in .sig file total length is given
	m_crc16Length = max(0, m_totalLength - 0x20)
	*/
	//uint32_t m_crc16Length;
	//Need to define these
	uint32_t m_attributeFlags;
	uint32_t m_totalLength;
};

/*
Leaf nodes are a bucket for hash collisions
Was UVDFLIRTSignatureTreeLeafNode
*/

class UVDFLIRTSignatureTreeHashNode;
struct UVDFLIRTSignatureTreeBasicNodeCompare
{
	bool operator()(UVDFLIRTSignatureTreeBasicNode *first, UVDFLIRTSignatureTreeBasicNode *second) const;
};

class UVDFLIRTSignatureTreeHashNode
{
public:
	//Returns true if first (l) precedes second (r)
	//f(x, x) must be false. 
	//static int basicSetCompare(UVDFLIRTSignatureTreeBasicNode *l, UVDFLIRTSignatureTreeBasicNode *r);
	//static bool basicSetCompareStrictWeak(UVDFLIRTSignatureTreeBasicNode *l, UVDFLIRTSignatureTreeBasicNode *r);
	typedef std::set<UVDFLIRTSignatureTreeBasicNode *, UVDFLIRTSignatureTreeBasicNodeCompare> BasicSet;
	//typedef std::set<UVDFLIRTSignatureTreeBasicNode *> BasicSet;

public:
	UVDFLIRTSignatureTreeHashNode();
	UVDFLIRTSignatureTreeHashNode(const UVDFLIRTModule *function);
	~UVDFLIRTSignatureTreeHashNode();
	
	//Do a full collision check and such
	uv_err_t insert(UVDFLIRTModule *function);
	uv_err_t compare(const UVDFLIRTSignatureTreeHashNode *second);

	uv_err_t size(uint32_t *size);
	uv_err_t debugDump(const std::string &prefix, uint32_t hashNodeIndex);

public:
	//Is this supposed to be sorted by anything?
	BasicSet m_bucket;
	//totalLength maybe?  Its one of the few things left
	//yes, appears should be sorted by totalLength
	//std::vector<UVDFLIRTSignatureTreeBasicNode *> m_bucket;

	//Entire length including prefix and tailing bytes, if present
	uint16_t m_crc16;
	uint32_t m_leadingLength;
};

struct UVDFLIRTSignatureTreeHashNodeCompare
{
	bool operator()(UVDFLIRTSignatureTreeHashNode *first, UVDFLIRTSignatureTreeHashNode *second) const;
};
class UVDFLIRTSignatureTreeHashNodes
{
public:
	//static int hashSetCompare(UVDFLIRTSignatureTreeHashNode *l, UVDFLIRTSignatureTreeHashNode *r);
	typedef std::set<UVDFLIRTSignatureTreeHashNode *, UVDFLIRTSignatureTreeHashNodeCompare> HashSet;

public:
	UVDFLIRTSignatureTreeHashNodes();
	~UVDFLIRTSignatureTreeHashNodes();

	uv_err_t insert(UVDFLIRTModule *function);
	uv_err_t debugDump(const std::string &prefix);

public:
	//Takes care of sorting itself
	HashSet m_nodes;
};

//was UVDFLIRTSignatureTreeInternalNode
/*
Things to remember
-We may have a 32 byte signature and a subsequence prefix match
This implementation was done in the odd C/C++ style because it was the easiest to do with STL containers that matched FLIRT spec
*/
class UVDFLIRTSignatureTreeLeadingNode;
struct UVDFLIRTSignatureLeadingNodeCompare
{
	bool operator()(UVDFLIRTSignatureTreeLeadingNode *first, UVDFLIRTSignatureTreeLeadingNode *second) const;
};
class UVDFLIRTSignatureTreeLeadingNode
{
public:
	//static int leadingChildrenComapre(UVDFLIRTSignatureTreeLeadingNode *l, UVDFLIRTSignatureTreeLeadingNode *r);
	typedef std::set<UVDFLIRTSignatureTreeLeadingNode *, UVDFLIRTSignatureLeadingNodeCompare> LeadingChildrenSet;
	//typedef std::set<UVDFLIRTSignatureTreeLeadingNode *> LeadingChildrenSet;

public:
	UVDFLIRTSignatureTreeLeadingNode();
	~UVDFLIRTSignatureTreeLeadingNode();
	static uv_err_t fromSubsequence(const UVDFLIRTSignatureRawSequence *seq, UVDFLIRTSignatureRawSequence::const_iterator pos, uint32_t n, UVDFLIRTSignatureTreeLeadingNode **out);
	
	//Careful using these
	//uv_err_t insertLeadingNode(UVDFLIRTSignatureTreeLeadingNode *node, std::vector<UVDFLIRTSignatureTreeNode *>::iterator pos);
	//uv_err_t insertHashNode(UVDFLIRTSignatureTreeLeafNode *node);
	//uv_err_t insertHashNode(UVDFLIRTModule *function);
	
	//Place the node as best as possible into the tree, noting we have already parsed the first so many bytes
	//Internal nodes will be rearranged as necessary
	//Originally I was thinking we might steal the data allocated in function, but since we will likely have ot split, its not economical
	uv_err_t insert(UVDFLIRTModule *function);
	//uv_err_t insertSubseq(UVDFLIRTModule *function, UVDFLIRTSignatureRawSequence::const_iterator sequencePosition);

	uv_err_t size(uint32_t *size);

	//Create a new child node, splitting us at pos
	//pos should not equal begin and will be the first node in the new tree
	uv_err_t split(UVDFLIRTSignatureRawSequence::iterator pos);
	
	uv_err_t debugDump(const std::string &prefix);
	
public:
	//For the current node
	UVDFLIRTSignatureRawSequence m_bytes;
	
	UVDFLIRTSignatureTreeHashNodes m_crcNodes;
	//Sorted by next bytes
	LeadingChildrenSet m_leadingChildren;
};

#endif

