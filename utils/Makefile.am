INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler

LDADD =						\
	$(top_builddir)/poppler/libpoppler.la

bin_PROGRAMS = pdffonts pdfimages pdfinfo pdftops pdftotext pdftohtml

man1_MANS = pdffonts.1 pdfimages.1 pdfinfo.1 pdftops.1 pdftotext.1 pdftohtml.1

pdffonts_SOURCES	= pdffonts.cc parseargs.c
pdfimages_SOURCES	= pdfimages.cc ImageOutputDev.cc parseargs.c
pdfinfo_SOURCES		= pdfinfo.cc parseargs.c
pdftops_SOURCES		= pdftops.cc parseargs.c
pdftotext_SOURCES	= pdftotext.cc parseargs.c
pdftohtml_SOURCES	= pdftohtml.cc parseargs.c	\
	HtmlFonts.cc HtmlLinks.cc HtmlOutputDev.cc
