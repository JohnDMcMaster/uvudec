/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/flirt/sig/reader.h"
#include "uvd/flirt/sig/sig.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

UVDFLIRTSigReader::UVDFLIRTSigReader(UVDFLIRTSignatureDB *db)
{
	m_db = db;
	m_file_pos = 0;	
	m_file_contents = NULL;
	m_cur_ptr = NULL;
	m_file_size = 0;
}

UVDFLIRTSigReader::~UVDFLIRTSigReader()
{
}

uv_err_t UVDFLIRTSigReader::preparse()
{
	FILE *file = NULL;   
	struct stat astat;

	file = fopen(m_file.c_str(), "rb");
	if( !file )
	{
		 printf_error("file %s not found\n", m_file.c_str());
		 return UV_DEBUG(UV_ERR_GENERAL);
	}

	if( stat(m_file.c_str(), &astat) )
	{
		 printf_error("no size\n");
		 return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	m_file_size = astat.st_size;
	m_file_contents = (char *)malloc(m_file_size);
	
	if( !m_file_contents )
	{
		 printf_error("alloc fail\n");
		 return UV_DEBUG(UV_ERR_GENERAL);
	}

	if( fread(m_file_contents, 1, m_file_size, file) != m_file_size )
	{
		 printf_error("bad file read\n");
		 return UV_DEBUG(UV_ERR_GENERAL);
	}
	m_cur_ptr = m_file_contents;

	fclose(file);

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSigReader::decompress()
{
	//website says something about InfoZIP algorithm
	printf_error("ZIP decompression not supported\n");
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDFLIRTSigReader::advance(int bytes)
{
	m_file_pos += bytes;
	m_cur_ptr += bytes;
	return UV_ERR_OK;
}

int UVDFLIRTSigReader::read_byte()
{
	uint8_t ret = *m_cur_ptr;
	advance(1);
	return ret;
}

int UVDFLIRTSigReader::read16()
{
	return (read_byte() << 8)
		+ read_byte();
}

int UVDFLIRTSigReader::bitshift_read()
{
	uint32_t first = read_byte();
	
	if( first & 0x80 )
		return ((first & 0x7F) << 8) + read_byte();
	return first;
}

int UVDFLIRTSigReader::read_relocation_bitmask()
{
	uint32_t first = 0;
	uint32_t lower = 0;
	uint32_t upper = 0;

	first = read_byte();
	printf_flirt_debug("first byte: 0x%02X\n", first);
	
	//No bit prefix
	//Max val 0x7F
	if( (first & 0x80) != 0x80 )
	{
		return first;
	}
	//0x80 bit prefix
	//Max val 0x7FFF
	else if( (first & 0xC0) != 0xC0 )
	{
		//0x7F trims off the 0x80 escape bit
		return ((first & 0x7F) << 8) + read_byte();
	}
	//0xC0 bit prefix
	//Max val 0x3FFFFFFF
	else if( (first & 0xE0) != 0xE0 )
	{
		upper = ((first & 0x3F) << 8) + read_byte();
		lower = read16();
	}
	//0xE0 bit prefix
	//Max val 0xFFFFFFFF
	//NOTE: 0xF0 etc seems reserved
	else
	{
		upper = read16();
		lower = read16();
	}
	printf_flirt_debug("upper: 0x%02X, lower: 0x%02X\n", upper, lower);
	uint32_t ret = lower + (upper << 16);
	return ret;
}

uv_err_t UVDFLIRTSigReader::load(const std::string &in)
{
	m_file = in;
	
	uv_assert_err_ret(preparse());
	uv_assert_err_ret(parse_header());

	if( m_db->m_header.feature_flags & UVD__IDASIG__FEATURE__COMPRESSED)
	{
		uv_assert_err_ret(decompress());
	}
	
	printf_flirt_debug("Root node at 0x%08X\n", m_file_pos);
	uv_assert_err_ret(parse_tree());
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSigReader::parse_header()
{
	memcpy(&m_db->m_header, m_cur_ptr, sizeof(m_db->m_header));
	uv_assert_err_ret(advance(sizeof(m_db->m_header)));

	if( memcmp(&m_db->m_header, "IDASGN", 6) )
	{
		printf_error("magic fail\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	/*
	if( m_header.version == 5)
		numberModules = oldNumberModules;
	Maybe some differences when using ver 6
	Ver 5 issues apply as well
	*/
	if( m_db->m_header.version != 7)
	{
		printf_error("version mismatch\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
		
	//printf("last (n_module) offest: 0x%08X\n", offsetof(struct UVD_IDA_sig_header_t, n_modules));
	if( sizeof(struct UVD_IDA_sig_header_t) != 0x29)
	{
		printf_error("UVD_IDA_sig_header_t wrong size: 0x%08X\n", sizeof(struct UVD_IDA_sig_header_t));
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	char library_name[256];
	memcpy(&library_name[0], m_cur_ptr, m_db->m_header.library_name_sz);
	library_name[m_db->m_header.library_name_sz] = 0;
	uv_assert_err_ret(advance(m_db->m_header.library_name_sz));
	m_db->m_libraryName = library_name;

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSigReader::parse_tree()
{
	uint16_t n_internal_nodes = bitshift_read();

	printf_flirt_debug("n_internal_nodes: 0x%04X\n", n_internal_nodes);
	//Internal node
	if( n_internal_nodes )
	{
		uint32_t relocation_bitmask;

		//Simply all of the ones with the same prefix
		for( int i = 0; i < n_internal_nodes; ++i )
		{
			uint32_t n_node_bytes = 0;
			uint32_t cur_relocation_bitmask = 0;
	
			n_node_bytes = read_byte();
			printf_flirt_debug("n_node_bytes: 0x%02X\n", n_node_bytes);
			//Only allowed 32 bytes
			if( n_node_bytes > 0x20u )
			{
				printf_error("Too many bytes, leading max 0x20, found 0x%02X\n", n_node_bytes);
				return UV_DEBUG(UV_ERR_GENERAL);
			}

			cur_relocation_bitmask = 1 << (n_node_bytes - 1);

			if( n_node_bytes >= 0x10 )
			{
				relocation_bitmask = read_relocation_bitmask();
			}
			else
			{
				relocation_bitmask = bitshift_read();
			}
			printf_flirt_debug("relocation_bitmask: 0x%08X\n", n_internal_nodes);

			//Relocations don't appear until the end
			std::string nextPatBytes;
			for( uint32_t j = 0; j < n_node_bytes; ++j )
			{
				if( cur_relocation_bitmask & relocation_bitmask )
				{
					nextPatBytes += "..";
				}
				else
				{
					char buff[3];
					
					snprintf(buff, sizeof(buff), "%02X", read_byte());
					nextPatBytes += buff;
				}
				cur_relocation_bitmask >>= 1;
			}
			
			uv_assert_err_ret(m_module.m_sequence.append(nextPatBytes));
			uv_assert_err_ret(parse_tree());
			uv_assert_err_ret(m_module.m_sequence.shorten(n_node_bytes));
		}
	}
	//Leaf node
	else
	{
		uint32_t read_flags;
		//Loop for each element with the same prefix, but possibly different crc16
		//Listed in increasing sorted order of crc16
		do
		{
			m_module.m_crc16Length = read_byte();
			m_module.m_crc16 = read16();
			
			//Loop for each bucketed signature
			//All in this loop have the same crc16 and same length
			//What may be different is what the relocation symbols are
			do
			{
				uint32_t ref_cur_offset = 0;
								
				m_module.m_totalLength = bitshift_read();
				m_module.m_publicNames.clear();
				m_module.m_references.clear();
				
				//Loop for each public name
				do
				{
					UVDFLIRTPublicName publicName;
					uint32_t delta = 0;
					
					delta = bitshift_read();
				
					read_flags = read_byte();
					uv_assert_err_ret(publicName.setLocal(read_flags < 0x20));
					
					//Read reference name
					for( int i = 0; ; ++i )
					{
						if( i >= 1024 )
						{
							printf_error("reference length exceeded\n");
							return UV_DEBUG(UV_ERR_GENERAL);
						}
					
						if( read_flags <= UVD__IDASIG__NAME__ESCAPE_MAX )
						{
							read_flags = read_byte();
						}
						if( read_flags <= UVD__IDASIG__NAME__ESCAPE_MAX )
						{
							break;
						}
				
						publicName.m_name += (char)read_flags;
						read_flags = 0;
					}
					ref_cur_offset += delta;
					uv_assert_err_ret(publicName.setOffset(ref_cur_offset));
					//printf_flirt_debug(" %04X:%s", ref_cur_offset, publicName.m_name.c_str());
					m_module.m_publicNames.push_back(publicName);
				} while( read_flags & UVD__IDASIG__NAME__MORE_NAMES );
				
				//FIXME: not sure what this is
				//Looks like some offset + value?
				if( read_flags & UVD__IDASIG__NAME__PAREN )
				{
					uint32_t first;
					uint32_t second;
				
					first = bitshift_read();
					second = read_byte();
					printf_flirt_debug("That unknown thing: (0x%04X: 0x%02X)\n", first, second);
					m_module.m_hasUnknowns = true;
					m_module.m_unknown0 = first;
					m_module.m_unknown1 = second;
				}
				else
				{
					m_module.m_hasUnknowns = false;
					//Why not, maybe better than stale value from previous
					m_module.m_unknown0 = 0;
					m_module.m_unknown1 = 0;
				}
				
				//Symbol linked references
				//This indicates we only preserve one of the references from the .pat file since there is no loop here
				if( read_flags & UVD__IDASIG__NAME__SYMBOL_LINKED_REFERENCE)
				{
					UVDFLIRTSignatureReference reference;
					uint32_t ref_name_len;
							
					reference.m_offset = bitshift_read();
					ref_name_len = read_byte();
					if( !ref_name_len)
						ref_name_len = bitshift_read();
					//FIXME: figure out how to convert read_flags to this
					//Do we just copy it over directly?
					reference.m_attributeFlags = 0x00;
					
					reference.m_name = std::string(m_cur_ptr, ref_name_len);
					//If last char is 0, we have a special flag set
					if( m_cur_ptr[ref_name_len - 1] == 0)
						reference.m_offset = -reference.m_offset;
					
					uv_assert_err_ret(advance(ref_name_len));
					m_module.m_references.push_back(reference);
				}
				//Ready to roll
				uv_assert_err_ret(m_db->insert(&m_module));
			} while( read_flags & UVD__IDASIG__NAME__MORE_BASIC );
		} while( read_flags & UVD__IDASIG__NAME__MORE_HASH );
	}
	
	return UV_ERR_OK;
}

