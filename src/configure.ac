# versioning
m4_define([unique_major_version], [3])
m4_define([unique_minor_version], [0])
m4_define([unique_micro_version], [2])
m4_define([unique_version], [unique_major_version.unique_minor_version.unique_micro_version])
m4_define([unique_api_version], [unique_major_version.unique_minor_version])

# if functions have been added, set to 0; otherwise, increment
# with every release
m4_define([unique_interface_age], [2])
m4_define([unique_binary_age], [m4_eval(100 * unique_minor_version + unique_micro_version)])
# This is the X.Y used in -lunique-FOO-X.Y
m4_define([unique_api_version], [3.0])
# This is the X.Y used in the protocol negotiation
m4_define([unique_protocol_version], [1.0])

AC_PREREQ([2.63])

AC_INIT([unique], [unique_version],
        [http://bugzilla.gnome.org/enter_bug.cgi?product=libunique],
        [libunique])

AC_CONFIG_SRCDIR([unique/unique.h])
AC_CONFIG_MACRO_DIR([build/autotools])

AM_INIT_AUTOMAKE([1.11 no-define -Wno-portability dist-bzip2 no-dist-gzip])
AM_CONFIG_HEADER([config.h])

AM_SILENT_RULES([yes])

AC_PROG_CC_C_O

AM_PATH_GLIB_2_0

LT_PREREQ([2.2])
LT_INIT([disable-static])

# Honor aclocal flags
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

# version symbols
UNIQUE_MAJOR_VERSION=unique_major_version
UNIQUE_MINOR_VERSION=unique_minor_version
UNIQUE_MICRO_VERSION=unique_micro_version
UNIQUE_VERSION=unique_version
UNIQUE_API_VERSION=unique_api_version
UNIQUE_PROTOCOL_VERSION=unique_protocol_version
AC_SUBST(UNIQUE_MAJOR_VERSION)
AC_SUBST(UNIQUE_MINOR_VERSION)
AC_SUBST(UNIQUE_MICRO_VERSION)
AC_SUBST(UNIQUE_VERSION)
AC_SUBST(UNIQUE_API_VERSION)
AC_SUBST(UNIQUE_PROTOCOL_VERSION)

# libtool
m4_define([lt_current], [m4_eval(100 * unique_minor_version + unique_micro_version - unique_interface_age)])
m4_define([lt_revision], [unique_interface_age])
m4_define([lt_age], [m4_eval(unique_binary_age - unique_interface_age)])
UNIQUE_LT_VERSION_INFO="lt_current:lt_revision:lt_age"
UNIQUE_LT_CURRENT_MINUS_AGE=m4_eval(lt_current - lt_age)
AC_SUBST(UNIQUE_LT_VERSION_INFO)
AC_SUBST(UNIQUE_LT_CURRENT_MINUS_AGE)

m4_define([glib_required], [2.12.0])
m4_define([gtk_required],  [2.90.0])
m4_define([dbus_required], [0.70])

PKG_CHECK_MODULES(UNIQUE, glib-2.0 >= glib_required dnl
                          gtk+-3.0 >= gtk_required)

GTK_CHECK_BACKEND([x11], [gtk_required],
                  [
                    AC_PATH_XTRA

                    PKG_CHECK_MODULES(UNIQUE_X11, x11)

                    UNIQUE_CFLAGS="$UNIQUE_CFLAGS $UNIQUE_X11_CFLAGS"
                    UNIQUE_LIBS="$UNIQUE_LIBS $UNIQUE_X11_LIBS"
                  ]
)

AC_SUBST(UNIQUE_CFLAGS)
AC_SUBST(UNIQUE_LIBS)

dnl D-Bus backend dependencies
m4_define([have_dbus_default], [yes])
AC_ARG_ENABLE([dbus],
              [AC_HELP_STRING([--enable-dbus=@<:@yes/no@:>@],
                              [Whether D-BUS IPC should be enabled])],
              [],
              [enable_dbus=have_dbus_default])

AS_CASE([$enable_dbus],

        [yes],
        [
          PKG_CHECK_MODULES(DBUS, dbus-glib-1 >= dbus_required,
                            [have_dbus=yes],
                            [have_dbus=no])
        ],

        [no], [have_dbus=no],

        [AC_MSG_ERROR([Unknown argument to --enable-dbus])]
)

AS_IF([test "x$have_dbus" = "xyes"],
      [
        AC_SUBST(DBUS_CFLAGS)
        AC_SUBST(DBUS_LIBS)
        AC_DEFINE([HAVE_DBUS], [1], [Building with D-Bus support])
        AC_PATH_PROG(DBUS_BINDING_TOOL, [dbus-binding-tool])
      ]
)

AM_CONDITIONAL([HAVE_DBUS], [test "x$have_dbus" = "xyes"])

dnl GDBus backend
dnl This is the default backend if GIO is recent enough
m4_define([gdbus_gio_required],[2.25.7])
PKG_CHECK_MODULES([GDBUS],[gio-2.0 >= gdbus_gio_required],[have_gdbus=yes],[have_gdbus=no])

AS_IF([test "x$have_gdbus" = "xyes"],
  [
    AC_DEFINE([HAVE_GDBUS],[1],[Define if GDBus backend is enabled])
  ]
  )

AM_CONDITIONAL([HAVE_GDBUS],[test "$have_gdbus" = "yes"])

dnl Bacon backend
dnl This is the fallback backend, so we *need* these headers and functions
dnl even if we end up using D-Bus
m4_define([have_bacon_default], [yes])
AC_ARG_ENABLE([bacon],
              [AC_HELP_STRING([--enable-bacon=@<:@yes/no@:>@],
                              [Whether Unix domain sockets IPC should be enabled])],
              [],
              [enable_bacon=have_bacon_default])

AS_IF([test "x$enable_bacon" = "xyes"],
      [
        AC_CHECK_HEADERS([fcntl.h sys/types.h sys/socket.h sys/un.h],
                         [have_bacon=yes],
                         [have_bacon=no])
      ],

      [have_bacon=no]
)

AS_IF([test "x$have_bacon" = "xyes"],
      [
        AC_DEFINE([HAVE_BACON], [1], [Building with Unix domain socket support])
      ]
)

AM_CONDITIONAL([HAVE_BACON], [test "x$have_bacon" = "xyes"])

dnl Choose the default backend
AC_MSG_CHECKING([for default IPC mechanism])
AS_IF([test "x$have_gdbus" = "xyes"],
      [
        UNIQUE_DEFAULT_BACKEND=gdbus
        AC_MSG_RESULT([GDBus])
      ],

      [test "x$have_dbus" = "xyes"],
      [
        UNIQUE_DEFAULT_BACKEND=dbus
        AC_MSG_RESULT([D-Bus])
      ],

      [test "x$have_bacon" = "xyes"],
      [
        UNIQUE_DEFAULT_BACKEND=bacon
        AC_MSG_RESULT([Unix domain sockets])
      ],

      [AC_MSG_ERROR([No IPC backend enabled.])]
)

AC_SUBST(UNIQUE_DEFAULT_BACKEND)

# use strict compiler flags only during development cycles
m4_define([maintainer_flags_default], [m4_if(m4_eval(unique_minor_version % 2), [1], [yes], [no])])
AC_ARG_ENABLE([maintainer-flags],
              [AC_HELP_STRING([--enable-maintainer-flags=@<:@no/yes@:>@],
                              [Use strict compiler flags @<:@default=maintainer_flags_default@:>@])],
              [],
              [enable_maintainer_flags=maintainer_flags_default])

AS_IF([test "x$enable_maintainer_flags" = "xyes" && test "x$GCC" = "xyes"],
      [
        AS_COMPILER_FLAGS([MAINTAINER_CFLAGS],
                          ["-Wall -Wshadow -Wcast-align -Wuninitialized
                            -Wno-strict-aliasing -Wempty-body -Wformat
                            -Wformat-security -Winit-self
                            -Wdeclaration-after-statement -Wvla"])
      ]
)

AC_SUBST(MAINTAINER_CFLAGS)

# enable debug flags and symbols
m4_define([debug_default], [m4_if(m4_eval(unique_minor_version % 2), [1], [yes], [minimum])])
AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
                              [Turn on debugging @<:@default=debug_default@:>@])],
              [],
              [enable_debug=debug_default])

