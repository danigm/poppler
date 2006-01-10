ACLOCAL_AMFLAGS = -I m4
DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

if BUILD_SPLASH_OUTPUT
splash_subdir = splash
splash_pc_file = poppler-splash.pc
endif

if BUILD_CAIRO_OUTPUT
cairo_pc_file = poppler-cairo.pc
endif

if BUILD_POPPLER_GLIB
glib_subdir = glib
glib_pc_file = poppler-glib.pc
endif

if BUILD_POPPLER_QT
qt_subdir = qt
qt_pc_file = poppler-qt.pc
endif

if BUILD_POPPLER_QT4
qt4_subdir = qt4
qt4_pc_file = poppler-qt4.pc
endif

if BUILD_UTILS
utils_subdir = utils
endif

SUBDIRS = goo fofi $(splash_subdir) poppler $(utils_subdir) $(glib_subdir) $(qt_subdir) test $(qt4_subdir)

EXTRA_DIST =					\
	README-XPDF				\
	poppler.pc.in				\
	poppler-cairo.pc.in			\
	poppler-splash.pc.in			\
	poppler-qt.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =				\
	poppler.pc				\
	$(cairo_pc_file)			\
	$(splash_pc_file)			\
	$(glib_pc_file)				\
	$(qt_pc_file)                           \
	$(qt4_pc_file)
