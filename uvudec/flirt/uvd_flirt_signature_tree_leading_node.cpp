/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_flirt_pattern.h"
#include "uvd_flirt_signature_tree.h"
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

uv_err_t UVDFLIRTSignatureTreeLeadingNode::insert(UVDFLIRTFunction *function)
{
	//Before was considering walking this seq, maybe reimplement this if needed later
	return UV_DEBUG(insertSubseq(function, function->m_leadingSequence.const_begin()));
}

uv_err_t UVDFLIRTSignatureTreeLeadingNode::insertSubseq(UVDFLIRTFunction *function, UVDFLIRTSignatureRawSequence::const_iterator sequencePosition)
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
	uv_assert_err_ret(m_bytes.matchPosition(&function->m_leadingSequence, sequencePosition, thisBranchPoint));
	
	/*
	Full tail match?
	This is the (most common) terminal case
	Ex
		Node:  ABCA232ABC
		Our:   ABCA232ABC
	*/
	if( thisBranchPoint == m_bytes.end() && sequencePosition == function->m_leadingSequence.end() )
	{
		//Add to our bucket
		uv_assert_err_ret(m_crcNodes.insert(function));
	}
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
	else if( sequencePosition == function->m_leadingSequence.end() )
	{
		UVDFLIRTSignatureTreeLeadingNode *newNode = NULL;
		
		uv_assert_err_ret(split(thisBranchPoint));
		//We should only have a single child now of our new node
		uv_assert_ret(m_leadingChildren.size() == 1);		
		//And now add a second child
		//static uv_err_t fromSubsequence(const UVDFLIRTSignatureRawSequence *seq, UVDFLIRTSignatureRawSequence::iterator pos, uint32_t n, UVDFLIRTSignatureTreeLeadingNode **out);
		uv_assert_err_ret(fromSubsequence(&m_bytes, thisBranchPoint, UVDFLIRTSignatureRawSequence::npos, &newNode));
		uv_assert_ret(newNode);
		m_leadingChildren.insert(newNode);

		//This node now has the proper prefix and we can bucket it
		uv_assert_err_ret(m_crcNodes.insert(function));		
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
	else if( thisBranchPoint == m_bytes.end() )
	{
		/*
		If any of hte children start with the same prefix, we are golden
		Otherwise, we must create a new node
		*/
	
		UVDFLIRTSignatureTreeLeadingNode *childNode = NULL;

		for( LeadingChildrenSet::iterator iter = m_leadingChildren.begin(); iter != m_leadingChildren.end(); ++iter )
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
			uv_assert_err_ret(fromSubsequence(&function->m_leadingSequence, sequencePosition, UVDFLIRTSignatureRawSequence::npos, &childNode));
			uv_assert_ret(childNode);
			m_leadingChildren.insert(childNode);
		}
		//Alternativly we could recurse, but since we already know what needs to be done, why bother
		uv_assert_err_ret(childNode->insertSubseq(function, sequencePosition));
	}
	//We got lazy and didn't go to sequence endings
	else
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
		
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureTreeLeadingNode::size(uint32_t *size)
{
	return UV_ERR_OK;
}

