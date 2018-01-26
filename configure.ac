AC_INIT([macsad], [0.2])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])

#Check for programs:
AC_PROG_CC

# Checks for library functions:
AC_CHECK_FUNCS([malloc memset socket])

# Checks for header files:
AC_CHECK_HEADERS([stdint.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

#########################################################################
# Get GCC version
#########################################################################

AX_COMPILER_VERSION
CC_VERSION=$ax_cv_c_compiler_version

CC_VERSION_MAJOR=$(echo $CC_VERSION | cut -d'.' -f1)
CC_VERSION_MINOR=$(echo $CC_VERSION | cut -d'.' -f2)
CC_VERSION_PATCH=$(echo $CC_VERSION | cut -d'.' -f3)

# Set correct pkgconfig version
PKGCONFIG_VERSION=$(echo $VERSION | awk -F '.git' '{print $1}')
AC_SUBST(PKGCONFIG_VERSION)

# Default warning setup
#MAC_CFLAGS="$MAC_CFLAGS -W -Wall -Werror"
MAC_CFLAGS="$MAC_CFLAGS -W -Wall"

dnl Use -Werror in the checks below since Clang emits a warning instead of
dnl an error when it encounters an unknown warning option.
AC_DEFUN([MAC_CHECK_CFLAG],
     [AX_CHECK_COMPILE_FLAG([$1],
                [MAC_CFLAGS="$MAC_CFLAGS $1"],
                [], [-W -Wall -Werror],
                [AC_LANG_SOURCE([int main(void)
                        {return 0;}])])])
MAC_CHECK_CFLAG([-Wstrict-prototypes])
MAC_CHECK_CFLAG([-Wmissing-prototypes])
MAC_CHECK_CFLAG([-Wmissing-declarations])
MAC_CHECK_CFLAG([-Wpointer-arith])

MAC_CFLAGS="$MAC_CFLAGS -std=c99"
MAC_CXXFLAGS="$MAC_CXXFLAGS -std=c++11"

# Extra flags for example to suppress certain warning types
MAC_CFLAGS="$MAC_CFLAGS $MAC_CFLAGS_EXTRA"

# Default include setup 
# Need to fix this later.
CFLAGS="$CFLAGS $ODP_CFLAGS"
CXXFLAGS="$CXXFLAGS $ODP_CXXFLAGS"

AC_ARG_ENABLE([looom],
    AS_HELP_STRING([--enable-looom], [Enable Loooom agent support in the MACSAD switch]))
AS_IF([test "x$enable_looom" = "xyes"], [
    AC_DEFINE([LOOOM_AGENT], [1], [Define Looom agent support])
])

AC_ARG_ENABLE([static],
    AS_HELP_STRING([--enable-static], [Enable static libs in the MACSAD switch]),
    [AS_CASE(${enableval}, [yes], [], [no], [],
		    [AC_MSG_ERROR([bad value ${enableval} for --enable-static])])],
	[enable_static=no])
AM_CONDITIONAL([MAC_STATIC], [test x$enable_static = xyes])

AC_ARG_ENABLE([odpd],
    AS_HELP_STRING([--enable-odpd], [Compile with dpdk optimized reference ODP]),
    [AS_CASE(${enableval}, [yes], [], [no], [],
		    [AC_MSG_ERROR([bad value ${enableval} for --enable-odpd])])],
	[enable_odpd=no])
AM_CONDITIONAL([MAC_ODPD], [test x$enable_odpd = xyes])

AC_ARG_ENABLE([odpn],
    AS_HELP_STRING([--enable-odpn], [Compile with dpdk optimized reference ODP]),
    [AS_CASE(${enableval}, [yes], [], [no], [],
		    [AC_MSG_ERROR([bad value ${enableval} for --enable-odpn])])],
	[enable_odpn=no])
AM_CONDITIONAL([MAC_ODPN], [test x$enable_odpn = xyes])

AC_ARG_ENABLE([odp-dpdk],
    AS_HELP_STRING([--enable-odp-dpdk], [Compile with dpdk optimized reference ODP]),
    [AS_CASE(${enableval}, [yes], [], [no], [],
		    [AC_MSG_ERROR([bad value ${enableval} for --enable-odp-dpdk])])],
	[enable_odp_dpdk=no])
AM_CONDITIONAL([MAC_ODP_DPDK], [test x$enable_odp_dpdk = xyes])

AC_DEFUN([MAC_PKG_CHECK_MODULES],
[AC_REQUIRE([PKG_PROG_PKG_CONFIG])dnl
_save_PKG_CONFIG=$PKG_CONFIG
AS_IF([test "x$enable_static" = "xyes"], [
	PKG_CONFIG="$PKG_CONFIG --static"
])
#PKG_CONFIG="$PKG_CONFIG --static"
PKG_CHECK_MODULES([$1], [$2], [$3], [$4], [$5])
PKG_CONFIG=$_save_PKG_CONFIG[]dnl
])

AS_IF([test "x$enable_odp_dpdk" = "xyes"], [
    echo "macsad(odp_dpdk) is SET"
#	DPDK_CFLAGS  += -I "$(RTE_SDK)/$(RTE_TARGET)/include"
#	DPDK_LDFLAGS += -L$(RTE_SDK)/x86_64-native-linuxapp-gcc/lib
 	DPDK_LIBS="-lrt -lcrypto -ldl -lpcap -Wl,--whole-archive,-ldpdk,--no-whole-archive -ldl -lm -lpcap"
    #PKG_CHECK_MODULES([libodp], [libodp-dpdk = 1.15.0.0])
    #PKG_CHECK_MODULES([libodphelper], [libodphelper = 1.15.0.0])
    MAC_PKG_CHECK_MODULES([libodp], [libodp-dpdk = 1.15.0.0])
    MAC_PKG_CHECK_MODULES([libodphelper], [libodphelper = 1.15.0.0])
])

AS_IF([test "x$enable_odpd" = "xyes"], [
    echo "macsad(odp) is SET"
#	DPDK_LDFLAGS += -L$(RTE_SDK)/x86_64-native-linuxapp-gcc/lib
    DPDK_LIBS="-lrt -lcrypto -ldl -lpcap"
#   DPDK_LIBS="-lrt -lcrypto -ldl -lpcap -Wl,--whole-archive,-ldpdk,--no-whole-archive -ldl -lm -lpcap"
#   PKG_CHECK_MODULES([libodp], [libodp-linux = 1.16.0.0])
#	PKG_CHECK_MODULES([libodphelper], [libodphelper = 1.16.0.0])
    MAC_PKG_CHECK_MODULES([libodp], [libodp-linux = 1.16.0.0])
    MAC_PKG_CHECK_MODULES([libodphelper], [libodphelper = 1.16.0.0])
])

AS_IF([test "x$enable_odp_dpdk" != "xyes"] && [test "x$enable_odpd" != "xyes"], [
    echo "macsad(socket_mmap/netmap) is SET"
#   PKG_CHECK_MODULES([libodp], [libodp-linux = 1.16.0.0])
#	PKG_CHECK_MODULES([libodphelper], [libodphelper = 1.16.0.0])
    MAC_PKG_CHECK_MODULES([libodp], [libodp-linux = 1.16.0.0])
    MAC_PKG_CHECK_MODULES([libodphelper], [libodphelper = 1.16.0.0])
])

AX_PTHREAD(, [AC_MSG_ERROR([pthread support is required])])

AC_CONFIG_FILES([Makefile])

# distribute the changed variables among the Makefiles
AC_SUBST([CFLAGS])
#AC_SUBST([DPDK_LDFLAGS])
#AC_SUBST([DPDK_CFLAGS])
AC_SUBST([MAC_ODPD])
AC_SUBST([MAC_ODPN])
AC_SUBST([MAC_ODP_DPDK])
AC_SUBST([DPDK_LIBS])

AC_OUTPUT
AC_MSG_RESULT([
    $PACKAGE $VERSION

    host:           ${host}
    cc:		        ${CC}
    cc version:     ${CC_VERSION}
    cflags:         ${CFLAGS}
    odp_libs:       ${libodp_LIBS}
    helper_libs:    ${libodphelper_LIBS}
    dpdk_libs:      ${DPDK_LIBS}
])
