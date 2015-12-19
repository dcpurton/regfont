SUBDIRS = src
dist_doc_DATA = ChangeLog COPYING README
bindistdir = $(PACKAGE)-$(VERSION)-$(host_cpu)

install-data-hook:
	make distdir=$(PACKAGE)-$(VERSION)-src dist-zip
	$(install_sh_DATA) $(distdir)-src.zip $(bindistdir)

bindist:
	make bindir=/ docdir=/ DESTDIR=$(abs_top_builddir)/$(bindistdir) install-strip
	zip -rq $(bindistdir).zip $(bindistdir)
	rm -rf $(bindistdir)

