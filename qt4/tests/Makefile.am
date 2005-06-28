INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	-I$(top_srcdir)/qt4/src			\
	$(POPPLER_QT4_CXXFLAGS)

LDADDS =					\
	$(top_builddir)/poppler/libpoppler.la	\
	$(top_builddir)/qt4/src/libpoppler-qt4.la	\
	$(POPPLER_QT4_LIBS)

noinst_PROGRAMS = test-poppler-qt4 stress-poppler-qt4


test_poppler_qt4_SOURCES =			\
       test-poppler-qt4.cpp

test_poppler_qt4_LDADD = $(LDADDS)



stress_poppler_qt4_SOURCES =			\
       stress-poppler-qt4.cpp

stress_poppler_qt4_LDADD = $(LDADDS)

TESTS = \
	check_orientation


check_PROGRAMS = $(TESTS)

check_orientation_SOURCES = check_orientation.cpp
check_orientation_LDADD = $(LDADDS)
