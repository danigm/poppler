INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	-I$(top_srcdir)/qt4/src			\
	$(POPPLER_QT4_CXXFLAGS)

LDADDS =					\
	$(top_builddir)/poppler/libpoppler.la	\
	$(top_builddir)/qt4/src/libpoppler-qt4.la	\
	$(POPPLER_QT4_LIBS)

noinst_PROGRAMS = test-poppler-qt4 stress-poppler-qt4 \
	poppler-fonts


test_poppler_qt4_SOURCES =			\
       test-poppler-qt4.cpp

test_poppler_qt4_LDADD = $(LDADDS)


poppler_fonts_SOURCES =			\
       poppler-fonts.cpp

poppler_fonts_LDADD = $(LDADDS)



stress_poppler_qt4_SOURCES =			\
       stress-poppler-qt4.cpp

stress_poppler_qt4_LDADD = $(LDADDS)

TESTS = \
	check_author           \
	check_orientation      \
	check_permissions


check_PROGRAMS = $(TESTS)

check_orientation_SOURCES = check_orientation.cpp
check_orientation_LDADD = $(LDADDS)

check_author_SOURCES = check_author.cpp
check_author_LDADD = $(LDADDS)

check_permissions_SOURCES = check_permissions.cpp
check_permissions_LDADD = $(LDADDS)
