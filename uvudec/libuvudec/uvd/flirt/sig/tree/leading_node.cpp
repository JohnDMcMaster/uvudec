/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/config/config.h"
#include "uvd/flirt/pat/pat.h"
#include "uvd/flirt/sig/tree/tree.h"
#include "uvd/util/util.h"
#include "uvd/flirt/flirt.h"
#include <limits.h>

/*
UVDFLIRTSignatureTreeLeadingNode
*/

bool UVDFLIRTSignatureLeadingNodeCompare::operator()(UVDFLIRTSignatureTreeLeadingNode *first, UVDFLIRTSignatureTreeLeadingNode *second) const
{
	if( first == second )
	{
		return true;
	}
	//True if first strictly less than second
	return first->m_bytes.compare(&second->m_bytes) < 0;
}

UVDFLIRTSignatureTreeLeadingNode::UVDFLIRTSignatureTreeLeadingNode()
{
	//m_hasLeadingChildren = 0;
}

UVDFLIRTSignatureTreeLeadingNode::~UVDFLIRTSignatureTreeLeadingNode()
{
	/*
	FIXME: clean this up
	
	union...deal with destructor later
	if( m_hasLeadingChildren )
	for( std::vector<UVDFLIRTSignatureTreeNode *>::iterator iter = m_children.begin(); iter != m_children.end(); ++iter )
	{
		{
			delete (UVDFLIRTSignatureTreeHashNode *)*iter;
		}
		else
		{
			delete (UVDFLIRTSignatureTreeLeadingNode *)*iter;
		}
	}
	*/
}

/*
uv_err_t UVDFLIRTSignatureTreeLeadingNode::insertHashNode(UVDFLIRTModule *function)
{
	uv_assert_err_ret(m_crcNodes.insert(node));
	
	return UV_ERR_OK;
}
*/

#if 0
uv_err_t UVDFLIRTSignatureTreeLeadingNode::insertHashNode(UVDFLIRTSignatureTreeHashNode *node)
{
	/*
    Ira Kane: Hey, cool. Snag one.
    Harry Block: Snag one?
    Kane: Yeah, snag it and put it in the bucket.
    Block: I've seen this movie, the black dude dies first. You snag it.
	(http://en.wikiquote.org/wiki/Evolution_%28film%29)
    */
	uv_assert_err_ret(m_crcNodes.insert(node));
	return UV_ERR_OK;
}
#endif

//static uv_err_t fromSubsequence(const UVDFLIRTSignatureRawSequence *seq, UVDFLIRTSignatureRawSequence::const_iterator pos, uint32_t n, UVDFLIRTSignatureTreeLeadingNode **out);
uv_err_t UVDFLIRTSignatureTreeLeadingNode::fromSubsequence(const UVDFLIRTSignatureRawSequence *seq, UVDFLIRTSignatureRawSequence::const_iterator pos, uint32_t n, UVDFLIRTSignatureTreeLeadingNode **out)
{
	UVDFLIRTSignatureTreeLeadingNode *ret = NULL;
	
	printf_flirt_debug("new node from subsequence\n");

	//Create new child node
	ret = new UVDFLIRTSignatureTreeLeadingNode();
	uv_assert_ret(ret);
	uv_assert_err_ret(seq->subseqTo(&ret->m_bytes, pos, n));
		
	uv_assert_ret(out);
	*out = ret;
		
	return UV_ERR_OK;

}

uv_err_t UVDFLIRTSignatureTreeLeadingNode::split(UVDFLIRTSignatureRawSequence::iterator pos)
{
	UVDFLIRTSignatureTreeLeadingNode *child = NULL;
	std::set<UVDFLIRTSignatureTreeLeadingNode *> temp;
	//To assert hunt down a nasty infinite recursion error
	uint32_t preSplitSize = 0;
	
	printf_flirt_debug("new node from split, orig node: %s\n", m_bytes.toString().c_str());
	//Should not split at beginning
	uv_assert_ret(pos.m_cur != m_bytes.m_bytes);
	//UVD_PRINT_STACK();
	
	preSplitSize = m_bytes.size();
	//Create new child node
	child = new UVDFLIRTSignatureTreeLeadingNode();
	uv_assert_ret(child);
	uv_assert_err_ret(m_bytes.subseqTo(&child->m_bytes, pos));
	uv_assert_err_ret(m_bytes.truncate(pos));
	
	//transfer children to it
	child->m_leadingChildren = m_leadingChildren;
	m_leadingChildren.clear();
	child->m_crcNodes.m_nodes = m_crcNodes.m_nodes;
	m_crcNodes.m_nodes.clear();
	
	printf_flirt_debug("split, orig new state: %s, child: %s\n", m_bytes.toString().c_str(), child->m_bytes.toString().c_str());
	//We should be branching on this node, not creating children
	uv_assert_ret(m_bytes.size() != 0);

	//Out with the old, in with the new
	m_leadingChildren.insert(child);

	uv_assert_ret(preSplitSize != m_bytes.size());

	
	return UV_ERR_OK;
}

