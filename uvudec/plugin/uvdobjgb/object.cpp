/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
References:
http://www.zophar.net/fileuploads/2/10597teazh/gbrom.txt
http://nocash.emubase.de/pandocs.htm#thecartridgeheader
*/

#include "uvdobjgb/object.h"

UVDGBObject::UVDGBObject()
{
}

UVDGBObject::~UVDGBObject()
{
}

uv_err_t UVDGBObject::init(UVDData *data)
{
	uv_assert_err_ret(UVDObject::init(data));
	//We have a single section, a raw binary blob

	/*
	UVDSection *section = NULL;

	section = new UVDSection();
	uv_assert_ret(section);
	section->m_data = data;
	
	//Basic assumptions for a ROM image.  W is probably most debatable as we could be in flash or ROM
	section->m_R = UVD_TRI_TRUE;
	section->m_W = UVD_TRI_FALSE;
	section->m_X = UVD_TRI_TRUE;
	
	m_sections.push_back(section);
	*/
	
	return UV_ERR_OK;
}

//XXX: this should probably be a generic object function
//Also, figure out how to resolve address space nicely
uv_err_t UVDGBObject::getEntryPoint(uv_addr_t entryPoint)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
0104-0133 - Nintendo Logo
These bytes define the bitmap of the Nintendo logo that is displayed when
the gameboy gets turned on. The hexdump of this bitmap is:
	CE ED 66 66 CC 0D 00 0B 03 73 00 83 00 0C 00 0D
	00 08 11 1F 88 89 00 0E DC CC 6E E6 DD DD D9 99
	BB BB 67 63 6E 0E EC CC DD DC 99 9F BB B9 33 3E

