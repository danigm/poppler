# Copyright 2008 Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(POPPLER_ADD_TEST exe build_flag)
  set(build_test ${${build_flag}})
  if(NOT build_test)
    set(_add_executable_param ${_add_executable_param} EXCLUDE_FROM_ALL)
  endif(NOT build_test)

  add_executable(${exe} ${_add_executable_param} ${ARGN})

  # if the tests are EXCLUDE_FROM_ALL, add a target "buildtests" to build all tests
  if(NOT build_test)
    get_directory_property(_buildtestsAdded BUILDTESTS_ADDED)
    if(NOT _buildtestsAdded)
      add_custom_target(buildtests)
      set_directory_properties(PROPERTIES BUILDTESTS_ADDED TRUE)
    endif(NOT _buildtestsAdded)
    add_dependencies(buildtests ${exe})
  endif(NOT build_test)
endmacro(POPPLER_ADD_TEST)

macro(POPPLER_ADD_UNITTEST exe build_flag)
  set(build_test ${${build_flag}})
  if(NOT build_test)
    set(_add_executable_param ${_add_executable_param} EXCLUDE_FROM_ALL)
  endif(NOT build_test)

  add_executable(${exe} ${_add_executable_param} ${ARGN})
  add_test(${exe} ${EXECUTABLE_OUTPUT_PATH}/${exe})

  # if the tests are EXCLUDE_FROM_ALL, add a target "buildtests" to build all tests
  if(NOT build_test)
    get_directory_property(_buildtestsAdded BUILDTESTS_ADDED)
    if(NOT _buildtestsAdded)
      add_custom_target(buildtests)
      set_directory_properties(PROPERTIES BUILDTESTS_ADDED TRUE)
    endif(NOT _buildtestsAdded)
    add_dependencies(buildtests ${exe})
  endif(NOT build_test)
endmacro(POPPLER_ADD_UNITTEST)

macro(POPPLER_CREATE_INSTALL_PKGCONFIG generated_file install_location)
  if(NOT WIN32)
    configure_file(${generated_file}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${generated_file} @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${generated_file} DESTINATION ${install_location})
  endif(NOT WIN32)
endmacro(POPPLER_CREATE_INSTALL_PKGCONFIG)

macro(SHOW_END_MESSAGE what enabled)
  string(LENGTH ${what} length_what)
  math(EXPR left_char "20 - ${length_what}")
  set(blanks)
  foreach(_i RANGE 1 ${left_char})
    set(blanks "${blanks} ")
  endforeach(_i)
  if(${enabled})
    set(enabled_string "yes")
  else(${enabled})
    set(enabled_string "no")
  endif(${enabled})

  message("  ${what}:${blanks} ${enabled_string}")
endmacro(SHOW_END_MESSAGE)


set(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH}
                              "${CMAKE_INSTALL_PREFIX}/include" )

set(CMAKE_SYSTEM_PROGRAM_PATH ${CMAKE_SYSTEM_PROGRAM_PATH}
                              "${CMAKE_INSTALL_PREFIX}/bin" )

set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH}
                              "${CMAKE_INSTALL_PREFIX}/lib" )

# under Windows dlls may be also installed in bin/
if(WIN32)
  set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH}
                                "${CMAKE_INSTALL_PREFIX}/bin" )
endif(WIN32)

