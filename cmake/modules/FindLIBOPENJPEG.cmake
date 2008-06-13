# - Try to find the libopenjpeg library
# Once done this will define
#
#  LIBOPENJPEG_FOUND - system has libopenjpeg
#  LIBOPENJPEG_INCLUDE_DIRS - the libopenjpeg include directories
#  LIBOPENJPEG_LIBRARIES - Link these to use libopenjpeg
#  LIBOPENJPEG_INCLUDE_DIR is internal and deprecated for use

# Copyright (c) 2008, Albert Astals Cid, <aacid@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (LIBOPENJPEG_LIBRARIES AND LIBOPENJPEG_INCLUDE_DIR)

  # in cache already
  set(LIBOPENJPEG_FOUND TRUE)

else (LIBOPENJPEG_LIBRARIES AND LIBOPENJPEG_INCLUDE_DIR)

  #reset vars
  set(LIBOPENJPEG_LIBRARIES)
  set(LIBOPENJPEG_INCLUDE_DIR)

  find_path (LIBOPENJPEG_INCLUDE_DIR openjpeg.h)
  find_library(LIBOPENJPEG_LIBRARIES openjpeg)
  if(LIBOPENJPEG_INCLUDE_DIR AND LIBOPENJPEG_LIBRARIES)
    set(LIBOPENJPEG_FOUND TRUE)
  endif(LIBOPENJPEG_INCLUDE_DIR AND LIBOPENJPEG_LIBRARIES)

  IF (LIBOPENJPEG_FOUND)
    IF (NOT LIBOPENJPEG_FIND_QUIETLY)
       MESSAGE(STATUS "Found libopenjpeg: ${LIBOPENJPEG_LIBRARIES}")
    ENDIF (NOT LIBOPENJPEG_FIND_QUIETLY)
  ELSE (LIBOPENJPEG_FOUND)
    IF (LIBOPENJPEG_FIND_REQUIRED)
       MESSAGE(FATAL_ERROR "Could not find libopenjpeg library")
    ENDIF (LIBOPENJPEG_FIND_REQUIRED)
  ENDIF (LIBOPENJPEG_FOUND)

endif (LIBOPENJPEG_LIBRARIES AND LIBOPENJPEG_INCLUDE_DIR)

set(LIBOPENJPEG_INCLUDE_DIRS ${LIBOPENJPEG_INCLUDE_DIR})