We retain ownership of the data
Data is only valid as long as this object is valid
*/
uv_err_t UVDGBObject::getStartupLogo(const UVDData **out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

//out set to true if matches expected value
uv_err_t UVDGBObject::isNintendoLogo(uvd_bool_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

//"0134-0143 - Title"
uv_err_t UVDGBObject::getTitle(std::string &title)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
"013F-0142 - Manufacturer Code
In older cartridges this area has been part of the Title (see above), in
newer cartridges this area contains an 4 character uppercase manufacturer
code. Purpose and Deeper Meaning unknown."
*/
uv_err_t UVDGBObject::getManufacturerCode(uint32_t *code)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
"0143 - CGB Flag
In older cartridges this byte has been part of the Title (see above). 
In CGB cartridges the upper bit is used to enable CGB functions. 
This is required, otherwise the CGB switches itself into Non-CGB-Mode. 
Typical values are:
	80h - Game supports CGB functions, but works on old gameboys also.
	C0h - Game works on CGB only (physically the same as 80h).
Values with Bit 7 set, and either Bit 2 or 3 set, will switch the gameboy
into a special non-CGB-mode with uninitialized palettes. Purpose unknown,
eventually this has been supposed to be used to colorize monochrome games
that include fixed palette data at a special location in ROM.
*/
uv_err_t UVDGBObject::getCGBFlags(uint8_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
0144-0145 - New Licensee Code
Specifies a two character ASCII licensee code, indicating the company or
publisher of the game. These two bytes are used in newer games only (games
that have been released after the SGB has been invented). Older games are
using the header entry at 014B instead.
*/
uv_err_t UVDGBObject::getNewLicenseeCode(uint16_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
0146 - SGB Flag
Specifies whether the game supports SGB functions, common values are:
	00h = No SGB functions (Normal Gameboy or CGB only game)
	03h = Game supports SGB functions
The SGB disables its SGB functions if this byte is set to another value
than 03h.
*/
uv_err_t UVDGBObject::getSGBFlags(uint8_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDGBObject::isSGBEnabled(uvd_bool_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
0147 - Cartridge Type
Specifies which Memory Bank Controller (if any) is used in the cartridge,
and if further external hardware exists in the cartridge.

00h  ROM ONLY                 13h  MBC3+RAM+BATTERY
01h  MBC1                     15h  MBC4
02h  MBC1+RAM                 16h  MBC4+RAM
03h  MBC1+RAM+BATTERY         17h  MBC4+RAM+BATTERY
05h  MBC2                     19h  MBC5
06h  MBC2+BATTERY             1Ah  MBC5+RAM
08h  ROM+RAM                  1Bh  MBC5+RAM+BATTERY
09h  ROM+RAM+BATTERY          1Ch  MBC5+RUMBLE
0Bh  MMM01                    1Dh  MBC5+RUMBLE+RAM
0Ch  MMM01+RAM                1Eh  MBC5+RUMBLE+RAM+BATTERY
0Dh  MMM01+RAM+BATTERY        FCh  POCKET CAMERA
0Fh  MBC3+TIMER+BATTERY       FDh  BANDAI TAMA5
10h  MBC3+TIMER+RAM+BATTERY   FEh  HuC3
11h  MBC3                     FFh  HuC1+RAM+BATTERY
12h  MBC3+RAM
*/
uv_err_t UVDGBObject::getCartridgeType(uint8_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
0148 - ROM Size
Specifies the ROM Size of the cartridge. Typically calculated as
"32KB shl N".

00h -  32KByte (no ROM banking)
01h -  64KByte (4 banks)
02h - 128KByte (8 banks)
03h - 256KByte (16 banks)
04h - 512KByte (32 banks)
05h -   1MByte (64 banks)  - only 63 banks used by MBC1
06h -   2MByte (128 banks) - only 125 banks used by MBC1
07h -   4MByte (256 banks)
52h - 1.1MByte (72 banks)
53h - 1.2MByte (80 banks)
54h - 1.5MByte (96 banks)
*/
//Return in bytes
uv_err_t UVDGBObject::getROMSize(uint32_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

//Raw byte value
uv_err_t UVDGBObject::getROMSizeRaw(uint8_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
0149 - RAM Size
Specifies the size of the external RAM in the cartridge (if any).

00h - None
01h - 2 KBytes
02h - 8 Kbytes
03h - 32 KBytes (4 banks of 8KBytes each)

When using a MBC2 chip 00h must be specified in this entry, even though
the MBC2 includes a built-in RAM of 512 x 4 bits.
*/
//Return in bytes
uv_err_t UVDGBObject::getRAMSize(uint32_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

//Raw byte value
uv_err_t UVDGBObject::getRAMSizeRaw(uint8_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
014A - Destination Code
Specifies if this version of the game is supposed to be sold in japan, or
anywhere else. Only two values are defined.

00h - Japanese
01h - Non-Japanese
*/
uv_err_t UVDGBObject::getDestinationCode(uint8_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
014B - Old Licensee Code
Specifies the games company/publisher code in range 00-FFh. A value of 33h
signalizes that the New License Code in header bytes 0144-0145 is used instead.
(Super GameBoy functions won't work if <> $33.)
*/
uv_err_t UVDGBObject::getOldLicenseeCode(uint8_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
014C - Mask ROM Version number
Specifies the version number of the game. That is usually 00h.
*/
uv_err_t UVDGBObject::getMaskROMVersioNumber(uint8_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
014D - Header Checksum
Contains an 8 bit checksum across the cartridge header bytes 0134-014C.
The checksum is calculated as follows:
	x=0:FOR i=0134h TO 014Ch:x=x-MEM[i]-1:NEXT
The lower 8 bits of the result must be the same than the value in this entry. 
The GAME WON'T WORK if this checksum is incorrect.
*/
uv_err_t UVDGBObject::getHeaderChecksum(uint8_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDGBObject::computetHeaderChecksum(uint8_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}
		
uv_err_t UVDGBObject::isHeaderChecksumValid(uvd_bool_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDGBObject::computeGlobalChecksum(uint16_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
014E-014F - Global Checksum
Contains a 16 bit checksum (upper byte first) across the whole cartridge ROM.
Produced by adding all bytes of the cartridge (except for the two checksum
bytes). The Gameboy doesn't verify this checksum.
*/
uv_err_t UVDGBObject::getGlobalChecksum(uint16_t *out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDGBObject::canLoad(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence,
		void *user)
{
	//While this may work, likely its not a good loader and should be a last resort
	*confidence = UVD_MATCH_POOR;
	return UV_ERR_OK;
}

uv_err_t UVDGBObject::tryLoad(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out,
		void *user)
{
	UVDGBObject *binaryObject = NULL;
	
	binaryObject = new UVDGBObject();
	uv_assert_ret(binaryObject);
	uv_assert_err_ret(binaryObject->init(data));
	
	uv_assert_ret(out);
	*out = binaryObject;
	return UV_ERR_OK;
}

