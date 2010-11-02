#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([uvudec], [3.0.0], [johndmcmaster@gmail.com])
AC_CONFIG_SRCDIR([plugin/uvdstrings/main.cpp])
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

AC_CONFIG_FILES([Makefile.configure libuvudec/libuvudec.pc])
AC_OUTPUT
