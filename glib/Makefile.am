INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	$(cairo_includes)			\
	$(POPPLER_GLIB_CFLAGS)			\
	$(FREETYPE_CFLAGS)			\
	-DDATADIR=\""$(datadir)"\"

poppler_glib_includedir = $(includedir)/poppler/glib
poppler_glib_include_HEADERS =			\
	poppler-action.h			\
	poppler-document.h			\
	poppler-page.h				\
	poppler.h

lib_LTLIBRARIES = libpoppler-glib.la
libpoppler_glib_la_SOURCES =			\
	poppler-action.cc			\
	poppler-document.cc			\
	poppler-page.cc				\
	poppler.cc				\
	poppler-private.h

libpoppler_glib_la_LIBADD =			\
	$(POPPLER_GLIB_LIBS)			\
	$(FREETYPE_LIBS)

noinst_PROGRAMS = test-poppler-glib

test_poppler_glib_SOURCES =			\
       test-poppler-glib.c

test_poppler_glib_LDADD =			\
	$(top_builddir)/poppler/libpoppler.la	\
	libpoppler-glib.la			\
	$(POPPLER_GLIB_LIBS)			\
	$(FREETYPE_LIBS)
