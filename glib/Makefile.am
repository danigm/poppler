if BUILD_CAIRO_OUTPUT

cairo_includes =				\
	$(CAIRO_CFLAGS)

cairo_libs =					\
	$(top_builddir)/poppler/libpoppler-cairo.la	\
	$(CAIRO_LIBS)

endif

SUBDIRS = . reference demo
INCLUDES =					\
	-DG_LOG_DOMAIN=\"Poppler\"		\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	$(cairo_includes)			\
	$(POPPLER_GLIB_CFLAGS)			\
	$(GDK_CFLAGS)				\
	$(FREETYPE_CFLAGS)			\
	$(FONTCONFIG_CFLAGS)                    \
	$(cairo_includes)			

stamp_files = stamp-poppler-enums.h

poppler-enums.h: stamp-poppler-enums.h
	@true
stamp-poppler-enums.h: $(poppler_glib_public_headers)
	( cd $(srcdir) && $(GLIB_MKENUMS) \
		--fhead "#ifndef __POPPLER_TYPE_ENUMS_H__\n" \
		--fhead "#define __POPPLER_TYPE_ENUMS_H__ 1\n\n" \
		--fhead "#include <glib-object.h>\n\n" \
		--fhead "G_BEGIN_DECLS\n\n" \
		--ftail "G_END_DECLS\n\n" \
		--ftail "#endif /* __POPPLER_TYPE_ENUMS_H__ */\n" \
		--fprod "\n/* --- @filename@ --- */" \
		--eprod "#define POPPLER_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
		--eprod "GType @enum_name@_get_type (void) G_GNUC_CONST;\n" \
		$(poppler_glib_public_headers) ) > xgen-$(@F) \
	&& ( cmp -s xgen-$(@F) $(@F:stamp-%=%) || cp xgen-$(@F) $(@F:stamp-%=%) ) \
	&& rm -f xgen-$(@F) \
	&& echo timestamp > $(@F)

poppler-enums.c: $(poppler_glib_public_headers)
	( cd $(srcdir) && $(GLIB_MKENUMS) \
		--fhead "#include \"poppler-enums.h\"\n\n" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--fprod "\n#include \"@filename@\"" \
		--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
		--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "  { 0, NULL, NULL }\n};\n\n" \
		--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
		--vtail "  static GType type = 0;\n\n" \
		--vtail "  if (G_UNLIKELY (type == 0))\n" \
		--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
		--vtail "  return type;\n}\n\n" \
		$(poppler_glib_public_headers) ) > xgen-$(@F) \
	&& ( cmp -s xgen-$(@F) $(@F) || cp xgen-$(@F) $(@F) ) \
	&& rm -f xgen-$(@F)

poppler_glib_public_headers =			\
	poppler-action.h			\
	poppler-document.h			\
	poppler-page.h				\
	poppler-attachment.h			\
	poppler-form-field.h			\
	poppler-annot.h				\
	poppler.h

poppler_glib_includedir = $(includedir)/poppler/glib

poppler_glib_include_HEADERS =			\
	$(poppler_glib_public_headers)		\
	poppler-enums.h				\
	poppler-features.h

lib_LTLIBRARIES = libpoppler-glib.la
libpoppler_glib_la_SOURCES =			\
	$(BUILT_SOURCES)			\
	poppler-action.cc			\
	poppler-document.cc			\
	poppler-page.cc				\
	poppler-attachment.cc			\
	poppler-form-field.cc			\
	poppler-annot.cc			\
	poppler.cc				\
	poppler-private.h

libpoppler_glib_la_LIBADD =				\
	$(top_builddir)/poppler/libpoppler.la		\
	$(POPPLER_GLIB_LIBS)				\
	$(GDK_LIBS)					\
	$(FREETYPE_LIBS)				\
	$(FONTCONFIG_LIBS)				\
	$(cairo_libs)

libpoppler_glib_la_LDFLAGS = -version-info 3:0:0

if BUILD_WITH_GDK
noinst_PROGRAMS = test-poppler-glib

test_poppler_glib_SOURCES =			\
       test-poppler-glib.cc

test_poppler_glib_LDADD =			\
	$(top_builddir)/poppler/libpoppler.la	\
	libpoppler-glib.la			\
	$(POPPLER_GLIB_LIBS)			\
	$(GDK_LIBS)				\
	$(FREETYPE_LIBS)			\
	$(cairo_libs)
endif

BUILT_SOURCES =					\
	poppler-enums.c				\
	poppler-enums.h				\
	poppler-features.h

CLEANFILES = $(BUILT_SOURCES) $(stamp_files)
DISTCLEANFILES = $(BUILT_SOURCES) $(stamp_files)
MAINTAINERCLEANFILES = $(BUILT_SOURCES) $(stamp_files)
