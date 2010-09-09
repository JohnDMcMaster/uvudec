/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "flirt/sig/tree/tree.h"
#include "uvd_config.h"
#include "uvd_util.h"
#include <limits.h>

bool UVDFLIRTSignatureTreeHashNodeCompare::operator()(UVDFLIRTSignatureTreeHashNode *first, UVDFLIRTSignatureTreeHashNode *second) const
{
	if( first == second )
	{
		return true;
	}
	//True if first strictly less than second
	return first->compare(second) > 0;
}

/*
UVDFLIRTSignatureTreeHashNode
*/

UVDFLIRTSignatureTreeHashNode::UVDFLIRTSignatureTreeHashNode()
{
	m_crc16 = 0;
	m_leadingLength = 0;
}

UVDFLIRTSignatureTreeHashNode::UVDFLIRTSignatureTreeHashNode(const UVDFLIRTFunction *function)
{
	m_crc16 = function->m_crc16;
	m_leadingLength = uvd_min(function->m_sequence.size(), g_config->m_flirt.m_patLeadingLength);
}

UVDFLIRTSignatureTreeHashNode::~UVDFLIRTSignatureTreeHashNode()
{
	for( BasicSet::iterator iter = m_bucket.begin(); iter != m_bucket.end(); ++iter )
	{
		delete *iter;
	}
}

uv_err_t UVDFLIRTSignatureTreeHashNode::insert(UVDFLIRTFunction *function)
{
	UVDFLIRTSignatureTreeBasicNode *node = NULL;
	
	uv_assert_err_ret(UVDFLIRTSignatureTreeBasicNode::fromFunction(function, &node)); 
	uv_assert_ret(node);
	uv_assert_ret(m_bucket.find(node) == m_bucket.end());
	m_bucket.insert(node);

	return UV_ERR_OK;
}

int UVDFLIRTSignatureTreeHashNode::compare(const UVDFLIRTSignatureTreeHashNode *r)
{
	if( this == r )
	{
		return 0;
	}
	if( this == NULL )
	{
		return INT_MIN;
	}
	if( r == NULL )
	{
		return INT_MAX;
	}
	
	//uint16_t m_crc16;
	//uint32_t m_leadingLength;

	//Special cases out of the way
	if( m_crc16 != r->m_crc16 ) 
	{
		return m_crc16 - r->m_crc16;
	}
	else
	{
		return m_leadingLength - r->m_leadingLength;
	}
}

uv_err_t UVDFLIRTSignatureTreeHashNode::debugDump(const std::string &prefix, uint32_t hashNodeIndex)
{
	uint32_t basicNodeIndex = 0;
	for( BasicSet::iterator iter = m_bucket.begin(); iter != m_bucket.end(); ++iter )
	{
		UVDFLIRTSignatureTreeBasicNode *basicNode = *iter;

		printf("%s%d) CRC16:0x%.4X leadingLength:0x%.2X\n", prefix.c_str(), hashNodeIndex, m_crc16, m_leadingLength);
		uv_assert_ret(basicNode);
		uv_assert_err_ret(basicNode->debugDump(prefix + g_config->m_flirt.m_debugDumpTab, basicNodeIndex));
		++basicNodeIndex;
	}
	return UV_ERR_OK;
}

/*
UVDFLIRTSignatureTreeHashNodes
*/
UVDFLIRTSignatureTreeHashNodes::UVDFLIRTSignatureTreeHashNodes()
{
}

UVDFLIRTSignatureTreeHashNodes::~UVDFLIRTSignatureTreeHashNodes()
{
}

uv_err_t UVDFLIRTSignatureTreeHashNodes::insert(UVDFLIRTFunction *function)
{
	UVDFLIRTSignatureTreeHashNode *hashNode = NULL;
	UVDFLIRTSignatureTreeHashNode lookingFor(function);

	if( m_nodes.find(&lookingFor) == m_nodes.end() )
	{
		hashNode = new UVDFLIRTSignatureTreeHashNode(function);
		uv_assert_ret(hashNode);
		m_nodes.insert(hashNode);
	}
	else
	{
		hashNode = *m_nodes.find(&lookingFor);
		uv_assert_ret(hashNode);
	}

	uv_assert_err_ret(hashNode->insert(function));

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureTreeHashNodes::debugDump(const std::string &prefix)
{
	uint32_t hashNodeIndex = 0;
	for( UVDFLIRTSignatureTreeHashNodes::HashSet::iterator iter = m_nodes.begin(); iter != m_nodes.end(); ++iter )
	{
		UVDFLIRTSignatureTreeHashNode *hashNode = *iter;
		
		uv_assert_ret(hashNode);
		uv_assert_err_ret(hashNode->debugDump(prefix, hashNodeIndex));
		++hashNodeIndex;
	}
	return UV_ERR_OK;
}

