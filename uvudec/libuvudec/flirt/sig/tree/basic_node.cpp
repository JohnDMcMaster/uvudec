/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "flirt/sig/tree/tree.h"

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

uv_err_t UVDFLIRTSignatureTreeBasicNode::debugDump(const std::string &prefix, uint32_t basicNodeIndex)
{
	printf("%s%d) attributes:0x%.8X totalLength:0x%.4X", prefix.c_str(), basicNodeIndex, m_attributeFlags, m_totalLength);

	//FIXME: this was implemented incorrectly in the core structure, fix print when its fixed
	for( std::vector<std::string>::iterator iter = m_publicNames.begin(); iter != m_publicNames.end(); ++iter )
	{
		std::string cur = *iter;
		
		printf(" :0000 %s", cur.c_str());
	}
	for( std::vector<UVDFLIRTSignatureReference>::iterator iter = m_references.begin(); iter != m_references.end(); ++iter )
	{
		//Hmm wonder if this is safe
		//const UVDFLIRTSignatureReference &reference = *iter;
		UVDFLIRTSignatureReference reference = *iter;
	
		//FIXME: add attribute flag printing nicer
		printf(" ^0x%.4X %s (0x%.8X)", reference.m_offset, reference.m_name.c_str(), reference.m_attributeFlags);
	}

	printf("\n");
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureTreeBasicNode::size(uint32_t *sizeOut)
{
	uv_assert_ret(sizeOut);
	*sizeOut = m_publicNames.size();
	return UV_ERR_OK;
}

