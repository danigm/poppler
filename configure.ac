dnl Based on Xpdf configure.in and evince configure.ac

dnl Based on kde acinclude.m4.in, LGPL Licensed

AC_DEFUN([AC_FIND_FILE],
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    echo "configure: __oline__: $i/$j" >&AC_FD_CC
    if test -r "$i/$j"; then
      echo "taking that" >&AC_FD_CC
      $3=$i
      break 2
    fi
  done
done
])

AC_DEFUN([KDE_FIND_JPEG_HELPER],
[
AC_MSG_CHECKING([for libjpeg$2])
AC_CACHE_VAL(ac_cv_lib_jpeg_$1,
[
ac_save_LIBS="$LIBS"
LIBS="$all_libraries $USER_LDFLAGS -ljpeg$2 -lm"
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $all_includes $USER_INCLUDES"
AC_TRY_LINK(
[
#ifdef __cplusplus
extern "C" {
#endif
void jpeg_CreateDecompress();
#ifdef __cplusplus
}
#endif
],
[jpeg_CreateDecompress();],
            eval "ac_cv_lib_jpeg_$1=-ljpeg$2",
            eval "ac_cv_lib_jpeg_$1=no")
LIBS="$ac_save_LIBS"
CFLAGS="$ac_save_CFLAGS"
])

if eval "test ! \"`echo $ac_cv_lib_jpeg_$1`\" = no"; then
  enable_libjpeg=yes
  LIBJPEG_LIBS="$ac_cv_lib_jpeg_$1"
  AC_MSG_RESULT($ac_cv_lib_jpeg_$1)
else
  AC_MSG_RESULT(no)
  $3
fi

])

AC_DEFUN([AC_FIND_JPEG],
[
dnl first look for libraries
KDE_FIND_JPEG_HELPER(6b, 6b,
   KDE_FIND_JPEG_HELPER(normal, [],
    [
       LIBJPEG_LIBS=
    ]
   )
)

dnl then search the headers (can't use simply AC_TRY_xxx, as jpeglib.h
dnl requires system dependent includes loaded before it)
jpeg_incdirs="$includedir /usr/include /usr/local/include "
AC_FIND_FILE(jpeglib.h, $jpeg_incdirs, jpeg_incdir)
test "x$jpeg_incdir" = xNO && jpeg_incdir=

dnl if headers _and_ libraries are missing, this is no error, and we
dnl continue with a warning (the user will get no jpeg support)
dnl if only one is missing, it means a configuration error, but we still
dnl only warn
if test -n "$jpeg_incdir" && test -n "$LIBJPEG_LIBS" ; then
  AC_DEFINE_UNQUOTED(ENABLE_LIBJPEG, 1, [Define if you have libjpeg])
else
  if test -n "$jpeg_incdir" || test -n "$LIBJPEG_LIBS" ; then
    AC_MSG_WARN([
There is an installation error in jpeg support. You seem to have only one
of either the headers _or_ the libraries installed. You may need to either
provide correct --with-extra-... options, or the development package of
libjpeg6b. You can get a source package of libjpeg from http://www.ijg.org/
Disabling JPEG support.
])
  else
    AC_MSG_WARN([libjpeg not found. disable JPEG support.])
  fi
  jpeg_incdir=
  enable_libjpeg=no
  LIBJPEG_LIBS=
fi

AC_SUBST(LIBJPEG_LIBS)
AH_VERBATIM(_AC_CHECK_JPEG,
[/*
 * jpeg.h needs HAVE_BOOLEAN, when the system uses boolean in system
 * headers and I'm too lazy to write a configure test as long as only
 * unixware is related
 */
#ifdef _UNIXWARE
#define HAVE_BOOLEAN
#endif
])
])

dnl Based on kde acinclude.m4.in, LGPL Licensed


AC_PREREQ(2.59)
AC_INIT(poppler, 0.5.0)
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
AC_CHECK_FUNC(gettimeofday, AC_DEFINE(HAVE_GETTIMEOFDAY, 1, [Defines if gettimeofday is available on your system]))

dnl Enable these unconditionally.
AC_DEFINE([A4_PAPER], [1], [Default to A4 paper.])
AC_DEFINE([OPI_SUPPORT], [1], [Generate OPI comments in PS output.])
AC_DEFINE([MULTITHREADED], [1], [Enable multithreading support.])
AC_DEFINE([TEXTOUT_WORD_LIST], [1], [Enable word list support.])

