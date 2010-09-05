/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_flirt_signature_tree.h"

bool UVDFLIRTSignatureTreeBasicNodeCompare::operator()(UVDFLIRTSignatureTreeBasicNode *first, UVDFLIRTSignatureTreeBasicNode *second) const
{
	if( first == second )
	{
		return true;
	}
	//True if first strictly less than second
	return first->compare(second) > 0;
}

/*
UVDFLIRTSignatureTreeBasicNode
*/

UVDFLIRTSignatureTreeBasicNode::UVDFLIRTSignatureTreeBasicNode()
{
	m_totalLength = 0;
	m_attributeFlags = 0;
}

uv_err_t UVDFLIRTSignatureTreeBasicNode::fromFunction(const UVDFLIRTFunction *function, UVDFLIRTSignatureTreeBasicNode **out)
{
	UVDFLIRTSignatureTreeBasicNode *ret = NULL;
	
	ret = new UVDFLIRTSignatureTreeBasicNode();
	uv_assert_ret(ret);
	
	ret->m_publicNames = function->m_publicNames;
	ret->m_references = function->m_references;
	ret->m_attributeFlags = function->m_attributeFlags;
	ret->m_totalLength = function->m_totalLength;
	
	uv_assert_ret(out);
	*out = ret;
	
	return UV_ERR_OK;
}

UVDFLIRTSignatureTreeBasicNode::~UVDFLIRTSignatureTreeBasicNode()
{
	//Although we have a bunch of junk, none of its allocated
}

uv_err_t UVDFLIRTSignatureTreeBasicNode::insertReference(UVDFLIRTSignatureReference reference)
{
	m_references.push_back(reference);
	return UV_ERR_OK;
}

int UVDFLIRTSignatureTreeBasicNode::compare(const UVDFLIRTSignatureTreeBasicNode *second)
{
	if( this == second )
	{
		return 0;
	}
	return m_totalLength - second->m_totalLength;
}

