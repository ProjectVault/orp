#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([MicroSEL Embedded Operating System], [0.3], [], [mselOS])
AC_CONFIG_SRCDIR([src/os/system.c])
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

AC_ARG_ENABLE([sf2build],
              [AS_HELP_STRING([--enable-sf2build], [Enable build options for SF2 @<:@no@:>@])],
			  [case "${enableval}" in
			    yes) sf2build=true
					 AC_DEFINE( [SF2_BUILD], [1], [Include ARM/SmartFusion2 platform support])
				     ;;
				no)  sf2build=false ;;
				*) AC_MSG_ERROR([bad value ${enableval} for --enable-sf2build]) ;;
			  esac],[sf2build=false]
			  )

AC_ARG_ENABLE([ffstest],
              [AS_HELP_STRING([--enable-ffstest], [Turn on FFS unit test @<:@no@:>@])],
              [case "${enableval}" in
                yes) ffstest=true ;;
                no)  ffstest=false ;;
                *) AC_MSG_ERROR([bad value ${enableval} for --enable-ffstest]) ;;
              esac],[ffstest=false]
              )

AC_ARG_ENABLE([swaes],
              [AS_HELP_STRING([--enable-swaes], [Turn on software AES @<:@no@:>@])],
              [case "${enableval}" in
                yes) swaes=true ;;
                no)  swaes=false ;;
                *) AC_MSG_ERROR([bad value ${enableval} for --enable-swaes]) ;;
              esac],[swaes=false]
              )


AC_ARG_ENABLE([swsha],
              [AS_HELP_STRING([--enable-swsha], [Turn on software SHA @<:@no@:>@])],
              [case "${enableval}" in
                yes) swsha=true ;;
                no)  swsha=false ;;
                *) AC_MSG_ERROR([bad value ${enableval} for --enable-swsha]) ;;
              esac],[swsha=false]
              )

AC_ARG_ENABLE([swecc],
              [AS_HELP_STRING([--enable-swecc], [Turn on software ECC @<:@yes@:>@])],
              [case "${enableval}" in
                yes) swecc=true ;;
                no)  swecc=false ;;
                *) AC_MSG_ERROR([bad value ${enableval} for --enable-swecc]) ;;
              esac],[swecc=true]
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
AM_CONDITIONAL([SF2BUILD], [test x$sf2build = xtrue])
AM_CONDITIONAL([SW_AES],   [test x$swaes = xtrue])
AM_CONDITIONAL([SW_SHA],   [test x$swsha = xtrue])
AM_CONDITIONAL([SW_ECC],   [test x$swecc = xtrue])
AM_CONDITIONAL([FFS_TEST], [test x$ffstest = xtrue])


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
                 src/arch/Makefile
				 src/arch/openrisc/Makefile
				 src/arch/arm/Makefile
				 src/lib/Makefile
				 src/driver/Makefile
				 src/os/Makefile
                 include/Makefile
				 tests/Makefile
				 tests/common/build_vars])
AC_CONFIG_LINKS([tests/common/test_harness.sh:tests/common/test_harness.sh
                 tests/aes_test.expect:tests/aes_test.expect
                 tests/ecc_test.expect:tests/ecc_test.expect
                 tests/ffs_session.expect:tests/ffs_session.expect
                 tests/sha_test.expect:tests/sha_test.expect
                 tests/task_malloc.expect:tests/task_malloc.expect
                 tests/task_stack_overflow.expect:tests/task_stack_overflow.expect
                 tests/uart_test.expect:tests/uart_test.expect
                 tests/yield_loop.expect:tests/yield_loop.expect
                ])
AC_OUTPUT
