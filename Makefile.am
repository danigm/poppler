if BUILD_SPLASH_OUTPUT
splash_subdir = splash
endif

SUBDIRS = goo fofi $(splash_subdir) poppler test

EXTRA_DIST =					\
	README-XPDF				\
	poppler.pc.in				\
	poppler-cairo.pc.in			\
	poppler-splash.pc.in

if BUILD_SPLASH_OUTPUT

splash_pc_file = poppler-splash.pc

endif

if BUILD_CAIRO_OUTPUT

cairo_pc_file = poppler-cairo.pc

endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =				\
	poppler.pc				\
	$(cairo_pc_file)			\
	$(splash_pc_file)
