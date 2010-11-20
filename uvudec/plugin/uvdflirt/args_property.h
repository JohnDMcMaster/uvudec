/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_ARGS_PROPERTY_H
#define UVD_FLIRT_ARGS_PROPERTY_H

#include "uvdflirt/sig/format.h"
#include "uvdflirt/sig/sig.h"

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

/*
Version to use when creating files
*/
#define UVD_PROP_FLIRT_SIG_VERSION								"flirt.sig.version"
#define UVD_PROP_FLIRT_SIG_VERSION_DEFAULT						7

/*
Library name field
Also helps to advertise uvudec and tag files for lazy users
*/
#define UVD_PROP_FLIRT_SIG_LIB_NAME								"flirt.sig.lib_name"
#define UVD_PROP_FLIRT_SIG_LIB_NAME_DEFAULT						"uvudec unnamed library"

/*
File features flags
Includes things such as is startup signature, is compressed, etc
*/
#define UVD_PROP_FLIRT_SIG_FEATURES								"flirt.sig.features"
#define UVD_PROP_FLIRT_SIG_FEATURES_DEFAULT						UVD_FLIRT_SIG_FEATURE_NONE

/*
An unknown importance field
(pad ie was needed to make structure alignment correct)
Seem to be set to 0 in reference files
*/
#define UVD_PROP_FLIRT_SIG_PAD									"flirt.sig.pad"
#define UVD_PROP_FLIRT_SIG_PAD_DEFAULT							0x00

/*
Processor ID enum
*/
#define UVD_PROP_FLIRT_SIG_PROCESSOR_ID							"flirt.sig.processor_id"
#define UVD_PROP_FLIRT_SIG_PROCESSOR_ID_DEFAULT					UVD_FLIRT_SIG_PROCESSOR_ID_80X86

/*
Operating system flags
*/
#define UVD_PROP_FLIRT_SIG_OS_TYPES								"flirt.sig.OS_types"
#define UVD_PROP_FLIRT_SIG_OS_TYPES_DEFAULT						UVD_FLIRT_SIG_OS_WIN

/*
Applicable application type flags
*/
#define UVD_PROP_FLIRT_SIG_APP_TYPES							"flirt.sig.app_types"
#define UVD_PROP_FLIRT_SIG_APP_TYPES_DEFAULT					(UVD_FLIRT_SIG_APP_CONSOLE | UVD_FLIRT_SIG_APP_GRAPHICS | UVD_FLIRT_SIG_APP_EXE | UVD_FLIRT_SIG_APP_DLL | UVD_FLIRT_SIG_APP_DRV | UVD_FLIRT_SIG_APP_SINGLE_THREADED | UVD_FLIRT_SIG_APP_MULTI_THREADED | UVD_FLIRT_SIG_APP_32_BIT)

/*
Applicable object type flags
*/
#define UVD_PROP_FLIRT_SIG_FILE_TYPES							"flirt.sig.file_types"
#define UVD_PROP_FLIRT_SIG_FILE_TYPES_DEFAULT					UVD_FLIRT_SIG_FILE_PE

#endif

