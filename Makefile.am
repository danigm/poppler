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

SUBDIRS = goo fofi $(splash_subdir) poppler $(glib_subdir) test

EXTRA_DIST =					\
	README-XPDF				\
	poppler.pc.in				\
	poppler-cairo.pc.in			\
	poppler-splash.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =				\
	poppler.pc				\
	$(cairo_pc_file)			\
	$(splash_pc_file)			\
	$(glib_pc_file)
