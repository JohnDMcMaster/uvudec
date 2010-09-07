/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "flirt/flirt.h"
#include "flirt/function.h"
#include "flirt/sig/sig.h"
#include "flirt/sig/tree/tree.h"
#include "flirt/sig/writer.h"
#include <string.h>

UVDFLIRTSignatureDBWriter::UVDFLIRTSignatureDBWriter(UVDFLIRTSignatureDB *db)
{
	m_db = db;
}

UVDFLIRTSignatureDBWriter::~UVDFLIRTSignatureDBWriter()
{
}

uv_err_t UVDFLIRTSignatureDBWriter::bitshiftAppend(uint32_t data)
{
	/*
	int bitshift_read()
		uint32_t first = read_byte();
	
		if ( first & 0x80)
			return ((first & 0x7F) << 8) + read_byte();
		return first;
	*/
	uint8_t byte = 0;

	//15 bit limit, MSB is ignored for escape
	uv_assert_ret(data <= 0x7FFF);

	if( data >= 0x80 )
	{
		//First byte is the high byte
		byte = 0x80 | ((data & 0xFF00) >> 8);
		uv_assert_err_ret(m_tree.appendByte(byte));
	}
	else
	{
		byte = data & 0xFF;
		uv_assert_err_ret(m_tree.appendByte(byte));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::uint8Append(uint8_t in)
{
	uv_assert_err_ret(m_tree.appendByte(in));
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::uint16Append(uint16_t in)
{
	uv_assert_err_ret(m_tree.appendByte(in & 0xFF));
	uv_assert_err_ret(m_tree.appendByte((in & 0xFF00) >> 8));
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::updateHeader()
{
	/*
	char magic[6];
	uint8_t version;
	uint8_t processor;
	uint32_t file_types;
	uint16_t OS_types;
	uint16_t app_types;
	uint8_t feature_flags;
	char pad;
	uint16_t old_number_modules;
	uint16_t crc16;
	char ctype[0x22 - 0x16];
	uint8_t library_name_sz;
	uint16_t alt_ctype_crc;
	uint32_t n_modules;
	*/
	
	memcpy(&m_header.magic, UVD__IDASIG__MAGIC, UVD__IDASIG__MAGIC_SIZE);
	//Since thats the only published version
	m_header.version = 7;
	//FIXME: set to proper arch
	m_header.processor = UVD__IDASIG__ARCH__80X86;
	//FIXME: set to proper type
	m_header.file_types = UVD__IDASIG__FILE__PE;
	//FIXME: set to OS
	m_header.OS_types = UVD__IDASIG__OS__WIN;
	//FIXME: set to app type
	m_header.app_types = UVD__IDASIG__APP__CONSOLE | UVD__IDASIG__APP__GRAPHICS | UVD__IDASIG__APP__EXE | UVD__IDASIG__APP__DLL | UVD__IDASIG__APP__DRV | UVD__IDASIG__APP__SINGLE_THREADED | UVD__IDASIG__APP__MULTI_THREADED | UVD__IDASIG__APP__32_BIT;
	//FIXME: see about implementing compression
	///and find out what the other feature flags mean
	m_header.feature_flags = 0;
	//Set to 0 in test file
	m_header.pad = 0x00;

	uint32_t numberFunctions = 0;
	uv_assert_err_ret(m_db->size(&numberFunctions));
	m_header.old_number_modules = numberFunctions;
	m_header.n_modules = numberFunctions;

	//FIXME: this needs to be set
	//Eh I actually don't know what this is calculated from or why its there
	//A file integrity checksum on the tree maybe?
	m_header.crc16 = 0;
	
	//FIXME: what does this mean?
	//Looks like its suppose to be null terminated
	strcpy(&m_header.ctype[0], "ctp_");
	
	//Not null terminated
	m_header.library_name_sz = m_db->m_libraryName.size();

	//Unknown meaning, set to 0
	m_header.alt_ctype_crc = 0;
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::updateForWrite()
{
	//Header is allocated in the writter
	
	//Create the sig table
	//Internal node table?
	
	printf_flirt_debug("updateForWrite()\n");

	uv_assert_err_ret(updateHeader());
	
	return UV_ERR_OK;
}

//This is probably just the 32 bit write function, but the only 32 bit var was the mask
uv_err_t UVDFLIRTSignatureDBWriter::constructRelocationBitmask(uint32_t nNodeBytes, uint32_t relocationBitmask)
{
	if( nNodeBytes >= 0x10 )
	{
		/*
		//relocation_bitmask = read_relocation_bitmask();
		uint32_t first;
		uint32_t lower;
		uint32_t upper;

		first = read_byte();

		if ((first & 0x80) != 0x80)
			return first;

		if ((first & 0xC0) != 0xC0)
			return ((first & 0x7F) << 8) + read_byte();

		if ((first & 0xE0) != 0xE0) {
			upper = ((first & 0xFF3F) << 8) + read_byte();
			lower = read16();
		} else {
			upper = read16();
			lower = read16();
		}
		uint32_t ret = lower + (upper << 16);
		return ret;
		*/
		
		//Strictly speaking, we could be lazy and just use all 32 bit relocation masks
		if( relocationBitmask < 0x80 )
		{
			uv_assert_err_ret(uint8Append(relocationBitmask));
		}
		else if( relocationBitmask < 0x8000 )
		{
			uint8_t byte = 0;
		
			byte = ((relocationBitmask >> 8) & 0x7F) | 0x80;
			uv_assert_err_ret(uint8Append(byte));
			uv_assert_err_ret(uint8Append(relocationBitmask & 0xFF));
		}
		else if( relocationBitmask < 0x800000 )
		{
			uint8_t byte = 0;
			
			byte = ((relocationBitmask >> 16) & 0x7F) | 0xC0;
			uv_assert_err_ret(uint8Append(byte));
			byte = (relocationBitmask >> 8) & 0xFF;
			uv_assert_err_ret(uint8Append(byte));
			byte = relocationBitmask & 0xFFFF;
			uv_assert_err_ret(uint16Append(byte));
		}
		else
		{
			uv_assert_err_ret(uint16Append((relocationBitmask >> 16) | 0xE0));
			uv_assert_err_ret(uint16Append(relocationBitmask & 0xFFFF));
		}
	}
	else
	{
		//relocation_bitmask = bitshift_read();
		uv_assert_err_ret(bitshiftAppend(relocationBitmask));
	}
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::getRelocationBitmask(UVDFLIRTSignatureTreeLeadingNode *node, uint32_t *out)
{
	/*
	First byte shoudl be MSB
	*/
	
	uint32_t cur = 0;
	uint32_t ret = 0;
	
	cur = 1;
	for( UVDFLIRTSignatureRawSequence::iterator iter = node->m_bytes.begin(); iter != node->m_bytes.end(); )
	{
		if( !(*iter).m_isReloc )
		{
			uv_assert_err_ret(uint8Append((*iter).m_byte));
		}
		uv_assert_err_ret(iter.next());
	}
	
	uv_assert_ret(out);
	*out = ret;

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::constructLeadingNodeItem(UVDFLIRTSignatureTreeLeadingNode *node)
{
	uint32_t nNodeBytes = 0;
	uint32_t relocationBitmask = 0;

	uv_assert_ret(node);
	nNodeBytes = node->m_bytes.size();
	
	uv_assert_ret(nNodeBytes <= UVD_FLIRT_SIG_LEADING_LENGTH);
	//n_node_bytes = read_byte();
	uv_assert_err_ret(uint8Append(nNodeBytes));
	
	uv_assert_err_ret(getRelocationBitmask(node, &relocationBitmask));
	uv_assert_err_ret(constructRelocationBitmask(nNodeBytes, relocationBitmask));
	
	//Dump all of the non-relocation bytes
	for( UVDFLIRTSignatureRawSequence::iterator iter = node->m_bytes.begin(); iter != node->m_bytes.end(); )
	{
		if( !(*iter).m_isReloc )
		{
			uv_assert_err_ret(uint8Append((*iter).m_byte));
		}
		uv_assert_err_ret(iter.next());
	}
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::constructLeadingNodeRecurse(UVDFLIRTSignatureTreeLeadingNode *node)
{
	uint32_t nLeadingChildren = 0;

	nLeadingChildren = node->m_leadingChildren.size();
	printf_flirt_debug("nLeadingChildren: %d\n", nLeadingChildren);
	if( nLeadingChildren )
	{
		uv_assert_err_ret(bitshiftAppend(nLeadingChildren));

		//for (int i = 0; i < n_internal_nodes; ++i) {
		for( UVDFLIRTSignatureTreeLeadingNode::LeadingChildrenSet::iterator iter = node->m_leadingChildren.begin(); iter != node->m_leadingChildren.end(); ++iter )
		{
			UVDFLIRTSignatureTreeLeadingNode *cur = *iter;
			uv_assert_err_ret(constructLeadingNodeItem(cur));
			uv_assert_err_ret(constructLeadingNode(cur));
		}
	}

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::constructCRCNode(UVDFLIRTSignatureTreeLeadingNode *node)
{
	if( node->m_crcNodes.m_nodes.empty() )
	{
		return UV_ERR_OK;
	}
	uv_assert_err_ret(bitshiftAppend(0));

	for( UVDFLIRTSignatureTreeHashNodes::HashSet::iterator hashIter = node->m_crcNodes.m_nodes.begin(); hashIter != node->m_crcNodes.m_nodes.end(); ++hashIter )
	{
		UVDFLIRTSignatureTreeHashNode *hashNode = *hashIter;
		/*
		uint32_t tree_block_len = read_byte();
		uint32_t a_crc16 = read16();
		*/
		uv_assert_ret(hashNode);
		uv_assert_err_ret(uint8Append(hashNode->m_leadingLength));
		uv_assert_err_ret(uint16Append(hashNode->m_crc16));
		for( UVDFLIRTSignatureTreeHashNode::BasicSet::iterator basicIter = hashNode->m_bucket.begin(); basicIter != hashNode->m_bucket.end(); ++basicIter )
		{
			UVDFLIRTSignatureTreeBasicNode *basicNode = *basicIter;
			
			uv_assert_ret(basicNode);
			//total_len = bitshift_read();
			uv_assert_err_ret(bitshiftAppend(basicNode->m_totalLength));

			//Multiple public names are rare
			//This may or may not be correct
			//Public names are offset 0
			for( std::vector<std::string>::iterator nameIter = basicNode->m_publicNames.begin(); nameIter != basicNode->m_publicNames.end(); ++nameIter )
			{
				std::string name = *nameIter;
				uint32_t flags = 0;

				//Public names are at offset 0
				uv_assert_err_ret(bitshiftAppend(0));
				
				//Write string
				for( std::string::iterator iterName = name.begin(); iterName != name.end(); ++iterName )
				{
					char c = *iterName;
					
					//Chars less than 0x20 (space) need to be escaped
					if( c <= UVD__IDASIG__NAME__ESCAPE_MAX )
					{
						//Somewhat arbitrary which char we use I guess
						uv_assert_err_ret(uint8Append(c));
					}
					uv_assert_err_ret(uint8Append(c));
				}
				
				//String terminator should include next flags
				//Note these flags are all less than 0x20
				/*
				#define UVD__IDASIG__NAME__MORE_NAMES					0x01
				#define UVD__IDASIG__NAME__MORE_BASIC					0x08
				#define UVD__IDASIG__NAME__MORE_HASH					0x10
				*/
				if( (nameIter + 1) != basicNode->m_publicNames.end() )
				{
					flags |= UVD__IDASIG__NAME__MORE_NAMES;
				}
				UVDFLIRTSignatureTreeHashNode::BasicSet::iterator basicIterNext = basicIter;
				++basicIterNext;
				if( basicIterNext != hashNode->m_bucket.end() )
				{
					flags |= UVD__IDASIG__NAME__MORE_BASIC;
				}
				UVDFLIRTSignatureTreeHashNodes::HashSet::iterator hashIterNext = hashIter;
				++hashIterNext;
				if( hashIterNext != node->m_crcNodes.m_nodes.end() )
				{
					flags |= UVD__IDASIG__NAME__MORE_HASH;
				}
				uv_assert_err_ret(uint8Append(flags));				
			}
		}
	}

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::constructLeadingNode(UVDFLIRTSignatureTreeLeadingNode *node)
{
	//Leading prefixes should be resolved first, but we also print shorter nodes first
	
	//If we have internal leading nodes, we write the number of nodes
	//If we have leaf leading nodes, we write 0

	//Shortest should go first, so dump children first if they exist
	uv_assert_err_ret(constructCRCNode(node));
	uv_assert_err_ret(constructLeadingNodeRecurse(node));

	UVDFLIRTSignatureRawSequence m_bytes;
		
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::constructTree()
{
	//This seems to have been designed for easy iterative one pass write
	
	//Its gotta start somewhere
	printf_flirt_debug("Constructing tree w/ root node children: %d\n", m_db->m_tree->m_leadingChildren.size());
	uv_assert_err_ret(constructLeadingNode(m_db->m_tree));
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::construct()
{	
	printf_flirt_debug("construct()\n");

	uv_assert_err_ret(addRelocatableData((char *)&m_header, sizeof(m_header)));
	uv_assert_err_ret(constructTree());
	uv_assert_err_ret(addRelocatableData(&m_tree));

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBWriter::applyRelocations()
{
	printf_flirt_debug("applyRelocations()\n");

	//uv_assert_err_ret(applyHeaderRelocations());
	return UV_ERR_OK;
}

