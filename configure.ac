AC_INIT([macsad], [0.1])
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
MAC_CFLAGS="$MAC_CFLAGS -W -Wall -Werror"

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
MAC_CHECK_CFLAG([-Wold-style-definition])
MAC_CHECK_CFLAG([-Wpointer-arith])
MAC_CHECK_CFLAG([-Wcast-align])
MAC_CHECK_CFLAG([-Wnested-externs])
MAC_CHECK_CFLAG([-Wcast-qual])
MAC_CHECK_CFLAG([-Wformat-nonliteral])
MAC_CHECK_CFLAG([-Wformat-security])
MAC_CHECK_CFLAG([-Wundef])
MAC_CHECK_CFLAG([-Wwrite-strings])
MAC_CHECK_CFLAG([-Wformat-truncation=0])
MAC_CHECK_CFLAG([-Wformat-overflow=0])

MAC_CFLAGS="$MAC_CFLAGS -std=c99"
MAC_CXXFLAGS="$MAC_CXXFLAGS -std=c++11"

# Extra flags for example to suppress certain warning types
MAC_CFLAGS="$MAC_CFLAGS $MAC_CFLAGS_EXTRA"

# Check if compiler supports cmpxchng16 on x86-based architectures
case "${host}" in
  i?86? | x86*)
  if test "${CC}" != "gcc" -o ${CC_VERSION_MAJOR} -ge 5; then
     MAC_CHECK_CFLAG([-mcx16])
  fi
  ;;
esac

# Default include setup
CFLAGS="$CFLAGS $ODP_CFLAGS"
CXXFLAGS="$CXXFLAGS $ODP_CXXFLAGS"

AC_ARG_ENABLE([looom],
    AS_HELP_STRING([--enable-looom], [Enable Loooom agent support in the MACSAD switch]))
AC_ARG_ENABLE([odp-dpdk],
    AS_HELP_STRING([--enable-odp-dpdk], [Compile with dpdk optimized reference ODP]))
AS_IF([test "x$enable_looom" = "xyes"], [
    AC_DEFINE([LOOOM_AGENT], [1], [Define Looom agent support])
])


PKG_CHECK_MODULES([libodp], [libodp-linux = 1.16.0.0])
PKG_CHECK_MODULES([libodphelper], [libodphelper = 1.16.0.0])
#PKG_CHECK_MODULES([libodp], [libodp-dpdk = 1.15.0.0])
#PKG_CHECK_MODULES([libodphelper], [libodphelper = 1.15.0.0])
AX_PTHREAD(, [AC_MSG_ERROR([pthread support is required])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_MSG_RESULT([
    $PACKAGE $VERSION

    ODP Library version:    ${ODP_LIBSO_VERSION}
    Helper Library version: ${ODPHELPER_LIBSO_VERSION}

    implementation_name:    ${IMPLEMENTATION_NAME}
    host:           ${host}
    sysconfdir:     ${sysconfdir}
    libdir:         ${libdir}
    includedir:     ${includedir}

    cc:         ${CC}
    cc version:             ${CC_VERSION}
    cppflags:       ${CPPFLAGS}
    cflags:         ${CFLAGS}
    ldflags:        ${LDFLAGS}
    libs:           ${LIBS}
    defs:           ${DEFS}
])
