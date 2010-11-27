/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
NOTE NOTE NOTE
These are intended for libuvudec internal use only
But if you insist, be warned I will be testing integration of boost::filesystem and many of these funcs could disappear
*/

#ifndef UV_UTIL_H
#define UV_UTIL_H

#include <stdint.h>
#include <string>
#include "uvd/util/types.h"
#include "uvd/util/error.h"

#define uvd_stringify_1(x)		#x
#define uvd_stringify(x)		uvd_stringify_1(x)

#define UV_TEMP_PREFIX			"/tmp/uvXXXXXX"

/*
Use NULL as a base address
Convert to a pointer
Get the address of a memory location
Since this is a hard coded value, compiler will optimize into a hard coded full address
*/
#define OFFSET_OF(type, field)			((unsigned long) &(((type *) 0)->field))

/*
GNU headers don't cope well with min/max...lame
They don't expose it, but get confused if you try to define it yourself
maybe caused issues with member functions?
Put in namespace maybe and do some template magic?
	#define uvd_min(a, b) uvd_minCore<typeof(a)>(a, b)
	template <typename T> T uvnet::uvd_minCore<T>(T a, T b) ...
*/
#ifndef uvd_min
#define uvd_min(a, b) ({typeof(a) _a = a; typeof(b) _b = b; typeof(a) _ret; if( _a > _b ) _ret = _b; else _ret = _a; _ret;})
#endif //uvd_min
#ifndef uvd_max
#define uvd_max(a, b) ({typeof(a) _a = a; typeof(b) _b = b; typeof(a) _ret; if( _a < _b ) _ret = _b; else _ret = _a; _ret;})
#endif //uvd_max

class UVDFileUtil
{
public:
	//Returns just the file names
	//static uv_err_t listDirectory(const std::string &dir);

};

std::string uv_basename(const std::string &file);
std::string uv_dirname(const std::string &file);
//Return the directory we were installed to
//Valid before UVDInit()
uv_err_t UVDGetInstallDir(std::string &out);
//Return the canonical path to the currently executing program name
uv_err_t getProgramName(std::string &out);
//Given symbolic link file, give canonical path
uv_err_t resolveSymbolicLink(const std::string &linkFile, std::string &out);
//Resolve relative to current directory
uv_err_t getCannonicalFileName(const std::string &filename, std::string &out);
//Resolve relative to specified dir
uv_err_t getCannonicalFileNameCore(const std::string &filename, const std::string &relativeDir, std::string &out);
uv_err_t getWorkingDir(std::string &out);
uv_err_t UVDGetHomeDir(std::string &out);
uv_err_t collapsePath(const std::string &relativePath, std::string &out);
//Only add if something else isn't already present
uv_err_t weaklyEnsurePathEndsWithExtension(const std::string &in, const std::string &extension, std::string &out);
uv_err_t ensurePathEndsWithExtension(const std::string &in, const std::string &extension, std::string &out);
uv_err_t ensurePathEndsWith(const std::string &in, const std::string &ending, std::string &out);

uv_err_t isRegularFile(const std::string &file);
uv_err_t isDir(const std::string &file);
//If bestEffort is set, willl try to create all dirs needed
uv_err_t createDir(const std::string &file, bool bestEffort);

//Remove comments, maybe more later
uv_err_t uvdPreprocessLine(const std::string &lineIn, std::string &lineOut);
uv_err_t uvdParseLine(const std::string &line_in, std::string &key_in, std::string &value_in);
uv_err_t readFile(const std::string &sFile, std::string &sRet);
uv_err_t UVDReadFileByString(const std::string &sFile, std::string &sRet);
uv_err_t writeFile(const std::string &sFile, const std::string &sIn);
uv_err_t UVDWriteFileByString(const std::string &sFile, const std::string &sIn);
uv_err_t writeFile(const std::string &sFile, const char *buff, size_t buffsz);
uv_err_t UVDWriteFileByBuffer(const std::string &sFile, const char *buff, size_t buffsz);
uv_err_t parseFunc(const std::string &text, std::string &name, std::string &content);

//Stop at null terminators and replace non-printables with .
//I could swear std::string stopped at nulls on CentOS 5, but Fedora 13 seems to include them...so guess we need this
std::string UVDSafeStringFromBuffer(const char *buff, size_t size);

//"0x100-0x1FF" or "0x100,0x1FF" form
//If a single number is given, assume from that number to itself
//Will error if not of either of these forms
uv_err_t parseNumericRangeString(const std::string &s, uint32_t *first, uint32_t *second);

