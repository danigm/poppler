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
  pkg_check_modules(GTK2 "gtk+-2.0>=2.14" "gdk-pixbuf-2.0" "gthread-2.0" "gio-2.0")

  find_package_handle_standard_args(GLib DEFAULT_MSG GLIB2_LIBRARIES GLIB2_CFLAGS)
  find_package_handle_standard_args(GDK DEFAULT_MSG GDK2_LIBRARIES GDK2_CFLAGS)
  find_package_handle_standard_args(GTK DEFAULT_MSG GTK2_LIBRARIES GTK2_CFLAGS)

endif(NOT WIN32)
