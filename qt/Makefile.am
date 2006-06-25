INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	$(cairo_includes)			\
	$(POPPLER_QT_CXXFLAGS)			\
	$(SPLASH_CFLAGS)			\
	$(FREETYPE_CFLAGS)			


poppler_includedir = $(includedir)/poppler
poppler_include_HEADERS =			\
	poppler-qt.h				\
	poppler-page-transition.h               \
	poppler-link-qt3.h

lib_LTLIBRARIES = libpoppler-qt.la
libpoppler_qt_la_SOURCES =			\
	poppler-document.cc			\
	poppler-fontinfo.cc			\
	poppler-link.cc                         \
	poppler-page.cc				\
	poppler-page-transition.cc		\
	poppler-page-transition-private.h	\
	poppler-private.h

libpoppler_qt_la_LIBADD = 			\
	$(top_builddir)/poppler/libpoppler.la	\
	$(POPPLER_QT_LIBS)			\
	$(FREETYPE_LIBS)

libpoppler_qt_la_LDFLAGS = -version-info 1:0:0

if BUILD_SPLASH_OUTPUT

noinst_PROGRAMS = test-poppler-qt

endif

test_poppler_qt_SOURCES =			\
       test-poppler-qt.cpp

test_poppler_qt_LDADD =				\
	$(top_builddir)/poppler/libpoppler.la	\
	libpoppler-qt.la			\
	$(POPPLER_QT_LIBS)			\
	$(FREETYPE_LIBS)
