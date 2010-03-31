/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_FLIRT_ARGS_PROPERTY_H
#define UVD_FLIRT_ARGS_PROPERTY_H

#define UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH				"flirt.signature.length.min"
//In bytes
#define UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH_DEFAULT		4

#define UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH				"flirt.signature.length.max"
//In bytes.  This is FLAIR note, I don't actually know why yet.  Suspect it has to do with .pat format
#define UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH_DEFAULT		0x8000

//Output newline type, val cr, lf, or crlf
//IDA uses crlf it seems (at least on Windows)
#define UVD_PROP_FLIRT_PAT_NEWLINE						"flirt.pattern.newline"
#define UVD_PROP_FLIRT_PAT_NEWLINE_DEFAULT				"\r\n"

/*
Certain linkers do hacks to speed up near calls by replacing far call code with
more efficient near call code
This is an advanced feature and may not be implemented for some time
Likely it will use some sort of reverse peephole optimization scripting
Setting true will increase the relocatable area to include these non
relocatable opcodes

Should be set to a bool val
*/
#define UVD_PROP_FLIRT_RELOCATION_NEARFAR				"flirt.relocation.nearfar"
#define UVD_PROP_FLIRT_RELOCATION_NEARFAR_DEFAULT		false

#endif
