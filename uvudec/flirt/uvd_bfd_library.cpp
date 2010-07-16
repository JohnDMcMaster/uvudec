/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
Primary reference
http://ftp.gnu.org/old-gnu/Manuals/bfd-2.9.1/html_node/bfd_toc.html
Well actually a PDF ver found somewhere else, but I couldn't figure out the original link, seems roughly equiv

Supports over 200 formats on my system!
	./configure --enable-targets=all
A lot of these are arch specific of same lib format though
Tested with 2.19.1
	remove -Werror in bfd/Makefile	

2.17
	compiling binutils-2.17   with gcc-4.1.2 on  alphaev6-unknown-linux-gnu

	the problem:

	make[4]: Entering directory `/other/local/build/binutilbuild.alpha/opcodes'

	/bin/sh ./libtool --mode=compile gcc -DHAVE_CONFIG_H -I. -I../../binutils-2.17/./opcodes -I. -D_GNU_SOURCE -I. -I../../binutils-2.17/./opcodes -I../bfd -I../../binutils-2.17/./opcodes/../include -I../../binutils-2.17/./opcodes/../bfd -I../../binutils-2.17/./opcodes/../intl -I../intl -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror -g -O2 -c -o h8300-dis.lo ../../binutils-2.17/./opcodes/h8300-dis.c gcc -DHAVE_CONFIG_H -I. -I../../binutils-2.17/./opcodes -I. -D_GNU_SOURCE -I. -I../../binutils-2.17/./opcodes -I../bfd -I../../binutils-2.17/./opcodes/../include -I../../binutils-2.17/./opcodes/../bfd -I../../binutils-2.17/./opcodes/../intl -I../intl -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror -g -O2 -c ../../binutils-2.17/./opcodes/h8300-dis.c -o h8300-dis.o

	cc1: warnings being treated as errors
	../../binutils-2.17/./opcodes/h8300-dis.c: In function 'bfd_h8_disassemble':

	../../binutils-2.17/./opcodes/h8300-dis.c:365: warning: initialization discards qualifiers from pointer target type ../../binutils-2.17/./opcodes/h8300-dis.c:643: warning: initialization discards qualifiers from pointer target type ../../binutils-2.17/./opcodes/h8300-dis.c:669: warning: initialization discards qualifiers from pointer target type

	make[4]: *** [h8300-dis.lo] Error 1

	the solution:

	in line binutils-2.17/./opcodes/h8300-dis.c:365
	change
	op_type *nib = q->data.nib;
	to
	const op_type *nib = q->data.nib;

	line 643
	change
	op_type *args = q->args.nib;
	to
	const op_type *args = q->args.nib;

	line 669
	change
	op_type *args = q->args.nib;
	to
	const op_type *args = q->args.nib;
*/

#ifdef USING_LIBBFD

UVDBFDLibrary::UVDBFDLibrary()
{
}

UVDBFDLibrary::~UVDBFDLibrary()
{
	deinit();
}

uv_err_t UVDBFDLibrary::init()
{
	doInit();
	return UV_ERR_OK;
}

uv_err_t UVDBFDLibrary::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDBFDLibrary::prepareFLIRTAnalysis(UVDFLIRTPatternAnalysis *flirtEntries)
{
	return UV_DEBUG(UV_ERR_UNSUPPORTED);
}

uv_err_t UVDBFDLibrary::canParse(UVDData *data, uint32_t *canParse)
{
	*canParse = false;
	return UV_ERR_OK;
}

#endif
