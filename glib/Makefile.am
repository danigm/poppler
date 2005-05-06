if BUILD_CAIRO_OUTPUT

cairo_includes =				\
	$(CAIRO_CFLAGS)

cairo_libs =					\
	$(CAIRO_LIBS)

endif

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	$(cairo_includes)			\
	$(POPPLER_GLIB_CFLAGS)			\
	$(FREETYPE_CFLAGS)			\
	$(cairo_includes)			\
	-DDATADIR=\""$(datadir)"\"




poppler-enums.h: poppler-document.h
	( cd $(srcdir) && glib-mkenums \
			--fhead "#ifndef __POPPLER_TYPE_ENUMS_H__\n#define __POPPLER_TYPE_ENUMS_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" \
			--fprod "/* enumerations from \"@filename@\" */\n" \
			--vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n#define POPPLER_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
			--ftail "G_END_DECLS\n\n#endif /* __POPPLER_TYPE_ENUMS_H__ */" \
		$(poppler_glib_public_headers) ) > poppler-enums.h

poppler-enums.c: poppler-document.h
	( cd $(srcdir) && glib-mkenums \
			--fhead "#include \"poppler-enums.h\"\n#include \"poppler-document.h\"\n" \
			--ftail "#define __POPPLER_ENUMS_C__\n" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {" \
			--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n" \
		$(poppler_glib_public_headers) ) > poppler-enums.c


poppler_glib_public_headers =			\
	poppler-action.h			\
	poppler-document.h			\
	poppler-page.h				\
	poppler.h

poppler_glib_includedir = $(includedir)/poppler/glib

poppler_glib_include_HEADERS =			\
	$(poppler_glib_public_headers)		\
	poppler-enums.h

lib_LTLIBRARIES = libpoppler-glib.la
libpoppler_glib_la_SOURCES =			\
	poppler-action.cc			\
	poppler-enums.c				\
	poppler-document.cc			\
	poppler-page.cc				\
	poppler.cc				\
	poppler-private.h

libpoppler_glib_la_LIBADD =			\
	$(top_builddir)/poppler/libpoppler.la	\
	$(POPPLER_GLIB_LIBS)			\
	$(FREETYPE_LIBS)			\
	$(cairo_libs)

noinst_PROGRAMS = test-poppler-glib

test_poppler_glib_SOURCES =			\
       test-poppler-glib.c

test_poppler_glib_LDADD =			\
	$(top_builddir)/poppler/libpoppler.la	\
	libpoppler-glib.la			\
	$(POPPLER_GLIB_LIBS)			\
	$(FREETYPE_LIBS)			\
	$(cairo_libs)
