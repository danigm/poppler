INCLUDES = 					\
	-I$(top_srcdir)/glib			\
	$(GTK_TEST_CFLAGS)

if BUILD_GTK_TEST
noinst_PROGRAMS = poppler-glib-demo
endif

poppler_glib_demo_SOURCES = 			\
	main.c					\
	fonts.h					\
	fonts.c					\
	info.h					\
	info.c					\
	links.h					\
	links.c					\
	outline.h				\
	outline.c				\
	page.h					\
	page.c					\
	render.h				\
	render.c				\
	utils.h					\
	utils.c

LDADD =						\
	$(top_builddir)/glib/libpoppler-glib.la	\
	$(top_builddir)/poppler/libpoppler.la	\
	$(GTK_TEST_LIBS)
