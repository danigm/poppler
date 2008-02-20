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
.h.moc:
	moc -i $< -o $@

noinst_PROGRAMS = poppler_qt4viewer

poppler_qt4viewer_SOURCES =			\
	abstractinfodock.cpp			\
	abstractinfodock.h			\
	documentobserver.cpp			\
	documentobserver.h			\
	embeddedfiles.cpp			\
	embeddedfiles.h				\
	fonts.cpp				\
	fonts.h					\
	info.cpp				\
	info.h					\
	main_viewer.cpp				\
	navigationtoolbar.cpp			\
	navigationtoolbar.h			\
	pageview.cpp				\
	pageview.h				\
	permissions.cpp				\
	permissions.h				\
	toc.cpp					\
	toc.h					\
	viewer.cpp				\
	viewer.h

abstractinfodock.$(OBJEXT): abstractinfodock.moc
embeddedfiles.cpp.$(OBJEXT): embeddedfiles.moc
fonts.$(OBJEXT): fonts.moc
info.$(OBJEXT): info.moc
navigationtoolbar.$(OBJEXT): navigationtoolbar.moc
pageview.$(OBJEXT): pageview.moc
permissions.$(OBJEXT): permissions.moc
toc.$(OBJEXT): toc.moc
viewer.$(OBJEXT): viewer.moc

poppler_qt4viewer_LDADD = $(LDADDS)