/*
Alternativly, we could have added code with tightly interwoven end iterators
This was much easier for now to get to work with old code
*/
class UVDFLIRTSignatureTreeLeadingNodeInserter
{
public:
	//Entry point
	uv_err_t insert(UVDFLIRTSignatureTreeLeadingNode *rootNode, UVDFLIRTModule *function);
	//When it is determinted node shares no bytes with current position
	uv_err_t insertChildLeadingMatch(UVDFLIRTSignatureTreeLeadingNode *node, UVDFLIRTSignatureRawSequence::const_iterator sequencePosition);
	//Recursive case
	uv_err_t insertSubseq(UVDFLIRTSignatureTreeLeadingNode *node, UVDFLIRTSignatureRawSequence::const_iterator sequencePosition);

public:
	//The sequence we are trying to insert
	UVDFLIRTSignatureRawSequence m_leadingSequence;
	//Function the sequencei s from
	//We'll need this to go to the lower levels
	UVDFLIRTModule *m_function;
};

uv_err_t UVDFLIRTSignatureTreeLeadingNode::insert(UVDFLIRTModule *function)
{
	UVDFLIRTSignatureTreeLeadingNodeInserter inserter;

	//UVD_PRINT_STACK();
	printf_flirt_debug("Inserting function into db, size: %d, seq: %s\n", function->m_sequence.size(), function->m_sequence.toString().c_str());
	
	uv_assert_err_ret(inserter.insert(this, function));
	printf_flirt_debug("function done\n\n");
	return UV_ERR_OK;
};

uv_err_t UVDFLIRTSignatureTreeLeadingNodeInserter::insert(UVDFLIRTSignatureTreeLeadingNode *rootNode, UVDFLIRTModule *function)
{
	//TODO: we could add a special case for seq length <= 0x20 to use the input function instead
	//Careful not to delete it upon cleanup (or double frees rather)
	uint32_t leadingLength = 0;
	
	leadingLength = uvd_min(function->m_sequence.size(), g_config->m_flirt.m_patLeadingLength);	
	uv_assert_err_ret(function->m_sequence.subseqTo(&m_leadingSequence, function->m_sequence.const_begin(), leadingLength));
	
	m_function = function;

	printf_flirt_debug("Second stage begin insert, lead seq (size = %d): %s\n", m_leadingSequence.size(), m_leadingSequence.toString().c_str());
	uv_assert_ret(leadingLength == m_leadingSequence.size());
	//Since root node has no sequence associated with it, we must bypass the prefix check
	return UV_DEBUG(insertChildLeadingMatch(rootNode, m_leadingSequence.begin()));
}

