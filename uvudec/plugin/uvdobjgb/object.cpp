/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
References:
http://nocash.emubase.de/pandocs.htm#thecartridgeheader
http://www.zophar.net/fileuploads/2/10597teazh/gbrom.txt
*/

#include "uvdobjgb/object.h"

#define UVDOBJGB_LOGO_ADDR_MIN					0x0104
#define UVDOBJGB_LOGO_ADDR_MAX					0x0133
//Beging conflicting ranges
#define UVDOBJGB_TITLE_ADDR_MIN					0x0134
#define UVDOBJGB_TITLE_ADDR_MAX					0x0143
#define UVDOBJGB_MANUFACTURER_CODE_ADDR_MIN		0x013F
#define UVDOBJGB_MANUFACTURER_CODE_ADDR_MAX		0x0142
#define UVDOBJGB_CGB_ADDR_MIN					0x0143
//End conflicting ranges
#define UVDOBJGB_NEW_LICENSEE_CODE_ADDR_MIN		0x0144
#define UVDOBJGB_SGB_ADDR_MIN					0x0146
#define UVDOBJGB_CARTRIDGE_TYPE_ADDR_MIN		0x0147
#define UVDOBJGB_ROM_SIZE_ADDR_MIN				0x0148
#define UVDOBJGB_SAVE_RAM_SIZE_ADDR_MIN			0x0149
#define UVDOBJGB_DEST_CODE_ADDR_MIN				0x014A
#define UVDOBJGB_OLD_LIC_CODE_ADDR_MIN			0x014B
#define UVDOBJGB_MASK_ROM_VER_ADDR_MIN			0x014C
#define UVDOBJGB_HEADER_CHECKSUM_ADDR_MIN		0x014D
#define UVDOBJGB_GLOBAL_CHECKSUM_ADDR_MIN		0x014E

UVDGBObject::UVDGBObject()
{
}

UVDGBObject::~UVDGBObject()
{
}

uv_err_t UVDGBObject::init(UVDData *data)
{
	//Seems to be a ROM object
	uv_assert_err(UVDBinaryObject::init(data));
	
	m_nintendoLogo = new UVDDataChunk();
	uv_assert(m_nintendoLogo);
	uv_assert_err(m_nintendoLogo->init(m_data, UVDOBJGB_LOGO_ADDR_MIN, UVDOBJGB_LOGO_ADDR_MAX));

	return UV_ERR_OK;

error:
	m_data = NULL;
	delete m_nintendoLogo;
	return UV_DEBUG(UV_ERR_GENERAL);
}

//XXX: this should probably be a generic object function
//Also, figure out how to resolve address space nicely
//Probably needs to be paired with a section
/*
"0100-0103 - Entry Point
After displaying the Nintendo Logo, the built-in boot procedure jumps to this
address (100h), which should then jump to the actual main program in the
cartridge. Usually this 4 byte area contains a NOP instruction, followed by a
JP 0150h instruction. But not always.
*/
uv_err_t UVDGBObject::getEntryPoint(uv_addr_t *entryPoint)
{
	/*
	XXX: do we want to report the "real" entry point if this is a jump statement as per above?
	*/
	//return UV_DEBUG(m_data->readU32(UVDOBJGB_MANUFACTURER_CODE_ADDR_MIN, out));
	*entryPoint = 0x0100;
	return UV_ERR_OK;
}

