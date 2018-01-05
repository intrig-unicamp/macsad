AC_INIT([macsad], [0.1])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC

AC_ARG_ENABLE([looom],
    AS_HELP_STRING([--enable-looom], [Enable Loooom agent support in the MACSAD switch]))
AS_IF([test "x$enable_looom" = "xyes"], [
    AC_DEFINE([LOOOM_AGENT], [1], [Define Looom agent support])
])

PKG_CHECK_MODULES([libodp], [libodp-linux = 1.16.0.0])
PKG_CHECK_MODULES([libodphelper], [libodphelper = 1.16.0.0])
AX_PTHREAD(, [AC_MSG_ERROR([pthread support is required])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
