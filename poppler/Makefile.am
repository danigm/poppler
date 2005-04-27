if BUILD_SPLASH_OUTPUT

splash_sources =				\
	SplashOutputDev.cc

splash_headers =				\
	SplashOutputDev.h

splash_includes =				\
	$(SPLASH_CFLAGS)

splash_libs =					\
	$(SPLASH_LIBS)				\
	$(top_builddir)/splash/libsplash.la

endif


if BUILD_CAIRO_OUTPUT

cairo_sources =					\
	CairoFontEngine.cc			\
	CairoOutputDev.cc			\
	CairoOutputDevImage.cc			\
	CairoOutputDevX.cc

cairo_headers =					\
	CairoFontEngine.h			\
	CairoOutputDev.h			\
	CairoOutputDevImage.h			\
	CairoOutputDevX.h

cairo_includes =				\
	$(CAIRO_CFLAGS)

cairo_libs =					\
	$(CAIRO_LIBS)

endif

if BUILD_LIBJPEG

libjpeg_sources =				\
	DCTStream.h				\
	DCTStream.cc

libjpeg_libs =					\
	$(LIBJPEG_LIBS)

endif

if BUILD_ZLIB

zlib_sources =					\
	FlateStream.h				\
	FlateStream.cc

zlib_libs = 					\
	$(ZLIB_LIBS)

endif

INCLUDES =					\
	-I$(top_srcdir)				\
	$(splash_includes)			\
	$(cairo_includes)			\
	-DDATADIR=\""$(datadir)"\"

lib_LTLIBRARIES = libpoppler.la

libpoppler_la_LIBADD =				\
	$(top_builddir)/goo/libgoo.la		\
	$(top_builddir)/fofi/libfofi.la		\
	$(splash_libs)				\
	$(cairo_libs)				\
	$(libjpeg_libs)				\
	$(zlib_libs)

poppler_includedir = $(includedir)/poppler
poppler_include_HEADERS =	\
	$(splash_headers)	\
	$(cairo_headers)	\
	Annot.h			\
	Array.h			\
	BaseFile.h		\
	BuiltinFont.h		\
	BuiltinFontTables.h	\
	Catalog.h		\
	CharCodeToUnicode.h	\
	CMap.h			\
	Decrypt.h		\
	Dict.h			\
	Error.h			\
	FontEncodingTables.h	\
	Function.cc		\
	Function.h		\
	Gfx.h			\
	GfxFont.h		\
	GfxState.h		\
	GlobalParams.h		\
	JArithmeticDecoder.h	\
	JBIG2Stream.h		\
	JPXStream.h		\
	Lexer.h			\
	Link.h			\
	NameToCharCode.h	\
	Object.h		\
	Outline.h		\
	OutputDev.h		\
	Page.h			\
	Parser.h		\
	PDFDoc.h		\
	PDFDocEncoding.h	\
	PSTokenizer.h		\
	Stream-CCITT.h		\
	Stream.h		\
	UnicodeMap.h		\
	UnicodeMapTables.h	\
	UnicodeTypeTable.h	\
	XRef.h			\
	CharTypes.h		\
	CompactFontTables.h	\
	ErrorCodes.h		\
	NameToUnicodeTable.h	\
	PSOutputDev.h		\
	TextOutputDev.h		\
	UTF8.h			\
	poppler-config.h

libpoppler_la_SOURCES =		\
	$(splash_sources)	\
	$(cairo_sources)	\
	$(libjpeg_sources)	\
	$(zlib_sources)		\
	Annot.cc		\
	Array.cc 		\
	BuiltinFont.cc		\
	BuiltinFontTables.cc	\
	Catalog.cc 		\
	CharCodeToUnicode.cc	\
	CMap.cc			\
	Decrypt.cc		\
	Dict.cc 		\
	Error.cc 		\
	FontEncodingTables.cc	\
	Function.cc		\
	Gfx.cc 			\
	GfxFont.cc 		\
	GfxState.cc		\
	GlobalParams.cc		\
	JArithmeticDecoder.cc	\
	JBIG2Stream.cc		\
	JPXStream.cc		\
	Lexer.cc 		\
	Link.cc 		\
	NameToCharCode.cc	\
	Object.cc 		\
	Outline.cc		\
	OutputDev.cc 		\
	Page.cc 		\
	Parser.cc 		\
	PDFDoc.cc 		\
	PDFDocEncoding.cc	\
	PSTokenizer.cc		\
	Stream.cc 		\
	UnicodeMap.cc		\
	UnicodeTypeTable.cc	\
	XRef.cc			\
	PSOutputDev.cc		\
	TextOutputDev.cc	\
	PageLabelInfo.h		\
	PageLabelInfo.cc	