void UVDGBObject::debugPrint()
{
	printf_plugin_debug("\n\n");

	uv_addr_t entryPoint = 0;
	UV_DEBUG(getEntryPoint(&entryPoint));
	printf_plugin_debug("entry point: 0x%08X\n", entryPoint);

	/*
	FIXME: this causes a crash
	uvd_bool_t bIsNintendoLogo = false;
	UV_DEBUG(isNintendoLogo(&bIsNintendoLogo));
	printf_plugin_debug("is nintendo logo: %d\n", bIsNintendoLogo);
	*/

	std::string title;
	UV_DEBUG(getTitle(title));
	printf_plugin_debug("title: %s\n", title.c_str());

	uint32_t manufacturerCode = 0;
	UV_DEBUG(getManufacturerCode(&manufacturerCode));
	printf_plugin_debug("manufacturer code: 0x%08X\n", manufacturerCode);

	/*
	UV_DEBUG(getCGBFlags(uint8_t *out));

	UV_DEBUG(getNewLicenseeCode(uint16_t *out));

	UV_DEBUG(getSGBFlags(uint8_t *out));

	UV_DEBUG(isSGBEnabled(uvd_bool_t *out));

	UV_DEBUG(getCartridgeType(uint8_t *out));
	*/

	uint32_t ROMSize = 0;
	UV_DEBUG(getROMSize(&ROMSize));
	printf_plugin_debug("ROM size (bytes): 0x%08X\n", ROMSize);
	uint8_t ROMSizeRaw = 0;
	UV_DEBUG(getROMSizeRaw(&ROMSizeRaw));
	printf_plugin_debug("ROM size (raw): 0x%02X\n", ROMSizeRaw);

	uint32_t saveRAMSize = 0;
	UV_DEBUG(getSaveRAMSize(&saveRAMSize));
	printf_plugin_debug("Save RAM size (bytes): 0x%08X\n", saveRAMSize);
	uint8_t saveRAMSizeRaw = 0;
	UV_DEBUG(getSaveRAMSizeRaw(&saveRAMSizeRaw));
	printf_plugin_debug("save RAM size (raw): 0x%02X\n", saveRAMSizeRaw);

	/*
	UV_DEBUG(getDestinationCode(uint8_t *out));

	UV_DEBUG(getOldLicenseeCode(uint8_t *out));

	UV_DEBUG(getMaskROMVersioNumber(uint8_t *out));
	*/

	uvd_bool_t headerChecksumValid = false;
	uint8_t headerChecksumFromFile = 0;
	uint8_t headerChecksumComputed = 0;
	UV_DEBUG(getHeaderChecksum(&headerChecksumFromFile));
	UV_DEBUG(computeHeaderChecksum(&headerChecksumComputed));
	UV_DEBUG(isHeaderChecksumValid(&headerChecksumValid));
	printf_plugin_debug("header checksum, computed: 0x%02X, from file: 0x%02X, valid: %d\n",
			headerChecksumComputed, headerChecksumFromFile, headerChecksumValid);

	uvd_bool_t globalChecksumValid = false;
	uint16_t globalChecksumFromFile = 0;
	uint16_t globalChecksumComputed = 0;
	UV_DEBUG(getGlobalChecksum(&globalChecksumFromFile));
	UV_DEBUG(computeGlobalChecksum(&globalChecksumComputed));
	UV_DEBUG(isGlobalChecksumValid(&globalChecksumValid));
	printf_plugin_debug("global checksum, computed: 0x%04X, from file: 0x%04X, valid: %d\n",
			globalChecksumComputed, globalChecksumFromFile, globalChecksumValid);

	printf_plugin_debug("\n\n");
}

