include $(top_srcdir)/build/autotools/Makefile.am.silent

ACLOCAL_AMFLAGS = -I build/autotools

SUBDIRS = build unique tests doc

unique-3.0.pc: unique.pc
	$(QUIET_GEN)cp -f $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = unique-3.0.pc

EXTRA_DIST = unique.pc.in NEWS TODO

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --disable-rebuilds

DISTCLEANFILES = unique-3.0.pc

dist-hook:
	@if test -d "$(srcdir)/.git"; then \
	  echo Generating ChangeLog ; \
	  ( cd "$(srcdir)" \
	    && $(top_srcdir)/missing --run git log --stat ) > ChangeLog.tmp \
	  && mv -f ChangeLog.tmp $(top_distdir)/ChangeLog \
	  || ( rm -f ChangeLog.tmp; \
	       echo Failed to generate ChangeLog >&2 ); \
	else \
	  echo A git checkout is required to generate a ChangeLog >&2; \
	fi
