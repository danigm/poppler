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
	poppler-fonts test-password-qt4


test_poppler_qt4_SOURCES =			\
       test-poppler-qt4.cpp

test_poppler_qt4_LDADD = $(LDADDS)

test_password_qt4_SOURCES =			\
       test-password-qt4.cpp

test_password_qt4_LDADD = $(LDADDS)


poppler_fonts_SOURCES =			\
       poppler-fonts.cpp

poppler_fonts_LDADD = $(LDADDS)



stress_poppler_qt4_SOURCES =			\
       stress-poppler-qt4.cpp

stress_poppler_qt4_LDADD = $(LDADDS)

TESTS = \
	check_author           \
	check_orientation      \
	check_permissions      \
	check_linearised       \
	check_pagemode_fullscreen \
	check_pagemode_attach  \
	check_pagemode_none    \
	check_pagemode_thumbs  \
	check_pagemode_oc      \
	check_pagelayout_none  \
	check_pagelayout_single \
	check_pagelayout_facing \
	check_version


check_PROGRAMS = $(TESTS)

check_orientation_SOURCES = check_orientation.cpp
check_orientation_LDADD = $(LDADDS)

check_linearised_SOURCES = check_linearised.cpp
check_linearised_LDADD = $(LDADDS)

check_author_SOURCES = check_author.cpp
check_author_LDADD = $(LDADDS)

check_permissions_SOURCES = check_permissions.cpp
check_permissions_LDADD = $(LDADDS)

check_pagemode_fullscreen_SOURCES = check_pagemode_fullscreen.cpp
check_pagemode_fullscreen_LDADD = $(LDADDS)

check_pagemode_attach_SOURCES = check_pagemode_attach.cpp
check_pagemode_attach_LDADD = $(LDADDS)

check_pagemode_none_SOURCES = check_pagemode_none.cpp
check_pagemode_none_LDADD = $(LDADDS)

check_pagemode_thumbs_SOURCES = check_pagemode_thumbs.cpp
check_pagemode_thumbs_LDADD = $(LDADDS)

check_pagemode_oc_SOURCES = check_pagemode_oc.cpp
check_pagemode_oc_LDADD = $(LDADDS)

check_pagelayout_single_SOURCES = check_pagelayout_single.cpp
check_pagelayout_single_LDADD = $(LDADDS)

check_pagelayout_none_SOURCES = check_pagelayout_none.cpp
check_pagelayout_none_LDADD = $(LDADDS)

check_pagelayout_facing_SOURCES = check_pagelayout_facing.cpp
check_pagelayout_facing_LDADD = $(LDADDS)

check_version_SOURCES = check_version.cpp
check_version_LDADD = $(LDADDS)
