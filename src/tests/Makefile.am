INCLUDES = \
	-DG_LOG_DOMAIN=\"Test-Unique\" 	\
	-I$(top_srcdir) 		\
	-I$(top_builddir)	 	\
	$(UNIQUE_CFLAGS) 		\
	$(UNIQUE_DEBUG_FLAGS)

DEPS = $(top_builddir)/unique/libunique-3.0.la
LDADD = $(top_builddir)/unique/libunique-3.0.la $(UNIQUE_LIBS)

noinst_PROGRAMS = 	\
	test-unique

test_unique_DEPENDENCIES = $(DEPS)
test_unique_LDADD = $(LDADD)

test_unique_SOURCES = test-unique.c
