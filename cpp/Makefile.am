INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/goo			\
	-I$(top_srcdir)/poppler

SUBDIRS = . tests

poppler_includedir = $(includedir)/poppler/cpp
poppler_include_HEADERS =			\
	poppler-document.h			\
	poppler-embedded-file.h			\
	poppler-font.h				\
	poppler-global.h			\
	poppler-page.h				\
	poppler-page-transition.h		\
	poppler-rectangle.h			\
	poppler-toc.h				\
	$(builddir)/poppler-version.h

lib_LTLIBRARIES = libpoppler-cpp.la
libpoppler_cpp_la_SOURCES =			\
	poppler-document.cpp			\
	poppler-embedded-file.cpp		\
	poppler-font.cpp			\
	poppler-global.cpp			\
	poppler-page.cpp			\
	poppler-page-transition.cpp		\
	poppler-private.cpp			\
	poppler-rectangle.cpp			\
	poppler-toc.cpp				\
	poppler-version.cpp

libpoppler_cpp_la_LIBADD = 			\
	$(top_builddir)/poppler/libpoppler.la

libpoppler_cpp_la_LDFLAGS = -version-info 1:0:1 @create_shared_lib@
