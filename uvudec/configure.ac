# UVNet Universal Decompiler (uvudec)
# Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under GPL V3+, see COPYING for details

# It can be easily argued that autoconf is an archaic piece of software
# and I should use CMake sort of stuff instead
# This may be the case, but this was better than nothing and its good to know
# WHY I'd prefer CMake instead of "it looked better" than the long standing
# traditionional tools

AC_PREREQ([2.65])
# FIXME: copy this into Makefile.configure, split apart
AC_INIT([uvudec], [0.3.1], [JohnDMcMaster@gmail.com])
AC_CONFIG_SRCDIR([libuvudec/uvd/core/uvd.cpp])
AC_CONFIG_HEADERS([libuvudec/uvd/config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lQtCore':
AC_CHECK_LIB([QtCore], [main])
# FIXME: Replace `main' with a function in `-lQtGui':
AC_CHECK_LIB([QtGui], [main])
# FIXME: Replace `main' with a function in `-lcppunit':
AC_CHECK_LIB([cppunit], [main])
# FIXME: Replace `main' with a function in `-ldl':
AC_CHECK_LIB([dl], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_CHECK_HEADERS([float.h limits.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
# I don't use this
#AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor gettimeofday memset mkdir strdup strerror strstr strtol])

# USING_UVUDEC=@enable_uvudec@
AC_ARG_ENABLE([development],
    AS_HELP_STRING([--enable-development], [Enable set of options useful for development]))
AS_IF([test "x$enable_development" != "xyes"], [
	enable_development=N
], [
	enable_development=Y
	AC_DEFINE([UVD_ENABLE_DEVELOPMENT], [], [enable developer friendly options])
])
AC_SUBST(enable_development)

# USING_UVUDEC=@enable_uvudec@
AC_ARG_ENABLE([uvudec],
    AS_HELP_STRING([--disable-uvudec], [Disable building uvudec]))
AS_IF([test "x$enable_uvudec" != "xno"], [
	enable_uvudec=Y
])
AC_SUBST(enable_uvudec)
AC_DEFINE([enable_uvudec], [], [decompiler program])

# USING_BINTOOBJ=@enable_bin2obj@
AC_ARG_ENABLE([bin2obj],
    AS_HELP_STRING([--disable-bin2obj], [Disable building bin2obj]))
AS_IF([test "x$enable_bin2obj" != "xno"], [
	enable_bin2obj=Y
])
AC_SUBST(enable_bin2obj)
AC_DEFINE([enable_bin2obj], [], [binary to object program])

# USING_OBJ2PAT=@enable_obj2pat@
AC_ARG_ENABLE([obj2pat],
    AS_HELP_STRING([--disable-obj2pat], [Disable building obj2pat]))
AS_IF([test "x$enable_obj2pat" != "xno"], [
	enable_obj2pat=Y
])
AC_SUBST(enable_obj2pat)
AC_DEFINE([enable_obj2pat], [], [object to .pat program])

# USING_PAT2SIG=@enable_pat2sig@
AC_ARG_ENABLE([pat2sig],
    AS_HELP_STRING([--disable-pat2sig], [Disable building pat2sig]))
AS_IF([test "x$enable_pat2sig" != "xno"], [
	enable_pat2sig=Y
])
AC_SUBST(enable_pat2sig)
AC_DEFINE([enable_pat2sig], [], [.pat to .sig program])

# USING_FLIRTUTIL=@enable_flirtutil@
AC_ARG_ENABLE([flirtutil],
    AS_HELP_STRING([--disable-flirtutil], [Disable building flirtutil]))
AS_IF([test "x$enable_flirtutil" != "xno"], [
	enable_flirtutil=Y
])
AC_SUBST(enable_flirtutil)
AC_DEFINE([enable_flirtutil], [], [binary to object program])

# USING_GUI=@enable_GUI@
AC_ARG_ENABLE([GUI],
    AS_HELP_STRING([--disable-GUI], [Disable building GUI]))
AS_IF([test "x$enable_GUI" != "xno"], [
	enable_GUI=Y
])
AC_SUBST(enable_GUI)
AC_DEFINE([enable_GUI], [], [graphical user interface])

# Dependencies
# FIXME: I don't think I'm doing this correctly, but it works for now

# Qt
AC_ARG_ENABLE([qt-prefix],
	AS_HELP_STRING([--enable-qt-prefix], [Set Qt installed dir prefix]),
	[
		qt_prefix=$enableval
	],
	[qt_prefix=/opt/qtsdk-2010.04])
AC_SUBST(qt_prefix)
#AC_DEFINE([qt_prefix], [], [Qt installed dir])

# X11 (Qt dep)
AC_ARG_ENABLE([x11r6-lib-dir],
	AS_HELP_STRING([--enable-x11r6-lib-dir], [Set X11R6 lib dir]),
	[
		x11r6_lib_dir=$enableval
	],
	[x11r6_lib_dir=/usr/X11R6/lib])
AC_SUBST(x11r6_lib_dir)
#AC_DEFINE([x11r6_lib_dir], [], [X11R6 lib dir])

# binutils
AC_ARG_ENABLE([binutils-prefix],
	AS_HELP_STRING([--enable-binutils-prefix], [Set binutils installed dir prefix]),
	[
		binutils_prefix=$enableval
	],
	[binutils_prefix=/])
AC_SUBST(binutils_prefix)
#AC_DEFINE([binutils_prefix], [], [binutils installed dir])

# Python
AC_ARG_ENABLE([python-include-dir],
	AS_HELP_STRING([--enable-python-include-dir], [Set directory with Python.h]),
	[
		python_include_dir=$enableval
	],
	[python_include_dir=/usr/include/python2.6])
AC_SUBST(python_include_dir)
#AC_DEFINE([python_include_dir], [], [Python.h dir])

# Default decompile file
AC_ARG_ENABLE([default-decompile-file],
	AS_HELP_STRING([--enable-default-decompile-file], [Set default file to decompile]),
	[
		default_decompile_file=$enableval
	],
	[default_decompile_file=])
AC_SUBST(default_decompile_file)
#AC_DEFINE([default_decompile_file], [], [Default decompiled file])

# Default decompile file
AC_ARG_ENABLE([uvdasm-default-cpu-file],
	AS_HELP_STRING([--enable-uvdasm-default-cpu-file=<val>], [Set uvdasm plugin's default architecture file]),
	[
		uvdasm_default_cpu_file=$enableval
	],
	[uvdasm_default_cpu_file=8051/8051.op])
AC_SUBST(uvdasm_default_cpu_file)
#AC_DEFINE([uvdasm_default_cpu_file], [], [Default uvdasm plugin architecture file])

# Default decompile file
AC_ARG_ENABLE([install_dir],
	AS_HELP_STRING([--enable-install-dir=<val>], [Set install directory]),
	[
		install_dir=$enableval
	],
	[install_dir=\$\(PREFIX\)/opt/\$\(PACKAGE_NAME\)/\$\(PACKAGE_VERSION\)])
AC_SUBST(install_dir)
#AC_DEFINE([install_dir], [], [install directory])



AC_CONFIG_FILES([Makefile.configure Makefile.version libuvudec/libuvudec.pc])
AC_OUTPUT