AS_CASE([$enable_debug],

        [yes],
        [
          test "$cflags_set" = set || CFLAGS="$CFLAGS -g"
          UNIQUE_DEBUG_CFLAGS="-DUNIQUE_ENABLE_DEBUG"
        ],

        [no],
        [
          UNIQUE_DEBUG_CFLAGS="-DG_DISABLE_ASSERT -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS"
        ],

        [minimum],
        [
          UNIQUE_DEBUG_CFLAGS="-DG_DISABLE_CAST_CHECKS"
        ],

        [AC_MSG_ERROR([Unknown argument to --enable-debug])]
)

AC_SUBST(UNIQUE_DEBUG_CFLAGS)

# introspection
GOBJECT_INTROSPECTION_CHECK([0.9.0])

# gtk-doc
GLIB_PREFIX="`$PKG_CONFIG --variable=prefix glib-2.0`"
GDK_PREFIX="`$PKG_CONFIG --variable=prefix gtk+-3.0`"
GTK_PREFIX="`$PKG_CONFIG --variable=prefix gdk-3.0`"
AC_SUBST(GLIB_PREFIX)
AC_SUBST(GDK_PREFIX)
AC_SUBST(GTK_PREFIX)

GTK_DOC_CHECK([1.13])

AC_CONFIG_FILES([
        Makefile
        unique.pc
        build/Makefile
        build/autotools/Makefile
        doc/Makefile
        doc/reference/Makefile
        doc/reference/version.xml
        unique/Makefile
        unique/uniqueversion.h
        unique/bacon/Makefile
        unique/dbus/Makefile
        unique/gdbus/Makefile
        tests/Makefile
])

AC_OUTPUT

echo "
Unique $UNIQUE_VERSION

Configuration:
            Maintainer flags: $enable_maintainer_flags
                 Debug level: $enable_debug
         Build documentation: $enable_gtk_doc
    Build introspection data: $enable_introspection

Backends:
          Unix Domain Socket: $have_bacon
                       D-BUS: $have_dbus
                       GDBus: $have_gdbus

             Default backend: $UNIQUE_DEFAULT_BACKEND
"
