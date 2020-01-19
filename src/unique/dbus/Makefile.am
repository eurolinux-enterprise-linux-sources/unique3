include $(top_srcdir)/build/autotools/Makefile.am.silent

libuniqueincludedir = $(includedir)/unique-1.0/unique

INCLUDES = \
	-DG_LOG_DOMAIN=\"Unique-DBus\"	\
	-DG_DISABLE_SINGLE_INCLUDES \
	-I$(top_srcdir)

LDADD = $(UNIQUE_LIBS) $(DBUS_LIBS)

noinst_LTLIBRARIES = libunique-dbus.la

unique_dbus_xml = $(srcdir)/uniquebackend-dbus.xml

BUILT_SOURCES =	uniquebackend-bindings.h uniquebackend-glue.h

libunique_dbus_la_SOURCES = 	\
	uniquefactory-dbus.h	\
	uniquefactory-dbus.c	\
	uniquebackend-dbus.h	\
	uniquebackend-dbus.c	\
	$(BUILT_SOURCES)

libunique_dbus_la_CFLAGS =	\
	$(UNIQUE_CFLAGS)	\
	$(UNIQUE_DEBUG_CFLAGS)	\
	$(MAINTAINER_CFLAGS)	\
	$(DBUS_CFLAGS)

uniquebackend-bindings.h: $(unique_dbus_xml) $(DBUS_BINDING_TOOL)
	$(QUIET_GEN)$(DBUS_BINDING_TOOL) \
		--mode=glib-server \
		--output=$@ \
		--prefix=uniquebackend \
	$(unique_dbus_xml)

uniquebackend-glue.h: $(unique_dbus_xml) $(DBUS_BINDING_TOOL)
	$(QUIET_GEN)$(DBUS_BINDING_TOOL) \
		--mode=glib-client \
		--output=$@ \
		--prefix=uniquebackend \
	$(unique_dbus_xml)

EXTRA_DIST = $(unique_dbus_xml)
