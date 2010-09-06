/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_config.h"
#include "flirt/pat/pat.h"
#include "flirt/sig/tree/tree.h"
#include "uvd_util.h"
#include "flirt/flirt.h"
#include <limits.h>

/*
UVDFLIRTSignatureTreeLeadingNode
*/

bool UVDFLIRTSignatureLeadingNodeCompare::operator()(const UVDFLIRTSignatureTreeLeadingNode *first, const UVDFLIRTSignatureTreeLeadingNode *second) const
{
	if( first == second )
	{
		return true;
	}
	//True if first strictly less than second
	return first->m_bytes.compare(&second->m_bytes) > 0;
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
uv_err_t UVDFLIRTSignatureTreeLeadingNode::insertHashNode(UVDFLIRTFunction *function)
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
	
	//Create new child node
	child = new UVDFLIRTSignatureTreeLeadingNode();
	uv_assert_err_ret(m_bytes.subseqTo(&child->m_bytes, pos));
	uv_assert_err_ret(m_bytes.truncate(pos));
	
	//transfer children to it
	for( LeadingChildrenSet::iterator iter = m_leadingChildren.begin(); iter != m_leadingChildren.end(); ++iter )
	{
		UVDFLIRTSignatureTreeLeadingNode *ourChild = *iter;
		
		uv_assert_ret(ourChild);
		//uv_assert_err_ret(child->insert(ourChild));
		child->m_leadingChildren.insert(ourChild);
	}
	//Out with the old, in with the new
	m_leadingChildren.clear();
	//FIXME
	//uv_assert_err_ret(m_leadingChildren.insert(child));
	
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
	uv_err_t insert(UVDFLIRTSignatureTreeLeadingNode *rootNode, UVDFLIRTFunction *function);
	//Recursive case
	uv_err_t insertSubseq(UVDFLIRTSignatureTreeLeadingNode *node, UVDFLIRTSignatureRawSequence::const_iterator sequencePosition);

	//The sequence we are trying to insert
	UVDFLIRTSignatureRawSequence m_leadingSequence;
	//Function the sequencei s from
	//We'll need this to go to the lower levels
	UVDFLIRTFunction *m_function;
};

uv_err_t UVDFLIRTSignatureTreeLeadingNode::insert(UVDFLIRTFunction *function)
{
	UVDFLIRTSignatureTreeLeadingNodeInserter inserter;

	printf_flirt_debug("inserting function into db, size: %d, seq: %s\n", function->m_sequence.size(), function->m_sequence.toString().c_str());
	
	uv_assert_err_ret(inserter.insert(this, function));
	return UV_ERR_OK;
};

uv_err_t UVDFLIRTSignatureTreeLeadingNodeInserter::insert(UVDFLIRTSignatureTreeLeadingNode *rootNode, UVDFLIRTFunction *function)
{
	uint32_t leadingLength = 0;
	
	leadingLength = uvd_min(function->m_sequence.size(), g_config->m_flirt.m_patLeadingLength);	
	uv_assert_err_ret(function->m_sequence.subseqTo(&m_leadingSequence, function->m_sequence.const_begin(), leadingLength));
	
	m_function = function;

	//Before was considering walking this seq, maybe reimplement this if needed later
	return UV_DEBUG(insertSubseq(rootNode, m_leadingSequence.begin()));
}

uv_err_t UVDFLIRTSignatureTreeLeadingNodeInserter::insertSubseq(UVDFLIRTSignatureTreeLeadingNode *node, UVDFLIRTSignatureRawSequence::const_iterator sequencePosition)
{
	UVDFLIRTSignatureRawSequence::iterator thisBranchPoint;
	
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
	
	//Start by finding how much of the sequence we share
	//uv_err_t matchPosition(const UVDFLIRTSignatureRawSequence *other, const_iterator &otherStartEnd, const_iterator &thisMatchPoint) const;
	uv_assert_err_ret(node->m_bytes.matchPosition(&m_leadingSequence, sequencePosition, thisBranchPoint));
	
	
	/*
	A partial match
	Split and other craziness
	Recursive
	Case: our original node is longer
	Ex
		Node:  ABCA 232ABC
		Func:  ABCA
		           /\
		            \Split
	Ex
		Node:  ABCA 232ABC
		Func:  ABCA 993432
		           /\
		            \Split
	Split original
	*/
	if( thisBranchPoint != node->m_bytes.end() )
	{
		//UVDFLIRTSignatureTreeLeadingNode *newNode = NULL;
		
		uv_assert_err_ret(node->split(thisBranchPoint));
		//We should only have a single child now of our new node
		uv_assert_ret(node->m_leadingChildren.size() == 1);		
	}
	/*
	Ex
		Node:  ABCA
		Func:  ABCA 232ABC
		           /\
		            \Split
	Ex
		Note: this case is now unimportant since above split if necessary
			Watch out for use of thisBranchPoint though which is now invalid
		Node:  ABCA 232ABC
		Func:  ABCA 993432
		           /\
		            \Split
	*/
	if( sequencePosition != m_leadingSequence.end() )
	{
		UVDFLIRTSignatureTreeLeadingNode *childNode = NULL;

		//No prefix match?  Branch here
		////static uv_err_t fromSubsequence(const UVDFLIRTSignatureRawSequence *seq, UVDFLIRTSignatureRawSequence::iterator pos, uint32_t n, UVDFLIRTSignatureTreeLeadingNode **out);
		uv_assert_err_ret(node->fromSubsequence(&m_leadingSequence, sequencePosition, UVDFLIRTSignatureRawSequence::npos, &childNode));
		uv_assert_ret(childNode);
		node->m_leadingChildren.insert(childNode);

		//Alternativly we could recurse, but since we already know what needs to be done, why bother
		uv_assert_err_ret(childNode->m_crcNodes.insert(m_function));
		//uv_assert_err_ret(insertSubseq(childNode, sequencePosition));
	}
	/*
	Full tail match?
	This is the (most common) terminal case
	Ex
		Node:  ABCA232ABC
		Our:   ABCA232ABC
	This also will happen if we split our current node because the seq in was smaller
		See above
		Ex
			Node:  ABCA 232ABC
			Func:  ABCA
	*/
	//if( thisBranchPoint == node->m_bytes.end() && sequencePosition == m_leadingSequence.end() )
	else
	{
		//Add to our bucket
		uv_assert_err_ret(node->m_crcNodes.insert(m_function));
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
		
		uv_assert_err_ret(node->split(thisBranchPoint));
		//We should only have a single child now of our new node
		uv_assert_ret(node->m_leadingChildren.size() == 1);		
		//And now add a second child
		//static uv_err_t fromSubsequence(const UVDFLIRTSignatureRawSequence *seq, UVDFLIRTSignatureRawSequence::iterator pos, uint32_t n, UVDFLIRTSignatureTreeLeadingNode **out);
		uv_assert_err_ret(node->fromSubsequence(&node->m_bytes, thisBranchPoint, UVDFLIRTSignatureRawSequence::npos, &newNode));
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
	else if( thisBranchPoint == node->m_bytes.end() )
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

uv_err_t UVDFLIRTSignatureTreeLeadingNode::size(uint32_t *size)
{
	return UV_ERR_OK;
}