uv_err_t getTempFile(std::string &out);
std::string escapeArg(const std::string &sIn);
uv_err_t deleteFile(std::string &out);
uv_err_t UVDExecuteToFile(const std::string &sCommand,
		const std::vector<std::string> &args,
		int &rcProcess,
		const std::string *stdOutIn,
		const std::string *stdErrIn);
uv_err_t UVDExecuteToText(const std::string &sCommand,
		const std::vector<std::string> &args,
		int &rcProcess,
		std::string *stdOut,
		std::string *stdErr);

std::string limitString(const std::string &s, size_t maxLength);
//trim/strip whitespace
std::string trimString(const std::string &s);

std::vector<std::string> split(const std::string &s, char delim, bool ret_blanks = true);
std::vector<std::string> UVDSplit(const std::string &s, char delim, bool ret_blanks = true);
//Trims off line termination properly (eventually...)
std::vector<std::string> UVDSplitLines(const std::string &s);
std::vector<std::string> charPtrArrayToVector(char *const *argv, int argc);

//Should mark these deprecated in favor of the C++ versions
//they tend to be in old C code causing mem leaks
char **uv_split_lines(const char *str, unsigned int *n_ret);
char **uv_split_core(const char *str, char delim, unsigned int *n_ret, int ret_blanks);

/* Line numbering starts at 0 */
char *uv_get_line(const char *text, unsigned int lineNumber);

/*
Current line doesn't count
Originally for checking stuff after --- in .pat file
*/
uint32_t nonBlankLinesRemaining(std::vector<std::string> &lines, std::vector<std::string>::iterator iter);

/*
Data returned is malloc'd 
If size is non-NULL, the data size is stored there
If size is NULL, ascii data will be assumed and ret will be null terminated
Error upon file being NULL
If ret is null and size isn't, will report file length
*/
uv_err_t read_file(const char *file, uint8_t **ret, unsigned int *size);
uv_err_t read_filea(const char *file, char **ret);

/*
Instruction capitalization
Make sure its aligned with g_caps option
g_caps = 0: lower case
otherwise: upper case
*/
//const char *inst_caps(const char *inst);
//Change string in place to above policy
//Returns str
char *cap_fixup(char *str);
std::string UVDToUpper(const std::string &in);
std::string UVDToLower(const std::string &in);

/* Separate key/value pairs */
uv_err_t uvd_parse_line(const char *line_in, char **key_in, char **value_in);

std::string parseSubstring(const std::string &in, const std::string &seek, const std::string &start, const std::string &end, std::string::size_type *pos = 0);

//Get function arguments, parsing parenthesis correctly
uv_err_t getArguments(const std::string &in, std::vector<std::string> &out);

/*
As would be needed to pass to "system"
*/
std::string stringVectorToSystemArgument(const std::vector<std::string> &args);
//If variable is not found, will return UV_ERR_BLANK and value will be clear
uv_err_t getEnvironmentVariable(const std::string &variable, std::string &value);
//Used for benchmarking
uint64_t getTimingMicroseconds(void);

/*
Is the string a legal C symbol?
*/
uv_err_t isCSymbol(const std::string &in);
/*
Is the input a legal config identifier?
Currently they must follow C symbol syntax
*/
uv_err_t isConfigIdentifier(const std::string &in);

#define UVD_HEXDUMP(data, size)			UVDHexdump((uint8_t *)data, size)
#define UVD_HEXDUMP_OBJ(obj)			UVDHexdump((uint8_t *)obj, sizeof(*obj))
#define UVD_HEXDUMP_STR(s)				UVDHexdump((uint8_t *)s.c_str(), s.size())
#define UVD_HEXDUMP_CSTR(s)				UVDHexdump((uint8_t *)s, strlen(s))
#define UVD_HEXDUMP_THIS()				UVD_HEXDUMP_OBJ(this)
void UVDHexdump(const uint8_t *data, size_t size);
void UVDHexdumpCore(const uint8_t *data, size_t size, const std::string &prefix);

//printf like formatting to a std::string
std::string UVDSprintf(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));

#define UVD_WARN_IF_VERSION_MISMATCH()\
		do \
		{\
			if( strcmp(UVUDEC_VER_STRING, UVDGetVersion()) )\
			{\
				printf_warn("libuvudec version mismatch (exe: %s, libuvudec: %s)\n", UVUDEC_VER_STRING, UVDGetVersion());\
				fflush(stdout);\
			}\
		}\
		while( 0 )

#endif /* ifndef UV_UTIL_H */