/*
"0104-0133 - Nintendo Logo
These bytes define the bitmap of the Nintendo logo that is displayed when
the gameboy gets turned on. The hexdump of this bitmap is:
	CE ED 66 66 CC 0D 00 0B 03 73 00 83 00 0C 00 0D
	00 08 11 1F 88 89 00 0E DC CC 6E E6 DD DD D9 99
	BB BB 67 63 6E 0E EC CC DD DC 99 9F BB B9 33 3E

We retain ownership of the data
Data is only valid as long as this object is valid"
*/
static uint8_t g_nintendoLogo[] = 
{
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

uv_err_t UVDGBObject::getStartupLogo(const UVDData **out)
{
	uv_assert_ret(out);
	*out = m_nintendoLogo;
	return UV_ERR_OK;
}

//out set to true if matches expected value
uv_err_t UVDGBObject::isNintendoLogo(uvd_bool_t *out)
{
	uv_assert_ret(out);
	uv_assert_ret(m_data);
	*out = m_data->compareBytes(&g_nintendoLogo[0], sizeof(g_nintendoLogo)) == 0;
	return UV_ERR_OK;
}

//"0134-0143 - Title"
uv_err_t UVDGBObject::getTitle(std::string &out)
{
	return UV_DEBUG(m_data->readDataAsSafeString(UVDOBJGB_TITLE_ADDR_MIN,
			UVDOBJGB_TITLE_ADDR_MAX - UVDOBJGB_TITLE_ADDR_MIN + 1, out));
}

/*
"013F-0142 - Manufacturer Code
In older cartridges this area has been part of the Title (see above), in
newer cartridges this area contains an 4 character uppercase manufacturer
code. Purpose and Deeper Meaning unknown."
*/
uv_err_t UVDGBObject::getManufacturerCode(uint32_t *out)
{
	return UV_DEBUG(m_data->readU32(UVDOBJGB_MANUFACTURER_CODE_ADDR_MIN, out));
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
that include fixed palette data at a special location in ROM."
*/
uv_err_t UVDGBObject::getCGBFlags(uint8_t *out)
{
	return UV_DEBUG(m_data->readU8(UVDOBJGB_CGB_ADDR_MIN, out));
}

/*
"0144-0145 - New Licensee Code
Specifies a two character ASCII licensee code, indicating the company or
publisher of the game. These two bytes are used in newer games only (games
that have been released after the SGB has been invented). Older games are
using the header entry at 014B instead."
*/
uv_err_t UVDGBObject::getNewLicenseeCode(uint16_t *out)
{
	return UV_DEBUG(m_data->readU16(UVDOBJGB_NEW_LICENSEE_CODE_ADDR_MIN, out, UVD_DATA_ENDIAN_BIG));
}

uv_err_t UVDGBObject::licenseeCodeToString(uint32_t code, std::string &out)
{
	switch(code)
	{
	case 0x00:
		out = "none";
		break;
	case 0x01:
		out = "nintendo";
		break;
	case 0x09:
		out = "hot-b";
		break;
	case 0x0A:
		out = "jaleco";
		break;
	case 0x0C:
		out = "elite systems";
		break;
	case 0x13:
		out = "electronic arts";
		break;
	case 0x19:
		out = "itc entertainment";
		break;
	case 0x1A:
		out = "yanoman";
		break;
	case 0x1F:
		out = "virgin";
		break;
	case 0x24:
		out = "pcm complete";
		break;
	case 0x28:
		out = "kotobuki systems";
		break;
	case 0x29:
		out = "seta";
		break;
	case 0x31:
		out = "nintendo";
		break;
	case 0x32:
		out = "bandai";
		break;
	case 0x34:
		out = "konami";
		break;
	case 0x35:
		out = "hector";
		break;
	case 0x39:
		out = "banpresto";
		break;
	case 0x3C:
		out = "*entertainment i";
		break;
	case 0x41:
		out = "ubi soft";
		break;
	case 0x42:
		out = "atlus";
		break;
	case 0x46:
		out = "angel";
		break;
	case 0x47:
		out = "spectrum holoby";
		break;
	case 0x4A:
		out = "virgin";
		break;
	case 0x4D:
		out = "malibu";
		break;
	case 0x50:
		out = "absolute";
		break;
	case 0x51:
		out = "acclaim";
		break;
	case 0x53:
		out = "american sammy";
		break;
	case 0x54:
		out = "gametek";
		break;
	case 0x56:
		out = "ljn";
		break;
	case 0x57:
		out = "matchbox";
		break;
	case 0x5A:
		out = "mindscape";
		break;
	case 0x5B:
		out = "romstar";
		break;
	case 0x5D:
		out = "tradewest";
		break;
	case 0x60:
		out = "titus";
		break;
	case 0x67:
		out = "ocean";
		break;
	case 0x69:
		out = "electronic arts";
		break;
	case 0x6F:
		out = "electro brain";
		break;
	case 0x70:
		out = "infogrames";
		break;
	case 0x72:
		out = "broderbund";
		break;
	case 0x73:
		out = "sculptered soft";
		break;
	case 0x78:
		out = "t*hq";
		break;
	case 0x79:
		out = "accolade";
		break;
	case 0x7C:
		out = "microprose";
		break;
	case 0x7F:
		out = "kemco";
		break;
	case 0x83:
		out = "lozc";
		break;
	case 0x86:
		out = "*tokuma shoten i";
		break;
	case 0x8C:
		out = "vic tokai";
		break;
	case 0x8E:
		out = "ape";
		break;
	case 0x91:
		out = "chun soft";
		break;
	case 0x92:
		out = "video system";
		break;
	case 0x95:
		out = "varie";
		break;
	case 0x96:
		out = "yonezawa/s'pal";
		break;
	case 0x99:
		out = "arc";
		break;
	case 0x9A:
		out = "nihon bussan";
		break;
	case 0x9C:
		out = "imagineer";
		break;
	case 0x9D:
		out = "banpresto";
		break;
	case 0xA1:
		out = "hori electric";
		break;
	case 0xA2:
		out = "bandai";
		break;
	case 0xA6:
		out = "kawada";
		break;
	case 0xA7:
		out = "takara";
		break;
	case 0xAA:
		out = "broderbund";
		break;
	case 0xAC:
		out = "toei animation";
		break;
	case 0xAF:
		out = "namco";
		break;
	case 0xB0:
		out = "acclaim";
		break;
	case 0xB2:
		out = "bandai";
		break;
	case 0xB4:
		out = "enix";
		break;
	case 0xB7:
		out = "snk";
		break;
	case 0xB9:
		out = "pony canyon";
		break;
	case 0xBB:
		out = "sunsoft";
		break;
	case 0xBD:
		out = "sony imagesoft";
		break;
	case 0xC0:
		out = "taito";
		break;
	case 0xC2:
		out = "kemco";
		break;
	case 0xC4:
		out = "*tokuma shoten i";
		break;
	case 0xC5:
		out = "data east";
		break;
	case 0xC8:
		out = "koei";
		break;
	case 0xC9:
		out = "ufl";
		break;
	case 0xCB:
		out = "vap";
		break;
	case 0xCC:
		out = "use";
		break;
	case 0xCE:
		out = "*pony canyon or";
		break;
	case 0xCF:
		out = "angel";
		break;
	case 0xD1:
		out = "sofel";
		break;
	case 0xD2:
		out = "quest";
		break;
	case 0xD4:
		out = "ask kodansha";
		break;
	case 0xD6:
		out = "naxat soft";
		break;
	case 0xD9:
		out = "banpresto";
		break;
	case 0xDA:
		out = "tomy";
		break;
	case 0xDD:
		out = "ncs";
		break;
	case 0xDE:
		out = "human";
		break;
	case 0xE0:
		out = "jaleco";
		break;
	case 0xE1:
		out = "towachiki";
		break;
	case 0xE3:
		out = "varie";
		break;
	case 0xE5:
		out = "epoch";
		break;
	case 0xE8:
		out = "asmik";
		break;
	case 0xE9:
		out = "natsume";
		break;
	case 0xEB:
		out = "atlus";
		break;
	case 0xEC:
		out = "epic/sony records";
		break;
	case 0xF0:
		out = "a wave";
		break;
	case 0xF3:
		out = "extreme entertaint";
		break;
	default:
		return UV_ERR_NOTFOUND;
	}
	return UV_ERR_OK;
}

/*
"0146 - SGB Flag
Specifies whether the game supports SGB functions, common values are:
	00h = No SGB functions (Normal Gameboy or CGB only game)
	03h = Game supports SGB functions
The SGB disables its SGB functions if this byte is set to another value
than 03h."
*/
uv_err_t UVDGBObject::getSGBFlags(uint8_t *out)
{
	return UV_DEBUG(m_data->readU8(UVDOBJGB_SGB_ADDR_MIN, out));
}

uv_err_t UVDGBObject::isSGBEnabled(uvd_bool_t *out)
{
	uint8_t flags = 0;
	
	uv_assert_err_ret(getSGBFlags(&flags));
	if( flags == 0x03 )
	{
		*out = true;
	}
	else
	{
		*out = false;
		if( flags != 0x00 )
		{
			UVDPrintfWarning("SGB flags usually has either 0x03 or 0x00, got 0x%002X\n", flags);
		}
	}
	return UV_ERR_OK;
}

/*
"0147 - Cartridge Type
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
12h  MBC3+RAM"
*/
uv_err_t UVDGBObject::getCartridgeType(uint8_t *out)
{
	return UV_DEBUG(m_data->readU8(UVDOBJGB_CARTRIDGE_TYPE_ADDR_MIN, out));
}

uv_err_t UVDGBObject::cartridgeTypeToString(uint8_t cartridgeType, std::string &out)
{
	switch(cartridgeType)
	{
	case 0x00:
		out = "ROM";
		break;
	case 0x01:
		out = "MBC1";
		break;
	case 0x02:
		out = "MBC1+RAM";
		break;
	case 0x03:
		out = "MBC1+RAM+BATTERY";
		break;
	case 0x05:
		out = "MBC2";
		break;
	case 0x06:
		out = "MBC2+BATTERY";
		break;
	case 0x08:
		out = "ROM+RAM";
		break;
	case 0x09:
		out = "ROM+RAM+BATTERY";
		break;
	case 0x0B:
		out = "MMM01";
		break;
	case 0x0C:
		out = "MMM01+RAM";
		break;
	case 0x0D:
		out = "MMM01+RAM+BATTERY";
		break;
	case 0x0F:
		out = "MBC3+TIMER+BATTERY";
		break;
	case 0x10:
		out = "MBC3+TIMER+RAM+BATTERY";
		break;
	case 0x11:
		out = "MBC3";
		break;
	case 0x12:
		out = "MBC3+RAM";
		break;
	case 0x13:
		out = "MBC3+RAM+BATTERY";
		break;
	case 0x15:
		out = "MBC4";
		break;
	case 0x16:
		out = "MBC4+RAM";
		break;
	case 0x17:
		out = "MBC4+RAM+BATTERY";
		break;
	case 0x19:
		out = "MBC5";
		break;
	case 0x1A:
		out = "MBC5+RAM";
		break;
	case 0x1B:
		out = "MBC5+RAM+BATTERY";
		break;
	case 0x1C:
		out = "MBC5+RUMBLE";
		break;
	case 0x1D:
		out = "MBC5+RUMBLE+RAM";
		break;
	case 0x1E:
		out = "MBC5+RUMBLE+RAM+BATTERY";
		break;
	case 0xFC:
		out = "POCKET CAMERA";
		break;
	case 0xFD:
		out = "Bandai TAMA5";
		break;
	case 0xFE:
		out = "HuC3";
		break;
	case 0xFF:
		out = "HuC1+RAM+BATTERY";
		break;
	}
	return UV_ERR_OK;
}

/*
"0148 - ROM Size
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
54h - 1.5MByte (96 banks)"
*/
//Return in bytes
uv_err_t UVDGBObject::getROMSize(uint32_t *out)
{
	uint8_t raw = 0;
	
	uv_assert_err_ret(getROMSizeRaw(&raw));
	*out = 1 << (15 + raw);

	return UV_ERR_OK;
}

//Raw byte value
uv_err_t UVDGBObject::getROMSizeRaw(uint8_t *out)
{
	return UV_DEBUG(m_data->readU8(UVDOBJGB_ROM_SIZE_ADDR_MIN, out));
}

/*
"0149 - RAM Size
Specifies the size of the external RAM in the cartridge (if any).

00h - None
01h - 2 KBytes
02h - 8 Kbytes
03h - 32 KBytes (4 banks of 8KBytes each)

When using a MBC2 chip 00h must be specified in this entry, even though
the MBC2 includes a built-in RAM of 512 x 4 bits."

"Save RAM size
=============
One byte also, self-explanitory.  Sizes range from no RAM, to 32K.  The most
common ones used are 8k and 32k.  HEX values:
00- 0k          01- 2k          02- 8k          03- 32k"
*/
uv_err_t UVDGBObject::getSaveRAMSize(uint32_t *out)
{
	uint8_t raw = 0;
	
	uv_assert_err_ret(getSaveRAMSizeRaw(&raw));
	switch( raw )
	{
	case 0x00:
		*out = 0;
		break;
	case 0x01:
		//2KB
		*out = 1 << 11;
		break;
	case 0x02:
		//8KB
		*out = 1 << 13;
		break;
	case 0x03:
		//32KB
		*out = 1 << 15;
		break;
	default:
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}

//Raw byte value
uv_err_t UVDGBObject::getSaveRAMSizeRaw(uint8_t *out)
{
	return UV_DEBUG(m_data->readU8(UVDOBJGB_SAVE_RAM_SIZE_ADDR_MIN, out));
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
	return UV_DEBUG(m_data->readU8(UVDOBJGB_DEST_CODE_ADDR_MIN, out));
}

/*
014B - Old Licensee Code
Specifies the games company/publisher code in range 00-FFh. A value of 33h
signalizes that the New License Code in header bytes 0144-0145 is used instead.
(Super GameBoy functions won't work if <> $33.)
*/
uv_err_t UVDGBObject::getOldLicenseeCode(uint8_t *out)
{
	return UV_DEBUG(m_data->readU8(UVDOBJGB_OLD_LIC_CODE_ADDR_MIN, out));
}

/*
014C - Mask ROM Version number
Specifies the version number of the game. That is usually 00h.
*/
uv_err_t UVDGBObject::getMaskROMVersioNumber(uint8_t *out)
{
	return UV_DEBUG(m_data->readU8(UVDOBJGB_MASK_ROM_VER_ADDR_MIN, out));
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
	return UV_DEBUG(m_data->readU8(UVDOBJGB_HEADER_CHECKSUM_ADDR_MIN, out));
}

uv_err_t UVDGBObject::computeHeaderChecksum(uint8_t *out)
{
	uint8_t checksum = 0;

	for( uv_addr_t addr = 0x0134; addr <= 0x014C; ++addr )
	{
		uint8_t cur = 0;
		
		uv_assert_err_ret(m_data->readU8(addr, &cur));
		checksum -= cur + 1;
	}
	*out = checksum;

	return UV_ERR_OK;
}
		
uv_err_t UVDGBObject::isHeaderChecksumValid(uvd_bool_t *out)
{
	uint8_t fileChecksum = 0;
	uint8_t computedChecksum = 0;

	uv_assert_err_ret(getHeaderChecksum(&fileChecksum));
	uv_assert_err_ret(computeHeaderChecksum(&computedChecksum));
	uv_assert_ret(out);
	*out = fileChecksum == computedChecksum;

	return UV_ERR_OK;
}

/*
014E-014F - Global Checksum
Contains a 16 bit checksum (upper byte first) across the whole cartridge ROM.
Produced by adding all bytes of the cartridge (except for the two checksum
bytes). The Gameboy doesn't verify this checksum.
*/
uv_err_t UVDGBObject::getGlobalChecksum(uint16_t *out)
{
	//Upper byte first: 
	return UV_DEBUG(m_data->readU16(UVDOBJGB_GLOBAL_CHECKSUM_ADDR_MIN, out, UVD_DATA_ENDIAN_BIG));
}

uv_err_t UVDGBObject::computeGlobalChecksum(uint16_t *out)
{
	uint16_t checksum = 0;

	//We could move the if to outside the loop
	for( uv_addr_t addr = 0; addr < m_data->size(); ++addr )
	{
		uint8_t cur = 0;
		
		if( addr < 0x014E || addr > 0x014F )
		{
			uv_assert_err_ret(m_data->readU8(addr, &cur));
			checksum += cur;
		}
	}
	*out = checksum;

	return UV_ERR_OK;
}

uv_err_t UVDGBObject::isGlobalChecksumValid(uvd_bool_t *out)
{
	uint16_t fileGlobalChecksum = 0;
	uint16_t computedGlobalChecksum = 0;

	uv_assert_err_ret(getGlobalChecksum(&fileGlobalChecksum));
	uv_assert_err_ret(computeGlobalChecksum(&computedGlobalChecksum));
	uv_assert_ret(out);
	*out = fileGlobalChecksum == computedGlobalChecksum;

	return UV_ERR_OK;
}

uv_err_t UVDGBObject::canLoad(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence,
		void *user)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDGBObject temp;
	//Required for the game to work
	uvd_bool_t headerChecksumValid = false;
	//Optional
	uvd_bool_t globalChecksumValid = true;
	
	uv_assert_ret(confidence);
	uv_assert_err(temp.init((UVDData *)data));
	//temp.debugPrint();
	uv_assert_err(temp.isHeaderChecksumValid(&headerChecksumValid));
	//uv_assert_err_ret(isGlobalChecksumValid(&globalChecksumValid));
	//Think I read this is required to be correct
	//uv_err_t isNintendoLogo(uvd_bool_t *out);
	//temp.debugPrint();
	printf_plugin_debug("header checksum valid: %d\n", headerChecksumValid);
	printf_plugin_debug("global checksum valid: %d\n", globalChecksumValid);
	if( headerChecksumValid && globalChecksumValid )
	{
		*confidence = UVD_MATCH_ACCEPTABLE;
	}
	else
	{
		//At best the checksums are improperly calculated
		//we could check the nintendo logo as well
		*confidence = UVD_MATCH_POOR;
	}
	
	rc = UV_ERR_OK;
	
error:
	temp.m_data = NULL;
	return UV_DEBUG(rc);
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

