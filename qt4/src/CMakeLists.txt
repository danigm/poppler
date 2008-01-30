add_definitions(${QT4_DEFINITIONS})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/qt # for PageTransition
  ${QT4_INCLUDE_DIR}
)

set(poppler_qt4_SRCS
  poppler-annotation.cc
  poppler-document.cc
  poppler-embeddedfile.cc
  poppler-fontinfo.cc
  poppler-form.cc
  poppler-link.cc
  poppler-link-extractor.cc
  poppler-page.cc
  poppler-base-converter.cc
  poppler-pdf-converter.cc
  poppler-ps-converter.cc
  poppler-qiodeviceoutstream.cc
  poppler-sound.cc
  poppler-textbox.cc
  ${CMAKE_SOURCE_DIR}/qt/poppler-page-transition.cc
)
if (ENABLE_SPLASH)
  set(poppler_qt4_SRCS ${poppler_qt4_SRCS}
    ${CMAKE_SOURCE_DIR}/poppler/ArthurOutputDev.cc
  )
endif (ENABLE_SPLASH)
add_library(poppler-qt4 SHARED ${poppler_qt4_SRCS})
set_target_properties(poppler-qt4 PROPERTIES VERSION 2.0.0 SOVERSION 2)
target_link_libraries(poppler-qt4 poppler ${QT4_QTCORE_LIBRARY} ${QT4_QTGUI_LIBRARY} ${QT4_QTXML_LIBRARY})
install(TARGETS poppler-qt4 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

install(FILES
  poppler-qt4.h
  poppler-link.h
  poppler-annotation.h
  poppler-form.h
  ${CMAKE_SOURCE_DIR}/qt/poppler-page-transition.h
  DESTINATION include/poppler/qt4)

