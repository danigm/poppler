INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	$(POPPLER_QT4_CXXFLAGS)


poppler_includedir = $(includedir)/poppler

poppler_include_HEADERS =			\
	poppler-qt4.h

lib_LTLIBRARIES=libpoppler-qt4.la

libpoppler_qt4_la_SOURCES =			\
	poppler-document.cc			\
	poppler-page.cc				\
	poppler-private.h

libpoppler_qt4_la_LIBADD= 			\
	$(top_builddir)/poppler/libpoppler.la	\
	$(POPPLER_QT4_LIBS)

