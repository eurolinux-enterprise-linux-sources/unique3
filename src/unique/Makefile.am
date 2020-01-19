include $(top_srcdir)/build/autotools/Makefile.am.silent

NULL =

SUBDIRS =

if HAVE_BACON
SUBDIRS += bacon
endif

if HAVE_DBUS
SUBDIRS += dbus
endif

if HAVE_GDBUS
SUBDIRS += gdbus
endif

DIST_SUBDIRS = bacon dbus gdbus

INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = \
	-DG_DISABLE_SINGLE_INCLUDES	\
	-DGTK_DISABLE_SINGLE_INCLUDES	\
	-DG_DISABLE_DEPRECATED		\
	-DGTK_DISABLE_DEPRECATED	\
	-DG_LOG_DOMAIN=\"Unique\"	\
	-DPREFIX=\""$(prefix)"\"	\
	$(DISABLE_DEPRECATED_FLAGS)	\
	$(UNIQUE_DEBUG_CFLAGS)		\
	$(NULL)

AM_CFLAGS = $(MAINTAINER_CFLAGS) $(DBUS_CFLAGS) $(UNIQUE_CFLAGS)

BUILT_SOURCES =

EXTRA_DIST =
CLEANFILES =
DISTCLEANFILES =

unique_sources_h = \
	$(top_srcdir)/unique/uniqueapp.h 	\
	$(top_srcdir)/unique/uniquebackend.h	\
	$(top_srcdir)/unique/uniquemessage.h	\
	$(NULL)

unique_sources_c = \
	$(srcdir)/uniqueapp.c			\
	$(srcdir)/uniquebackend.c		\
	$(srcdir)/uniquemessage.c		\
	$(NULL)

unique_sources_priv_h = \
	$(top_srcdir)/unique/uniqueinternals.h	\
	$(NULL)

# glib-mkenums rules
glib_enum_h = uniqueenumtypes.h
glib_enum_c = uniqueenumtypes.c
glib_enum_headers = $(unique_sources_h)
include $(top_srcdir)/build/autotools/Makefile.am.enums

# glib-genmarshal rules
glib_marshal_list = uniquemarshal.list
glib_marshal_prefix = unique_marshal
include $(top_srcdir)/build/autotools/Makefile.am.marshal

unique_backend_libs =

if HAVE_BACON
unique_backend_libs += $(top_builddir)/unique/bacon/libunique-bacon.la
endif

if HAVE_DBUS
unique_backend_libs += $(top_builddir)/unique/dbus/libunique-dbus.la
endif

if HAVE_GDBUS
unique_backend_libs += $(top_builddir)/unique/gdbus/libunique-gdbus.la
endif

uniquedir = $(includedir)/unique-3.0/unique
unique_HEADERS = \
	$(unique_sources_h) 				\
	$(top_builddir)/unique/uniqueenumtypes.h	\
	$(top_builddir)/unique/uniqueversion.h		\
	$(top_srcdir)/unique/unique.h			\
	$(NULL)

lib_LTLIBRARIES = libunique-3.0.la

libunique_3_0_la_SOURCES = 		\
	$(unique_sources_c) 		\
	$(unique_sources_priv_h) 	\
	$(BUILT_SOURCES)

libunique_3_0_la_LIBADD = 		\
	$(unique_backend_libs) 		\
	$(UNIQUE_LIBS)			\
	$(DBUS_LIBS)

libunique_3_0_la_LDFLAGS = 			\
	-version-info $(UNIQUE_LT_VERSION_INFO)	\
	-export-dynamic				\
	-export-symbols-regex "^unique.*"	\
	-rpath $(libdir)			\
	-no-undefined

DISTCLEANFILES += uniqueversion.h

EXTRA_DIST += uniqueversion.h.in

# introspection
-include $(INTROSPECTION_MAKEFILE)

if HAVE_INTROSPECTION
INTROSPECTION_GIRS = Unique-3.0.gir

Unique-3.0.gir: libunique-3.0.la Makefile

Unique_3_0_gir_NAMESPACE = Unique
Unique_3_0_gir_VERSION = 3.0
Unique_3_0_gir_LIBS = libunique-3.0.la
Unique_3_0_gir_FILES = $(unique_sources_h) $(unique_sources_c) uniqueenumtypes.h uniqueenumtypes.c
Unique_3_0_gir_CFLAGS = $(INCLUDES) $(AM_CPPFLAGS)
Unique_3_0_gir_INCLUDES = GObject-2.0 Gtk-3.0
Unique_3_0_gir_SCANNERFLAGS = --warn-all --pkg-export unique-3.0 --c-include "unique/unique.h"

girdir = $(datadir)/gir-1.0
dist_gir_DATA = Unique-3.0.gir

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = Unique-3.0.typelib

CLEANFILES += $(dist_gir_DATA) $(typelibs_DATA)
endif # HAVE_INTROSPECTION