dnl Install xpdf headers
AC_ARG_ENABLE(xpdf-headers,
	      AC_HELP_STRING([--enable-xpdf-headers],
	                     [Install unsupported xpdf headers.]),
              enable_xpdf_headers=$enableval,
              enable_xpdf_headers="no")
AM_CONDITIONAL(ENABLE_XPDF_HEADERS, test x$enable_xpdf_headers = xyes)

AC_ARG_ENABLE(fixedpoint,
[  --enable-fixedpoint     use fixed point (instead of floating point) arithmetic],
AC_DEFINE(USE_FIXEDPOINT, [], [Use fixed point arithmetic]))

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

dnl Test for zlib
AC_ARG_ENABLE(zlib,
	      AC_HELP_STRING([--disable-zlib],
	                     [Don't build against zlib.]),
              enable_zlib=$enableval,
              enable_zlib="no")
if test x$enable_zlib = xyes; then
  AC_CHECK_LIB([z], [inflate],,
	       AC_MSG_ERROR("*** zlib library not found ***"))
  AC_CHECK_HEADERS([zlib.h],,
		   AC_MSG_ERROR("*** zlib headers not found ***"))
elif test x$enable_zlib = xtry; then
  AC_CHECK_LIB([z], [inflate],
               [enable_zlib="yes"],
	       [enable_zlib="no"])
  AC_CHECK_HEADERS([zlib.h],,
		   [enable_zlib="no"])
fi

if test x$enable_zlib = xyes; then
  ZLIB_LIBS="-lz"
  AC_SUBST(ZLIB_LIBS)
  AC_DEFINE(ENABLE_ZLIB)
fi

AM_CONDITIONAL(BUILD_ZLIB, test x$enable_zlib = xyes)
AH_TEMPLATE([ENABLE_ZLIB],
	    [Use zlib instead of builtin zlib decoder.])

dnl Test for libjpeg
AC_ARG_ENABLE(libjpeg,
	      AC_HELP_STRING([--disable-libjpeg],
	                     [Don't build against libjpeg.]),
              enable_libjpeg=$enableval,
              enable_libjpeg="try")
if test x$enable_libjpeg != xno; then
  AC_FIND_JPEG
fi

AM_CONDITIONAL(BUILD_LIBJPEG, test x$enable_libjpeg = xyes)
AH_TEMPLATE([ENABLE_LIBJPEG],
            [Use libjpeg instead of builtin jpeg decoder.])


dnl Check for freetype headers
FREETYPE_LIBS=
FREETYPE_CFLAGS=

PKG_CHECK_MODULES(FREETYPE, freetype2,
                  [freetype_pkgconfig=yes], [freetype_pkgconfig=no])

if test "x$freetype_pkgconfig" = "xyes"; then

  AC_DEFINE(HAVE_FREETYPE_H, 1, [Have FreeType2 include files])

else

  AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
  if test "x$FREETYPE_CONFIG" != "xno" ; then
	  
    FREETYPE_CFLAGS=`$FREETYPE_CONFIG --cflags`
    FREETYPE_LIBS=`$FREETYPE_CONFIG --libs`
    AC_DEFINE(HAVE_FREETYPE_H, 1, [Have FreeType2 include files])

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

CAIRO_VERSION="0.5"
AC_SUBST(CAIRO_VERSION)
AC_ARG_ENABLE(cairo-output,
              AC_HELP_STRING([--disable-cairo-output],
                             [Don't build the cairo graphics backend.]),
              enable_cairo_output=$enableval,
              enable_cairo_output="try")
if test x$enable_cairo_output = xyes; then
  PKG_CHECK_MODULES(CAIRO, cairo >= $CAIRO_VERSION fontconfig)
elif test x$enable_cairo_output = xtry; then
  PKG_CHECK_MODULES(CAIRO, cairo >= $CAIRO_VERSION fontconfig,
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
  PKG_CHECK_MODULES(POPPLER_GLIB, gdk-2.0 >= 2.4.0 fontconfig)
elif test x$enable_poppler_glib = xtry; then
  PKG_CHECK_MODULES(POPPLER_GLIB, gdk-2.0 >= 2.4.0 fontconfig,
                    [enable_poppler_glib="yes"],
                    [enable_poppler_glib="no"])
fi
if test x$enable_poppler_glib = xyes; then
  AC_PATH_PROG([GLIB_MKENUMS],[glib-mkenums])
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
qt_test_la_library="libqt-mt.la"
qt_test_library="libqt-mt.so"

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
    if test -r "$qt_check/$qt_test_la_library" ; then
        qt_libdir="$qt_check"
        break
    fi

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

dnl 
dnl Try Qt4
dnl
AC_ARG_ENABLE(poppler-qt4,
	      AC_HELP_STRING([--disable-poppler-qt4],
			     [Don't compile poppler Qt4 wrapper.]),
	      enable_poppler_qt4=$enableval,
	      enable_poppler_qt4="try")

have_qt4=no
dnl ===================================================================
dnl Check whether the Qt4 libraries are available.
dnl Adapted from Qt3 check above!
dnl ===================================================================

dnl Search paths for Qt4 - not much real experience with this yet.
qt4_incdirs="/usr/local/qt/include /usr/include/qt4 /usr/include/qt /usr/include /usr/X11R6/include/X11/qt /usr/X11R6/include/qt /usr/lib/qt/include"
qt4_libdirs="/usr/local/qt/lib /usr/lib/qt /usr/lib /usr/X11R6/lib/X11/qt /usr/X11R6/lib/qt /usr/lib/qt/lib"

if test -n "$QTDIR" ; then
    qt4_incdirs="$QTDIR/include $qt4_incdirs"
    qt4_libdirs="$QTDIR/lib $qt4_libdirs"
fi

dnl What to test
qt4_test_include="QtCore/QCoreApplication"
qt4_test_la_library="libQtCore.la"
qt4_test_library="libQtCore.so"
qt4_windows_test_library="QtCore4.dll"

dnl Check for Qt4 headers
AC_MSG_CHECKING([for Qt4 headers])
qt4_incdir="no"
for it in $qt4_incdirs ; do
    if test -r "$it/$qt4_test_include" ; then
        qt4_incdir="$it"
        break
    fi
done
AC_MSG_RESULT([$qt4_incdir])

dnl Check for Qt4 libraries
AC_MSG_CHECKING([for Qt4 libraries])
qt4_libdir="no"
for qt4_check in $qt4_libdirs ; do
    if test -r "$qt4_check/$qt4_test_la_library" ; then
        qt4_libdir="$qt4_check"
        break
    fi

    if test -r "$qt4_check/$qt4_test_library" ; then
        qt4_libdir="$qt4_check"
        break
    fi

    if test -r "$qt4_check/$qt4_windows_test_library" ; then
        qt4_libdir="$qt4_check"
        windows_qt="yes"
        break
    fi
done
AC_MSG_RESULT([$qt4_libdir])

if test "x$qt4_libdir" != "xno" ; then
    if test "x$qt4_incdir" != "xno" ; then
        have_qt4=yes;
    fi
fi

dnl Set the variables
POPPLER_QT4_CXXFLAGS="-I$qt4_incdir"
if test x$windows_qt = xyes; then
	POPPLER_QT4_LIBS="-L$qt4_libdir -lQtCore4 -lQtGui4"
else
	POPPLER_QT4_LIBS="-L$qt4_libdir -lQtCore -lQtGui"
fi

AC_SUBST(POPPLER_QT4_CXXFLAGS)
AC_SUBST(POPPLER_QT4_LIBS)

if test x$have_qt4 = xno ; then
    AC_MSG_WARN([Qt4 development libraries not found])
fi

if test x$enable_poppler_qt4 = xyes; then
  if test x$have_qt4 = xno; then
    AC_MSG_ERROR([Qt4 integration explicitly required, and Qt4 libraries not found])
  fi
elif test x$enable_poppler_qt4 = xtry; then
  if test x$have_qt4 = xyes; then
    enable_poppler_qt4="yes"
  else
    enable_poppler_qt4="no"
  fi
fi
AM_CONDITIONAL(BUILD_POPPLER_QT4, test x$enable_poppler_qt4 = xyes)


have_qt4testlib=no
dnl ===================================================================
dnl Check whether the QtTestLib headers and libs  are available.
dnl Adapted from Qt4 check above!
dnl ===================================================================

dnl We re-use much of the Qt4 environment.

dnl What to test
qt4test_test_include="QtTest/QtTest"
qt4test_test_la_library="libQtTest.la"
qt4test_test_library="libQtTest.so"

dnl Check for QtTestLib headers
AC_MSG_CHECKING([for QtTestLib headers])
qt4test_incdir="no"
for it in $qt4_incdirs ; do
    if test -r "$it/$qt4test_test_include" ; then
        qt4test_incdir="$it"
        break
    fi
done
AC_MSG_RESULT([$qt4test_incdir])

dnl Check for Qt4 libraries
AC_MSG_CHECKING([for QtTestLib libraries])
qt4test_libdir="no"
for qt4test_check in $qt4_libdirs ; do
    if test -r "$qt4test_check/$qt4test_test_la_library" ; then
        qt4test_libdir="$qt4test_check"
        break
    fi

    if test -r "$qt4test_check/$qt4test_test_library" ; then
        qt4test_libdir="$qt4test_check"
        break
    fi
done
AC_MSG_RESULT([$qt4test_libdir])

if test "x$qt4test_libdir" != "xno" ; then
    if test "x$qt4test_incdir" != "xno" ; then
        have_qt4testlib=yes;
    fi
fi

dnl Set the variables
POPPLER_QTTEST_CXXFLAGS="-I$qt4test_incdir"
POPPLER_QTTEST_LIBS="-L$qt4test_libdir -lQtTest"

AC_SUBST(POPPLER_QTTEST_CXXFLAGS)
AC_SUBST(POPPLER_QTTEST_LIBS)

if test x$have_qt4testlib = xno ; then
    AC_MSG_WARN([QtTestLib libraries not found])
fi

if test x$have_qt4testlib = xyes; then
    enable_poppler_qt4testlib="yes"
else
    enable_poppler_qt4testlib="no"
fi

AM_CONDITIONAL(BUILD_POPPLER_QT4TESTS, test x$enable_poppler_qt4testlib = xyes)

AC_ARG_ENABLE(gtk-test,
              AC_HELP_STRING([--disable-gtk-test],
	                     [Don't compile GTK+ test program.]),
              enable_gtk_test=$enableval,
              enable_gtk_test="try")
if test x$enable_gtk_test = xyes; then
  PKG_CHECK_MODULES(GTK_TEST, gtk+-2.0 >= 2.2.0 gdk-pixbuf-2.0 libglade-2.0)
elif test x$enable_gtk_test = xtry; then
  PKG_CHECK_MODULES(GTK_TEST, gtk+-2.0 >= 2.2.0 gdk-pixbuf-2.0 libglade-2.0,
                    [enable_gtk_test="yes"],
                    [enable_gtk_test="no"])
fi
AM_CONDITIONAL(BUILD_GTK_TEST, test x$enable_gtk_test = xyes)


AC_ARG_ENABLE(utils,
              AC_HELP_STRING([--disable-utils],
	                     [Don't compile poppler command line utils.]),
              enable_utils=$enableval,
              enable_utils="yes")
if test x$enable_utils = xyes; then
  PKG_CHECK_MODULES(UTILS, fontconfig)
fi
AM_CONDITIONAL(BUILD_UTILS, test x$enable_utils = xyes)

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
utils/Makefile
glib/Makefile
test/Makefile
qt/Makefile
qt4/Makefile
qt4/src/Makefile
qt4/tests/Makefile
poppler.pc
poppler-cairo.pc
poppler-splash.pc
poppler-glib.pc
poppler-qt.pc
poppler-qt4.pc])


echo ""
echo "Building poppler with support for:"
echo "  splash output:      $enable_splash_output"
echo "  cairo output:       $enable_cairo_output"
echo "  qt wrapper:         $enable_poppler_qt"
echo "  qt4 wrapper:        $enable_poppler_qt4"
echo "  qt4 unittests:      $enable_poppler_qt4testlib"
echo "  glib wrapper:       $enable_poppler_glib"
echo "  use libjpeg:        $enable_libjpeg"
echo "  use zlib:           $enable_zlib"
echo "  command line utils: $enable_poppler_glib"
