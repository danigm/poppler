dnl Based on Xpdf configure.in and evince configure.ac

AC_PREREQ(2.59)
AC_INIT(poppler, 0.3.0)
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
AC_CHECK_FUNCS(popen mkstemp mkstemps)

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


dnl Test for libjpeg
AC_ARG_ENABLE(libjpeg,
	      AC_HELP_STRING([--disable-libjpeg],
	                     [Don't build against libjpeg.]),
              enable_libjpeg=$enableval,
              enable_libjpeg="try")
if test x$enable_libjpeg = xyes; then
  AC_CHECK_LIB([jpeg], [jpeg_destroy_decompress],,
               AC_MSG_ERROR("*** libjpeg library not found ***))
  AC_CHECK_HEADERS([jpeglib.h],,
                   AC_MSG_ERROR("*** libjpeg headers not found ***))
elif test x$enable_libjpeg = xtry; then
  AC_CHECK_LIB([jpeg], [jpeg_destroy_decompress],
  	       [enable_libjpeg="yes"],
	       [enable_libjpeg="no"])
  AC_CHECK_HEADERS([jpeglib.h],,
		   [enable_libjpeg="no"])
fi

if test x$enable_libjpeg = xyes; then
  LIBJPEG_LIBS="-ljpeg"
  AC_SUBST(LIBJPEG_LIBS)
  AC_DEFINE(ENABLE_LIBJPEG)
fi

AM_CONDITIONAL(BUILD_LIBJPEG, test x$enable_libjpeg = xyes)
AH_TEMPLATE([ENABLE_LIBJPEG],
            [Use libjpeg instead of builtin jpeg decoder.])


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
if test x$enable_splash_output = xyes; then
  PKG_CHECK_MODULES(SPLASH, fontconfig)
fi
AM_CONDITIONAL(BUILD_SPLASH_OUTPUT, test x$enable_splash_output = xyes)
AH_TEMPLATE([HAVE_SPLASH], [Use splash for rendering.])
if test x$enable_splash_output = xyes; then
  AC_DEFINE(HAVE_SPLASH)
fi

AC_ARG_ENABLE(cairo-output,
              AC_HELP_STRING([--disable-cairo-output],
                             [Don't build the cairo graphics backend.]),
              enable_cairo_output=$enableval,
              enable_cairo_output="try")
if test x$enable_cairo_output = xyes; then
  PKG_CHECK_MODULES(CAIRO, cairo = 0.4.0 fontconfig)
elif test x$enable_cairo_output = xtry; then
  PKG_CHECK_MODULES(CAIRO, cairo = 0.4.0 fontconfig,
                    [enable_cairo_output="yes"],
                    [enable_cairo_output="no"])
fi

AM_CONDITIONAL(BUILD_CAIRO_OUTPUT, test x$enable_cairo_output = xyes)
AH_TEMPLATE([HAVE_CAIRO], [Use cairo for rendering.])
if test x$enable_cairo_output = xyes; then
  AC_DEFINE(HAVE_CAIRO)
fi

AC_ARG_ENABLE(poppler-glib,
              AC_HELP_STRING([--disable-poppler-glib],
	                     [Don't compile poppler glib wrapper.]),
              enable_poppler_glib=$enableval,
              enable_poppler_glib="try")
if test x$enable_poppler_glib = xyes; then
  PKG_CHECK_MODULES(POPPLER_GLIB, gdk-pixbuf-2.0 fontconfig)
elif test x$enable_poppler_glib = xtry; then
  PKG_CHECK_MODULES(POPPLER_GLIB, gdk-pixbuf-2.0 fontconfig,
                    [enable_poppler_glib="yes"],
                    [enable_poppler_glib="no"])
fi
AM_CONDITIONAL(BUILD_POPPLER_GLIB, test x$enable_poppler_glib = xyes)

AC_ARG_ENABLE(poppler-qt,
	      AC_HELP_STRING([--disable-poppler-qt],
			     [Don't compile poppler qt wrapper.]),
	      enable_poppler_qt=$enableval,
	      enable_poppler_qt="try")

have_qt=no
dnl ===================================================================
dnl Check whether the Qt libraries are available.
dnl Adapted from OpenOffice.org configure.in
dnl ===================================================================

dnl Search paths for Qt 
qt_incdirs="$QTINC /usr/local/qt/include /usr/include/qt /usr/include /usr/X11R6/include/X11/qt /usr/X11R6/include/qt /usr/lib/qt3/include /usr/lib/qt/include /usr/share/qt3/include"
qt_libdirs="$QTLIB /usr/local/qt/lib /usr/lib/qt /usr/lib /usr/X11R6/lib/X11/qt /usr/X11R6/lib/qt /usr/lib/qt3/lib /usr/lib/qt/lib /usr/share/qt3/lib"
if test -n "$QTDIR" ; then
    qt_incdirs="$QTDIR/include $qt_incdirs"
    qt_libdirs="$QTDIR/lib $qt_libdirs"
fi

dnl What to test
qt_test_include="qstyle.h"
qt_test_library="libqt-mt.la"

dnl Check for Qt headers
AC_MSG_CHECKING([for Qt headers])
qt_incdir="no"
for it in $qt_incdirs ; do
    if test -r "$it/$qt_test_include" ; then
        qt_incdir="$it"
        break
    fi
done
AC_MSG_RESULT([$qt_incdir])

dnl Check for Qt libraries
AC_MSG_CHECKING([for Qt libraries])
qt_libdir="no"
for qt_check in $qt_libdirs ; do
    if test -r "$qt_check/$qt_test_library" ; then
        qt_libdir="$qt_check"
        break
    fi
done
AC_MSG_RESULT([$qt_libdir])

if test "x$qt_libdir" != "xno" ; then
    if test "x$qt_incdir" != "xno" ; then
        have_qt=yes;
    fi
fi

dnl Set the variables
POPPLER_QT_CXXFLAGS="-I$qt_incdir"
POPPLER_QT_LIBS="$qt_libdir/$qt_test_library"

AC_SUBST(POPPLER_QT_CXXFLAGS)
AC_SUBST(POPPLER_QT_LIBS)

if test x$have_qt = xno ; then
    AC_MSG_WARN([Qt development libraries not found])
fi

if test x$enable_poppler_qt = xyes; then
  if test x$have_qt = xno; then
    AC_MSG_ERROR([Qt integration explicitly required, and Qt libraries not found])
  fi
elif test x$enable_poppler_qt = xtry; then
  if test x$have_qt = xyes; then
    enable_poppler_qt="yes"
  else
    enable_poppler_qt="no"
  fi
fi
AM_CONDITIONAL(BUILD_POPPLER_QT, test x$enable_poppler_qt = xyes)

AC_ARG_ENABLE(gtk-test,
              AC_HELP_STRING([--disable-gtk-test],
	                     [Don't compile GTK+ test program.]),
              enable_gtk_test=$enableval,
              enable_gtk_test="try")
if test x$enable_gtk_test = xyes; then
  PKG_CHECK_MODULES(GTK_TEST, gtk+-2.0 >= 2.2.0 gdk-pixbuf-2.0)
elif test x$enable_gtk_test = xtry; then
  PKG_CHECK_MODULES(GTK_TEST, gtk+-2.0 >= 2.2.0 gdk-pixbuf-2.0,
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
glib/Makefile
test/Makefile
qt/Makefile
poppler.pc
poppler-cairo.pc
poppler-splash.pc
poppler-glib.pc
poppler-qt.pc])


echo ""
echo "Building poppler with support for:"
echo "  splash output: $enable_splash_output"
echo "  cairo output:  $enable_cairo_output"
echo "  qt wrapper:    $enable_poppler_qt"
echo "  glib wrapper:  $enable_poppler_glib"
echo "  use libjpeg:   $enable_libjpeg"
