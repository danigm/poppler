INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/utils			\
	-I$(top_srcdir)/poppler			\
	$(UTILS_CFLAGS)

LDADD =						\
	$(top_builddir)/poppler/libpoppler.la

bin_PROGRAMS = pdffonts pdfimages pdfinfo pdftops pdftotext pdftohtml

man1_MANS = pdffonts.1 pdfimages.1 pdfinfo.1 pdftops.1 pdftotext.1 pdftohtml.1

common = parseargs.c parseargs.h

pdffonts_SOURCES	= pdffonts.cc $(common)
pdfimages_SOURCES	= pdfimages.cc ImageOutputDev.cc $(common)
pdfinfo_SOURCES		= pdfinfo.cc $(common)
pdftops_SOURCES		= pdftops.cc $(common)
pdftotext_SOURCES	= pdftotext.cc $(common)
pdftohtml_SOURCES	= pdftohtml.cc $(common)	\
	HtmlFonts.cc HtmlLinks.cc HtmlOutputDev.cc
