INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	$(cairo_includes)			\
	$(POPPLER_QT_CXXFLAGS)			\
	$(FREETYPE_CFLAGS)			\
	-DDATADIR=\""$(datadir)"\"


poppler_includedir = $(includedir)/poppler
poppler_include_HEADERS =			\
	poppler-qt.h

lib_LTLIBRARIES=libpoppler-qt.la
libpoppler_qt_la_SOURCES =			\
	poppler-document.cc			\
	poppler-page.cc				\
	poppler-private.h

libpoppler_qt_la_LIBADD= 			\
	$(POPPLER_QT_LIBS)			\
	$(FREETYPE_LIBS)

noinst_PROGRAMS = test-poppler-qt

test_poppler_qt_SOURCES =			\
       test-poppler-qt.cpp

test_poppler_qt_LDADD =				\
	$(top_builddir)/poppler/libpoppler.la	\
	libpoppler-qt.la			\
	$(POPPLER_QT_LIBS)			\
	$(FREETYPE_LIBS)
