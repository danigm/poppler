# - Try to find the liblcms library
# Once done this will define
#
#  LCMS_FOUND - system has liblcms
#  LCMS_INCLUDE_DIRS - the liblcms include directories
#  LCMS_LIBRARIES - Link these to use liblcms
#  LCMS_INCLUDE_DIR is internal and deprecated for use

# Copyright (c) 2008, Albert Astals Cid, <aacid@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (LCMS_LIBRARIES AND LCMS_INCLUDE_DIR)

  # in cache already
  set(LCMS_FOUND TRUE)

else (LCMS_LIBRARIES AND LCMS_INCLUDE_DIR)

  #reset vars
  set(LCMS_LIBRARIES)
  set(LCMS_INCLUDE_DIR)

  find_path (LCMS_INCLUDE_DIR lcms.h)
  find_library(LCMS_LIBRARIES lcms)
  if(LCMS_INCLUDE_DIR AND LCMS_LIBRARIES)
    set(LCMS_FOUND TRUE)
  endif(LCMS_INCLUDE_DIR AND LCMS_LIBRARIES)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LCMS DEFAULT_MSG LCMS_LIBRARIES LCMS_INCLUDE_DIR)

endif (LCMS_LIBRARIES AND LCMS_INCLUDE_DIR)

set(LCMS_INCLUDE_DIRS ${LCMS_INCLUDE_DIR})
