/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_SIGNATURE_WRITER_H

#include "uvd_relocation_writer.h"
#include "flirt/sig/format.h"

class UVDFLIRTSignatureDB;
class UVDFLIRTSignatureDBWriter : public UVDRelocationWriter
{
public:
	UVDFLIRTSignatureDBWriter(UVDFLIRTSignatureDB *db);
	~UVDFLIRTSignatureDBWriter();
	
	//Phase 1
	uv_err_t updateHeader();
	uv_err_t updateForWrite();

	//Phase 2
	uv_err_t constructLeadingNode(UVDFLIRTSignatureTreeLeadingNode *node);
	uv_err_t constructTree();
	uv_err_t construct();

	//Phase 3
	uv_err_t constructLeadingNodeItem(UVDFLIRTSignatureTreeLeadingNode *node);
	uv_err_t constructLeadingNodeRecurse(UVDFLIRTSignatureTreeLeadingNode *node);
	uv_err_t constructCRCNode(UVDFLIRTSignatureTreeLeadingNode *node);
	uv_err_t constructRelocationBitmask(uint32_t nNodeBytes, uint32_t relocationBitmask);
	uv_err_t getRelocationBitmask(UVDFLIRTSignatureTreeLeadingNode *node, uint32_t *out);
	uv_err_t applyRelocations();

protected:
	uv_err_t bitshiftAppend(uint32_t data);
	uv_err_t uint8Append(uint8_t in);
	uv_err_t uint16Append(uint16_t in);

public:
	UVDFLIRTSignatureDB *m_db;

	/*
	struct UVD_IDA_sig_header_t
	{
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
	}  __attribute__((__packed__));
	*/	
	struct UVD_IDA_sig_header_t m_header;
	//use _db->m_libraryName
	//std::string m_libraryName;
	UVDBufferedDataMemory m_tree;
};

#endif

