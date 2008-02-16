INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	-I$(top_srcdir)/qt4/src			\
	-I$(top_srcdir)/qt				\
	$(FONTCONFIG_CFLAGS)			\
	$(POPPLER_QT4_CFLAGS)

LDADDS =					\
	$(top_builddir)/poppler/libpoppler.la	\
	$(top_builddir)/qt4/src/libpoppler-qt4.la	\
	$(FONTCONFIG_LIBS)				\
	$(POPPLER_QT4_LIBS)

SUFFIXES: .moc

# This rule lets GNU make create any *.moc from the equivalent *.h
%.moc: %.h
	moc $< -o $@

noinst_PROGRAMS = poppler_qt4viewer

poppler_qt4viewer_SOURCES =			\
	abstractinfodock.cpp			\
	documentobserver.cpp			\
	fonts.cpp				\
	info.cpp				\
	main_viewer.cpp				\
	navigationtoolbar.cpp			\
	pageview.cpp				\
	toc.cpp					\
	viewer.cpp				\
	viewer.moc

poppler_qt4viewer_LDADD = $(LDADDS)
