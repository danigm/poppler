# - try to find GTK (and glib)
# Once done this will define
#
#  GLIB_FOUND - system has GLib
#  GLIB_CFLAGS - the GLib CFlags
#  GLIB_LIBRARIES - Link these to use GLib
#
# Copyright 2008 Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

if (NOT WIN32)
  include(UsePkgConfig)
  include(FindPkgConfig)

  pkg_check_modules(GLIB2 "glib-2.0>=${GLIB_REQUIRED}" "gobject-2.0>=${GLIB_REQUIRED}")
  pkg_check_modules(GDK2 "gdk-2.0")

  find_package_handle_standard_args(GLib DEFAULT_MSG GLIB2_LIBRARIES GLIB2_CFLAGS)
  find_package_handle_standard_args(GDK DEFAULT_MSG GDK2_LIBRARIES GDK2_CFLAGS)

  pkgconfig(gtk+-2.0 _LibGTK2IncDir _LibGTK2LinkDir GTK2LinkFlags GTK2Cflags)
  pkgconfig(gdk-pixbuf-2.0 _LibGDK2PixbufIncDir _LibGDK2PixbufLinkDir GDK2PixbufLinkFlags GDK2PixbufCflags)
  pkgconfig(gthread-2.0 _LibGThread2IncDir _LibGThread2LinkDir GThread2LinkFlags GThread2Cflags)

  if (_LibGTK2IncDir AND _LibGDK2PixbufIncDir AND  _LibGThread2IncDir)
    exec_program(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=2.14 gtk+-2.0 RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull)
    if(_return_VALUE STREQUAL "0")
      set(_gtk_FOUND TRUE)
    endif(_return_VALUE STREQUAL "0")

    if (_gtk_FOUND)
      set (GTK2_CFLAGS ${GTK2Cflags} ${GDK2PixbufCflags} ${GThread2Cflags})
      set (GTK2_LIBRARIES ${GTK2LinkFlags} ${GDK2PixbufLinkFlags} ${GThread2LinkFlags})
    endif (_gtk_FOUND)

    find_package_handle_standard_args(GTK DEFAULT_MSG GTK2_LIBRARIES GTK2_CFLAGS)

  endif (_LibGTK2IncDir AND _LibGDK2PixbufIncDir AND _LibGThread2IncDir)

endif(NOT WIN32)

mark_as_advanced(
  GTK2_CFLAGS
  GTK2_LIBRARIES
)

