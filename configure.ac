dnl Based on Xpdf configure.in and evince configure.ac

AC_PREREQ(2.59)
AC_INIT(poppler, 0.1.1)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_CONFIG_HEADER(poppler/poppler-config.h)

dnl ##### Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CC
AC_ISC_POSIX
AC_PROG_CC_STDC
#if test -z "$CXX" -a "$CC" = "gcc"; then
#  CXX="gcc"
#fi
AC_PROG_CXX
AC_PROG_INSTALL

dnl ##### Optional features.
AH_TEMPLATE([A4_PAPER],
            [Use A4 paper size instead of Letter for PostScript output.])
AC_ARG_ENABLE(a4-paper,
	      AS_HELP_STRING([--enable-a4-paper],
			     [use A4 paper size instead of Letter for PostScript output]),
              AC_DEFINE(A4_PAPER))

AH_TEMPLATE([OPI_SUPPORT],
            [Include support for OPI comments.])
AC_ARG_ENABLE(opi,
              AS_HELP_STRING([--enable-opi],
			     [include support for OPI comments]),
              AC_DEFINE(OPI_SUPPORT))

dnl Enable these unconditionally.
AC_DEFINE([MULTITHREADED], [1], [Enable multithreading support.])
AC_DEFINE([TEXTOUT_WORD_LIST], [1], [Enable word list support.])

dnl ##### Path to xpdfrc.
dnl This ugly kludge to get the sysconfdir path is needed because
dnl autoconf doesn't actually set the prefix variable until later.
if test "$sysconfdir" = '${prefix}/etc'; then
  if test "x$prefix" = xNONE; then
    system_xpdfrc="$ac_default_prefix/etc/xpdfrc"
  else
    system_xpdfrc="$prefix/etc/xpdfrc"
  fi
else
  system_xpdfrc="$sysconfdir/xpdfrc"
fi
AC_DEFINE_UNQUOTED(SYSTEM_XPDFRC, "$system_xpdfrc",
                   [Full path for the system-wide xpdfrc file.])

dnl ##### Checks for header files.
AC_PATH_XTRA
AC_HEADER_DIRENT

dnl ##### Switch over to C++.  This will make the checks below a little
dnl ##### bit stricter (requiring function prototypes in include files).
dnl ##### (99% of xpdf is written in C++.)
AC_LANG_CPLUSPLUS

dnl ##### Check for extra libraries needed by X.  (LynxOS needs this.)
AC_CHECK_FUNC(gethostbyname)
if test $ac_cv_func_gethostbyname = no; then
  AC_CHECK_LIB(bsd, gethostbyname, X_EXTRA_LIBS="$X_EXTRA_LIBS -lbsd")
fi

dnl ##### Checks for library functions.
AC_CHECK_FUNCS(popen)

dnl ##### Back to C for the library tests.
AC_LANG_C

dnl ##### Check for fseeko/ftello or fseek64/ftell64
dnl The LARGEFILE and FSEEKO macros have to be called in C, not C++, mode.
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO
AC_CHECK_FUNCS(fseek64, xpdf_cv_func_fseek64=yes, xpdf_cv_func_fseek64=no)
AC_CHECK_FUNCS(ftell64, xpdf_cv_func_ftell64=yes, xpdf_cv_func_ftell64=no)
if test "$xpdf_cv_func_fseek64" = yes -a "$xpdf_cv_func_ftell64" = yes; then
  AC_DEFINE(HAVE_FSEEK64)
fi

dnl Check for freetype headers
FREETYPE_LIBS=
FREETYPE_CFLAGS=
AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
if test "x$FREETYPE_CONFIG" != "xno" ; then
  FREETYPE_CFLAGS=`$FREETYPE_CONFIG --cflags`
  FREETYPE_LIBS=`$FREETYPE_CONFIG --libs`
  AC_DEFINE(HAVE_FREETYPE_H, 1, [Have FreeType2 include files])

  vers=`$FREETYPE_CONFIG --version 2>/dev/null | sed -e 's/libfreetype //' | awk 'BEGIN { FS = "."; } { printf "%d", ($1 * 1000 + $2) * 1000 + $3;}'`
  if test -n "$vers" && test "$vers" -le 9005003; then
    AC_DEFINE_UNQUOTED(HAVE_FREETYPE_217_OR_OLDER, 1, [Defines if your system has the freetype library 2.1.7 or older])
  else
    AC_DEFINE_UNQUOTED(HAVE_FREETYPE_217_OR_OLDER, 0, [Defines if your system has the freetype library 2.1.7 or older])
  fi

fi

AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

AC_ARG_ENABLE(splash-output,
              AC_HELP_STRING([--disable-splash-output],
	                     [Don't build the Splash graphics backend.]),,
              enable_splash_output="yes")
AM_CONDITIONAL(BUILD_SPLASH_OUTPUT, test x$enable_splash_output = xyes)

AC_ARG_ENABLE(cairo-output,
              AC_HELP_STRING([--disable-cairo-output],
                             [Don't build the cairo graphics backend.]),
              enable_cairo_output=$enableval,
              enable_cairo_output="try")
if test x$enable_cairo_output = xyes; then
  PKG_CHECK_MODULES(CAIRO, cairo >= 0.3)
elif test x$enable_cairo_output = xtry; then
  PKG_CHECK_MODULES(CAIRO, cairo >= 0.3,
                    [enable_cairo_output="yes"],
                    [enable_cairo_output="no"])
fi
AM_CONDITIONAL(BUILD_CAIRO_OUTPUT, test x$enable_cairo_output = xyes)

AC_ARG_ENABLE(gtk-test,
              AC_HELP_STRING([--disable-gtk-test],
	                     [Don't compile GTK+ test program.]),
              enable_gtk_test=$enableval,
              enable_gtk_test="try")
if test x$enable_gtk_test = xyes; then
  PKG_CHECK_MODULES(GTK_TEST, gtk+-2.0 >= 2.2.0)
elif test x$enable_gtk_test = xtry; then
  PKG_CHECK_MODULES(GTK_TEST, gtk+-2.0 >= 2.2.0,
                    [enable_gtk_test="yes"],
                    [enable_gtk_test="no"])
fi
AM_CONDITIONAL(BUILD_GTK_TEST, test x$enable_gtk_test = xyes)


AC_ARG_ENABLE(compile-warnings,
              AC_HELP_STRING([--enable-compile-warnings=@<:@no/yes/kde@:>@]
                             [Turn on compiler warnings.]),,
              [enable_compile_warnings="yes"])

if test "x$GCC" != xyes; then
  enable_compile_warnings=no
fi
case "$enable_compile_warnings" in
  no)	;;
  yes)	CXXFLAGS="-Wall -Wno-unused $CXXFLAGS" ;;
  kde)	CXXFLAGS="-Wnon-virtual-dtor -Wno-long-long -Wundef -ansi \
		-D_XOPEN_SOURCE=500 -D_BSD_SOURCE -Wcast-align \
		-Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith \
		-Wwrite-strings -O2 -Wformat-security \
		-Wmissing-format-attribute -fno-exceptions -fno-check-new \
		-fno-common $CXXFLAGS" ;;
esac


AC_OUTPUT([
Makefile
goo/Makefile
fofi/Makefile
splash/Makefile
poppler/Makefile
test/Makefile
poppler.pc
poppler-cairo.pc
poppler-splash.pc])


echo ""
echo "Building poppler with support for:"
echo "  splash output: $enable_splash_output"
echo "  cairo output: $enable_cairo_output"
