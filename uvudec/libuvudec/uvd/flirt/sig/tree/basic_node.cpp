/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/flirt/sig/tree/tree.h"
#include "uvd/util/util.h"

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

uv_err_t UVDFLIRTSignatureTreeBasicNode::fromModule(const UVDFLIRTModule *module, UVDFLIRTSignatureTreeBasicNode **out)
{
	UVDFLIRTSignatureTreeBasicNode *ret = NULL;
	
	ret = new UVDFLIRTSignatureTreeBasicNode();
	uv_assert_ret(ret);
	printf_flirt_debug("allocated new basic node with address 0x%08X\n", (int)ret);
	
	ret->m_publicNames = module->m_publicNames;
	ret->m_references = module->m_references;
	ret->m_attributeFlags = module->m_attributeFlags;
	ret->m_totalLength = module->m_totalLength;
	
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
	/*
	We could just do pointer based sorting, but I'd like output to be deterministic
	Also, later we might find one order or another is required
	*/
	
	uint32_t delta = 0;
	
	if( this == second )
	{
		return 0;
	}
	if( this == NULL )
	{
		return INT_MIN;
	}
	if( second == NULL )
	{
		return INT_MAX;
	}
	
	//Total length sounds like a good primary measure
	delta = m_totalLength - second->m_totalLength;
	if( delta )
	{
		return delta;
	}
	
	//And then I have no idea whats important after that
	//This is just being used for hash map uniqueness, so its probably somewhat arbitrary
	
	//Try this
	delta = m_attributeFlags - second->m_attributeFlags;
	if( delta )
	{
		return delta;
	}
	
	//Hmm okay how about public name related
	delta = m_publicNames.size() - second->m_publicNames.size();
	if( delta )
	{
		return delta;
	}
	std::vector<UVDFLIRTPublicName>::const_iterator publicNameIterThis = m_publicNames.begin();
	std::vector<UVDFLIRTPublicName>::const_iterator publicNameIterSecond = second->m_publicNames.begin();
	for( ;; )
	{
		if( publicNameIterThis == m_publicNames.end() || publicNameIterSecond == second->m_publicNames.end() )
		{
			break;
		}
	
		UVDFLIRTPublicName thisPublicName = *publicNameIterThis;
		UVDFLIRTPublicName secondPublicName = *publicNameIterSecond;
		delta = thisPublicName.compare(&secondPublicName);
		if( delta )
		{
			return delta;
		}
		++publicNameIterThis;
		++publicNameIterSecond;
	}
	
	//Okay the referneces better tell it like it is
	delta = m_references.size() - second->m_references.size();
	if( delta )
	{
		return delta;
	}
	std::vector<UVDFLIRTSignatureReference>::const_iterator signatureReferenceIterThis = m_references.begin();
	std::vector<UVDFLIRTSignatureReference>::const_iterator signatureReferenceIterSecond = second->m_references.begin();
	for( ;; )
	{
		if( signatureReferenceIterThis == m_references.end() || signatureReferenceIterSecond == second->m_references.end() )
		{
			break;
		}
	
		delta = (*signatureReferenceIterThis).compare(&(*signatureReferenceIterSecond));
		if( delta )
		{
			return delta;
		}
		++signatureReferenceIterThis;
		++signatureReferenceIterSecond;
	}

	//Ran out of ammo...but we tried pretty hard
	return 0;
}

std::string UVDFLIRTSignatureTreeBasicNode::debugString()
{
	std::string ret;
	
	ret += UVDSprintf("attributes:0x%08X totalLength:0x%04X", m_attributeFlags, m_totalLength);

	for( std::vector<UVDFLIRTPublicName>::iterator iter = m_publicNames.begin(); iter != m_publicNames.end(); ++iter )
	{
		const UVDFLIRTPublicName &cur = *iter;
		
		ret += " ";
		ret += cur.toString();
	}
	for( std::vector<UVDFLIRTSignatureReference>::iterator iter = m_references.begin(); iter != m_references.end(); ++iter )
	{
		//Hmm wonder if this is safe
		//const UVDFLIRTSignatureReference &reference = *iter;
		UVDFLIRTSignatureReference reference = *iter;
	
		//FIXME: add attribute flag printing nicer
		ret += UVDSprintf(" ^0x%04X %s", reference.m_offset, reference.m_name.c_str());
		if( reference.m_attributeFlags )
		{
			ret += UVDSprintf("(0x%08X)", reference.m_attributeFlags);
		}
	}

	return ret;
}

uv_err_t UVDFLIRTSignatureTreeBasicNode::debugDump(const std::string &prefix, uint32_t basicNodeIndex)
{
	std::string line = debugString();
	printf("%s%d) %s\n", prefix.c_str(), basicNodeIndex, line.c_str());

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureTreeBasicNode::size(uint32_t *sizeOut)
{
	uv_assert_ret(sizeOut);
	*sizeOut = m_publicNames.size();
	return UV_ERR_OK;
}

