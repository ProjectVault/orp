#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([encryptor demo for ORP], [0.1], [], [orp-encryptor])
AC_CONFIG_SRCDIR([src/encryptor.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Add a debug mode. TODO: disable by default when proj is stable */
AC_MSG_CHECKING(whether to enable debugging)
AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug], [Enable debugging @<:@yes@:>@])],
  [
    case "$enableval" in
      yes) enable_debug=yes ;;
    esac
  ],
  
  [enable_debug=yes]
)
AC_MSG_RESULT(${enable_debug-no})
AC_ARG_ENABLE([ffstest],
              [AS_HELP_STRING([--enable-ffstest], [Turn on FFS unit test @<:@no@:>@])],
              [case "${enableval}" in
                yes) ffstest=true ;;
                no)  ffstest=false ;;
                *) AC_MSG_ERROR([bad value ${enableval} for --enable-ffs-test]) ;;
              esac],[ffstest=false]
              )

# Filter out default CFLAGS
CFLAGS=${CFLAGS/-g/}
CFLAGS=${CFLAGS/-O2/}

AM_CONDITIONAL([DEBUG], [test x$enable_debug = xyes])

# Figure out target arch. TODO: also break down output by target board 
case $host in
  *arm*)   AC_SUBST( OS_ARCH, [arm] )      
  		   AC_DEFINE( [BUILD_ARCH_ARM], [arm], [Include ARM features])
		   arm=true
		   CFLAGS="$CFLAGS -nostartfiles" # fails CC check w/o this
		   ;;
  *or1k*)  AC_SUBST( OS_ARCH, [openrisc])
  		   AC_DEFINE( [BUILD_ARCH_OPENRISC], [openrisc], [Include OPENRISC functionality])
		   openrisc=true
		   CFLAGS="$CFLAGS -ffixed-r31" # -nostdlib"
		   ;;
  *)       AC_ERROR( This is not a supported architecture. Please set --host accordingly. )
  		   ;;
esac
AM_CONDITIONAL([ARM], [test x$arm = xtrue])
AM_CONDITIONAL([OPENRISC], [test x$openrisc = xtrue])


# Checks for programs.
AC_PROG_CC
#AC_PROG_RANLIB
AM_PROG_AS
AM_PROG_AR
LT_INIT([disable-shared])


# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h strings.h sys/param.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 include/Makefile])
AC_OUTPUT