uv_err_t UVDFLIRTSignatureTreeLeadingNodeInserter::insertChildLeadingMatch(UVDFLIRTSignatureTreeLeadingNode *node, UVDFLIRTSignatureRawSequence::const_iterator sequencePosition)
{
	UVDFLIRTSignatureTreeLeadingNode *childNode = NULL;

	printf_flirt_debug("checking for this and input leading match on node 0x%08X\n", (int)node);
	/*
	If there are any matches below us, we only need to match the first byte and recurse
	*/
	for( UVDFLIRTSignatureTreeLeadingNode::LeadingChildrenSet::iterator iter = node->m_leadingChildren.begin(); iter != node->m_leadingChildren.end(); ++iter )
	{
		UVDFLIRTSignatureTreeLeadingNode *cur = *iter;
		
		uv_assert_ret(cur);
		uv_assert_ret(cur != node);
		//First byte should match
		//We should be ga
		if( (*cur->m_bytes.begin()) == (*sequencePosition) )
		{
			childNode = cur;
			break;
		}
	}

	//if we have a partial match, we should further find out place below
	if( childNode )
	{
		printf_flirt_debug("matching child node 0x%08X, recursing\n", (int)childNode);
		//Alternativly we could recurse, but since we already know what needs to be done, why bother
		uv_assert_err_ret(insertSubseq(childNode, sequencePosition));
	}
	//No prefix match?  Branch here instead
	else
	{
		printf_flirt_debug("no child prefix match on current node, initiating split on input function until end\n");

		////static uv_err_t fromSubsequence(const UVDFLIRTSignatureRawSequence *seq, UVDFLIRTSignatureRawSequence::iterator pos, uint32_t n, UVDFLIRTSignatureTreeLeadingNode **out);
		uv_assert_err_ret(node->fromSubsequence(&m_leadingSequence, sequencePosition, UVDFLIRTSignatureRawSequence::npos, &childNode));
		uv_assert_ret(childNode);
		node->m_leadingChildren.insert(childNode);

		//Alternativly we could recurse, but since we already know what needs to be done, why bother
		uv_assert_err_ret(childNode->m_crcNodes.insert(m_function));
		//uv_assert_err_ret(insertSubseq(childNode, sequencePosition));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureTreeLeadingNodeInserter::insertSubseq(UVDFLIRTSignatureTreeLeadingNode *node, UVDFLIRTSignatureRawSequence::const_iterator sequencePosition)
{
	UVDFLIRTSignatureRawSequence::iterator nodeBranchPoint;
	
	/*
	Cases
	
	Full match:
		Add to m_crcNodes
	Partial match
		Split the node at the point
		Two children
		One is the previous (with all of the children xferred over)
		Other is our new node
	*/
	
	printf_flirt_debug("on node 0x%08X\n", (int)node);
	//Start by finding how much of the sequence we share
	//uv_err_t matchPosition(const UVDFLIRTSignatureRawSequence *other, const_iterator &otherStartEnd, const_iterator &thisMatchPoint) const;
	uv_assert_err_ret(node->m_bytes.differencePosition(&m_leadingSequence, sequencePosition, nodeBranchPoint));
	printf_flirt_debug("difference, existing: %s, inserting: %s\n", sequencePosition.toDebugString().c_str(), nodeBranchPoint.toDebugString().c_str());
	
	/*
	A partial match
	Split and other craziness
	Recursive
	Case: our original node is longer
	Ex
		old Node:  ABCA 232ABC
		new Func:  ABCA
		               /\
		                \Split
	Ex
		old Node:  ABCA 232ABC
		new Func:  ABCA 993432
		               /\
		                \Split
	Split original if we are in an intermediate case
	*/
	if( nodeBranchPoint != node->m_bytes.end() && nodeBranchPoint != node->m_bytes.begin() )
	{
		//UVDFLIRTSignatureTreeLeadingNode *newNode = NULL;
		printf_flirt_debug("initiating split\n");
		
		uv_assert_err_ret(node->split(nodeBranchPoint));
		//We should only have a single child now of our new node
		uv_assert_ret(node->m_leadingChildren.size() == 1);		
	}

	/*
	Full tail match?
	This is the (most common) terminal case
	Either from having a 
	
	Ex
		old Node:  ABCA232ABC
		new Our:   ABCA232ABC
	This also will happen if we split our current node because the seq in was smaller
		See above
		Ex
			old Node:  ABCA 232ABC
			new Func:  ABCA
	*/
	if( sequencePosition == m_leadingSequence.end() )
	//if( nodeBranchPoint == node->m_bytes.end() && sequencePosition == m_leadingSequence.end() )
	{
		printf_flirt_debug("iniating hash insert\n");
		//Add to our bucket
		uv_assert_err_ret(node->m_crcNodes.insert(m_function));
	}
	/*
	Ex
		Node:  ABCA
		Func:  ABCA 232ABC
		           /\
		            \Split
	Ex
		Note: this case is now unimportant since above split if necessary
			Watch out for use of nodeBranchPoint though which is now invalid
		Node:  ABCA 232ABC
		Func:  ABCA 993432
		           /\
		            \Split
		We may or may not have some matching bytes afer this
		If so, we should recursive to that node
	*/
	else
	{
		uv_assert_err_ret(insertChildLeadingMatch(node, sequencePosition));
	}

#if 0
	/*
	A partial match
	Split and other craziness
	Recursive
	Case: our original node is longer
	Ex
		Node:  ABCA232ABC
		Our:   ABCA
		           /\
		            \Split
	Split original and add to the original
	*/
	else if( sequencePosition == m_leadingSequence.end() )
	{
		UVDFLIRTSignatureTreeLeadingNode *newNode = NULL;
		
		uv_assert_err_ret(node->split(nodeBranchPoint));
		//We should only have a single child now of our new node
		uv_assert_ret(node->m_leadingChildren.size() == 1);		
		//And now add a second child
		//static uv_err_t fromSubsequence(const UVDFLIRTSignatureRawSequence *seq, UVDFLIRTSignatureRawSequence::iterator pos, uint32_t n, UVDFLIRTSignatureTreeLeadingNode **out);
		uv_assert_err_ret(node->fromSubsequence(&node->m_bytes, nodeBranchPoint, UVDFLIRTSignatureRawSequence::npos, &newNode));
		uv_assert_ret(newNode);
		node->m_leadingChildren.insert(newNode);

		//This node now has the proper prefix and we can bucket it
		uv_assert_err_ret(node->m_crcNodes.insert(m_function));		
	}
	/*
	Case: our new node is longer
	This can happen because
		-our node is in total longer 
		-we need to find the appropriete sub-tree to add to
	In either case, we must check to see if we don't have an appropiete child to match to
	This may be most common as it will happen a lot if the tree is well built as nodes will be short from high branching
	Ex
		Node:  ABCA
		Our:   ABCA232ABC
			       /\
			        \Split
	*/
	else if( nodeBranchPoint == node->m_bytes.end() )
	{
		/*
		If any of hte children start with the same prefix, we are golden
		Otherwise, we must create a new node
		*/
	
		UVDFLIRTSignatureTreeLeadingNode *childNode = NULL;

		for( UVDFLIRTSignatureTreeLeadingNode::LeadingChildrenSet::iterator iter = node->m_leadingChildren.begin(); iter != node->m_leadingChildren.end(); ++iter )
		{
			UVDFLIRTSignatureTreeLeadingNode *cur = *iter;
			
			uv_assert_ret(cur);
			//First byte should match
			//We should be ga
			if( (*cur->m_bytes.begin()) == (*sequencePosition) )
			{
				childNode = cur;
				break;
			}
		}
		//No prefix match?  Branch here
		if( !childNode )
		{
			////static uv_err_t fromSubsequence(const UVDFLIRTSignatureRawSequence *seq, UVDFLIRTSignatureRawSequence::iterator pos, uint32_t n, UVDFLIRTSignatureTreeLeadingNode **out);
			uv_assert_err_ret(node->fromSubsequence(&m_leadingSequence, sequencePosition, UVDFLIRTSignatureRawSequence::npos, &childNode));
			uv_assert_ret(childNode);
			node->m_leadingChildren.insert(childNode);
		}
		//Alternativly we could recurse, but since we already know what needs to be done, why bother
		uv_assert_err_ret(insertSubseq(childNode, sequencePosition));
	}
	else
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
#endif
		
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureTreeLeadingNode::size(uint32_t *sizeOut)
{
	uv_assert_ret(sizeOut);
	
	for( UVDFLIRTSignatureTreeHashNodes::HashSet::iterator iter = m_crcNodes.m_nodes.begin(); iter != m_crcNodes.m_nodes.end(); ++iter )
	{
		UVDFLIRTSignatureTreeHashNode *hashNode = *iter;
		uint32_t size = 0;
		
		uv_assert_ret(hashNode);
		uv_assert_err_ret(hashNode->size(&size));
		*sizeOut += size;
	}

	for( LeadingChildrenSet::iterator iter = m_leadingChildren.begin(); iter != m_leadingChildren.end(); ++iter )
	{
		UVDFLIRTSignatureTreeLeadingNode *leadingNode = *iter;
		uint32_t size = 0;

		uv_assert_ret(leadingNode);
		uv_assert_err_ret(leadingNode->size(&size));
		*sizeOut += size;		
	}

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureTreeLeadingNode::dump(const std::string &prefixIn, bool includeDebug)
{
	/*
	Root
		0422:
			0) CRC: blah blah
		ABC123:
			
			12:
				0)
				1)
			AC:
				0)
	*/
	
	std::string prefix = prefixIn;
	
	if( includeDebug && !UVDGetDebugFlag(UVD_DEBUG_TYPE_FLIRT) )
	{
		return UV_ERR_OK;
	}

	//We should be holding something useful
	//eh we might use this to debug the tree while being built
	//uv_assert_ret(!m_crcNodes.empty() || !m_leadingChildren.empty());
	
	//Root node does not have any leading bytes, don't indent it and such
	if( !m_bytes.empty() )
	{
		printf("%s%s (0x%08X):\n", prefix.c_str(), m_bytes.toString().c_str(), (int)this);
		prefix += g_config->m_flirt.m_debugDumpTab;
	}
	
	//Display shorter fellows first
	uv_assert_err_ret(m_crcNodes.debugDump(prefix));
	
	//And now the longer fellows
	for( LeadingChildrenSet::iterator iter = m_leadingChildren.begin(); iter != m_leadingChildren.end(); ++iter )
	{
		UVDFLIRTSignatureTreeLeadingNode *node = *iter;
		
		uv_assert_ret(node);
		node->dump(prefix, includeDebug);
	}
	return UV_ERR_OK;
}

