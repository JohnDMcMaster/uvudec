/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDOBJGB_OBJECT_H
#define UVDOBJGB_OBJECT_H

#include "uvd/object/object.h"
#include "uvd/util/types.h"
#include "uvd/data/data.h"
#include "uvd/object/object.h"

/*
A raw binary object as would be ripped off of a ROM (ex: EPROM)
*/
class UVDGBObject : public UVDObject
{
public:
	UVDGBObject();
	~UVDGBObject();

	virtual uv_err_t init(UVDData *data);

	//XXX: this should probably be a generic object function
	//Also, figure out how to resolve address space nicely
	uv_err_t getEntryPoint(uv_addr_t entryPoint);
	uv_err_t getStartupLogo(const UVDData **out);
	//out set to true if matches expected value
	uv_err_t isNintendoLogo(uvd_bool_t *out);
	uv_err_t getTitle(std::string &title);
	uv_err_t getManufacturerCode(uint32_t *out);
	uv_err_t getCGBFlags(uint8_t *out);
	uv_err_t getNewLicenseeCode(uint16_t *out);
	uv_err_t getSGBFlags(uint8_t *out);
	uv_err_t isSGBEnabled(uvd_bool_t *out);
	uv_err_t getCartridgeType(uint8_t *out);
	//Return in bytes
	uv_err_t getROMSize(uint32_t *out);
	//Raw byte value
	uv_err_t getROMSizeRaw(uint8_t *out);
	//Return in bytes
	uv_err_t getRAMSize(uint32_t *out);
	//Raw byte value
	uv_err_t getRAMSizeRaw(uint8_t *out);
	uv_err_t getDestinationCode(uint8_t *out);
	uv_err_t getOldLicenseeCode(uint8_t *out);
	uv_err_t getMaskROMVersioNumber(uint8_t *out);

	//From the actual ROM
	uv_err_t getHeaderChecksum(uint8_t *out);
	//Calculate what it "should" be
	uv_err_t computetHeaderChecksum(uint8_t *out);
	uv_err_t isHeaderChecksumValid(uvd_bool_t *out);
		
	//From the actual ROM
	uv_err_t getGlobalChecksum(uint16_t *out);
	//Calculate what it "should" be
	uv_err_t computeGlobalChecksum(uint16_t *out);
	uv_err_t isGlobalChecksumValid(uvd_bool_t *out);

	//Returns UV_ERR_NOTSUPPORTED if can't load
	static uv_err_t canLoad(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence,
			void *user);
	//How could this fail?
	//indicates we need a priorty system to load ELF files first etc
	static uv_err_t tryLoad(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out,
			void *user);

public:
	UVDData *m_nintendoLogo;
};

#endif

