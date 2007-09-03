INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	-I$(top_srcdir)/qt			\
	$(FONTCONFIG_CFLAGS)			\
	$(POPPLER_QT4_CFLAGS)

poppler_includedir = $(includedir)/poppler/qt4

poppler_include_HEADERS =			\
	poppler-qt4.h				\
	poppler-link.h				\
	poppler-annotation.h			\
	poppler-form.h				\
	../../qt/poppler-page-transition.h

lib_LTLIBRARIES = libpoppler-qt4.la

libpoppler_qt4_la_SOURCES =			\
	poppler-document.cc			\
	poppler-page.cc				\
	poppler-fontinfo.cc			\
	poppler-embeddedfile.cc			\
	poppler-textbox.cc			\
	poppler-link.cc				\
	poppler-annotation.cc			\
	poppler-link-extractor.cc		\
	../../qt/poppler-page-transition.cc	\
	poppler-sound.cc			\
	poppler-form.cc				\
	poppler-ps-converter.cc			\
	poppler-annotation-helper.h		\
	poppler-page-private.h			\
	poppler-link-extractor-private.h	\
	poppler-annotation-private.h		\
	poppler-private.h

libpoppler_qt4_la_LIBADD = 			\
	$(top_builddir)/poppler/libpoppler.la	\
	$(FONTCONFIG_LIBS)				\
	$(POPPLER_QT4_LIBS)

if BUILD_SPLASH_OUTPUT
libpoppler_qt4_la_LIBADD += 			\
	$(top_builddir)/poppler/libpoppler-arthur.la
endif

libpoppler_qt4_la_LDFLAGS = -version-info 2:0:0

