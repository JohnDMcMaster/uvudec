/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_ARGS_PROPERTY_H
#define UVD_FLIRT_ARGS_PROPERTY_H

#include "uvd/flirt/sig/sig.h"
#include "uvd/flirt/pat/pat.h"

//Enable options that make output more closely resemble what FLAIR would do
#define UVD_PROP_FLIRT_FLAIR_COMPATIBILITY						"flirt.flair_compatibility"
//If I change something, its because I think I can do better, but might want compatibility
#define UVD_PROP_FLIRT_FLAIR_COMPATIBILITY_DEFAULT				false

#define UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH						"flirt.signature.length.min"
//In bytes
#define UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH_DEFAULT				4

#define UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH						"flirt.signature.length.max"
//In bytes.  This is FLAIR note, I don't actually know why yet.  Suspect it has to do with .sig format
#define UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH_DEFAULT				0x8000

/*
.pat file related
*/

//Output newline type, val cr, lf, or crlf
//IDA uses crlf it seems (at least on Windows)
#define UVD_PROP_FLIRT_PAT_NEWLINE								"flirt.pattern.newline"
#define UVD_PROP_FLIRT_PAT_NEWLINE_DEFAULT						"\r\n"

#define UVD_PROP_FLIRT_PAT_LEADING_LENGTH						"flirt.pattern.leading.length"
#define UVD_PROP_FLIRT_PAT_LEADING_LENGTH_DEFAULT				UVD_FLIRT_SIG_LEADING_LENGTH

#define UVD_PROP_FLIRT_PAT_MODULE_LENGTH_MAX					"flirt.pattern.module.length.max"
#define UVD_PROP_FLIRT_PAT_MODULE_LENGTH_MAX_DEFAULT			0x8000

//Should functions be placed into their own signatures or grouped by modules?
#define UVD_PROP_FLIRT_PAT_FUNCTIONS_AS_MODULES					"flirt.pattern.functions_as_modules"
#define UVD_PROP_FLIRT_PAT_FUNCTIONS_AS_MODULES_DEFAULT			false

//FLAIR like to prepend underscores to some formats
#define UVD_PROP_FLIRT_PAT_PREFIX_UNDERSCORES					"flirt.pattern.prefix_underscores"
#define UVD_PROP_FLIRT_PAT_PREFIX_UNDERSCORES_DEFAULT			false

//FLAIR always puts a space after references, even if at the end of the line
#define UVD_PROP_FLIRT_PAT_REFERENCE_TRAILING_SPACE				"flirt.pattern.reference_trailing_space"
#define UVD_PROP_FLIRT_PAT_REFERENCE_TRAILING_SPACE_DEFAULT 	false

//FLAIR treats names with length shorter than 3 as unknown
#define UVD_PROP_FLIRT_PAT_PUBLIC_NAME_LENGTH_MIN				"flirt.pattern.name.length.min"
#define UVD_PROP_FLIRT_PAT_PUBLIC_NAME_LENGTH_MIN_DEFAULT		3

/*
Certain linkers do hacks to speed up near calls by replacing far call code with
more efficient near call code
This is an advanced feature and may not be implemented for some time
Likely it will use some sort of reverse peephole optimization scripting
Setting true will increase the relocatable area to include these non
relocatable opcodes

Should be set to a bool val
*/
#define UVD_PROP_FLIRT_PAT_RELOC_NEARFAR						"flirt.relocation.nearfar"
#define UVD_PROP_FLIRT_PAT_RELOC_NEARFAR_DEFAULT				false

/*
.sig file related
*/

#endif

