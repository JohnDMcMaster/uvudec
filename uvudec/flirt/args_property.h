/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_ARGS_PROPERTY_H
#define UVD_FLIRT_ARGS_PROPERTY_H

#include "flirt/sig/sig.h"
#include "flirt/pat/pat.h"

#define UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH				"flirt.signature.length.min"
//In bytes
#define UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH_DEFAULT		4

#define UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH				"flirt.signature.length.max"
//In bytes.  This is FLAIR note, I don't actually know why yet.  Suspect it has to do with .pat format
#define UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH_DEFAULT		0x8000

/*
.pat file related
*/

//Output newline type, val cr, lf, or crlf
//IDA uses crlf it seems (at least on Windows)
#define UVD_PROP_FLIRT_PAT_NEWLINE						"flirt.pattern.newline"
#define UVD_PROP_FLIRT_PAT_NEWLINE_DEFAULT				"\r\n"

#define UVD_PROP_FLIRT_PAT_LEADING_LENGTH				"flirt.pattern.leading.length"
#define UVD_PROP_FLIRT_PAT_LEADING_LENGTH_DEFAULT		UVD_FLIRT_SIG_LEADING_LENGTH

#define UVD_PROP_FLIRT_PAT_MODULE_LENGTH_MAX			"flirt.pattern.module.length.max"
#define UVD_PROP_FLIRT_PAT_MODULE_LENGTH_MAX_DEFAULT	0x8000

/*
Certain linkers do hacks to speed up near calls by replacing far call code with
more efficient near call code
This is an advanced feature and may not be implemented for some time
Likely it will use some sort of reverse peephole optimization scripting
Setting true will increase the relocatable area to include these non
relocatable opcodes

Should be set to a bool val
*/
#define UVD_PROP_FLIRT_PAT_RELOC_NEARFAR			"flirt.relocation.nearfar"
#define UVD_PROP_FLIRT_PAT_RELOC_NEARFAR_DEFAULT	false

/*
.sig file related
*/

#endif

