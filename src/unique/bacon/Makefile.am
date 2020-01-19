libuniqueincludedir = $(includedir)/unique-1.0/unique

INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Unique-Bacon\"	\
	-DG_DISABLE_SINGLE_INCLUDES

LDADD = $(UNIQUE_LIBS)

noinst_LTLIBRARIES = libunique-bacon.la

libunique_bacon_la_SOURCES = 	\
	uniquebackend-bacon.c	\
	uniquebackend-bacon.h	\
	uniquefactory-bacon.c	\
	uniquefactory-bacon.h	\
	uniquemessage-bacon.c	\
	uniquemessage-bacon.h

libunique_bacon_la_CFLAGS =	\
	$(UNIQUE_CFLAGS)	\
	$(UNIQUE_DEBUG_FLAGS)	\
	$(MAINTAINER_CFLAGS)
