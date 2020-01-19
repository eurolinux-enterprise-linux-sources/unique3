include $(top_srcdir)/build/autotools/Makefile.am.silent

noinst_LTLIBRARIES = libunique-gdbus.la

libunique_gdbus_la_SOURCES = 	\
	uniquebackend-gdbus.h	\
	uniquebackend-gdbus.c

libunique_gdbus_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Unique-GDBus\"	\
	-DG_DISABLE_SINGLE_INCLUDES	\
	-I$(top_srcdir)			\
	$(AM_CPPFLAGS)

libunique_gdbus_la_CFLAGS =	\
	$(UNIQUE_CFLAGS)	\
	$(UNIQUE_DEBUG_CFLAGS)	\
	$(MAINTAINER_CFLAGS)	\
	$(GDBUS_CFLAGS)		\
	$(AM_CFLAGS)

libunique_gdbus_la_LIBADD = \
	$(UNIQUE_LIBS) 	\
	$(GDBUS_LIBS)
