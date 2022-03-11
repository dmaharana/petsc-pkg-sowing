ALL: runtests
SHELL = /bin/sh

DOCTEXT = ../doctext

# Location of the files that provide the command defintions.
# This choice allows running testing without installing doctext first
DEFNDIR = ../../../share/doctext

# doctext allows the quote characters to be used to change font (emphasis
# and text).  So there are three cases of input files:
#   FILES_NO_QUOTEFMT - files without any quotes
#   FILES_QUOTEFMT    - files that use quotes for formatting
#   FILES_QUOTE_NOFMT - files with quotes, but not for formatting
#
# FILES = all files
# FILES_USING_QUOTE are all files that either have no quotes, or use the
#   quote for formatting
# There are a few files that are processed separately; this is to keep them
# from that general output files like f.tex.
FILES_QUOTEFMT    = allfmt.c tst3.c # mesh.c
FILES_NOQUOTEFMT  = test.nam tstpgm.c tstpgma.c tstpgmc.c tstpgm.h # estest.h
FILES_QUOTE_NOFMT = tst1.c tststra.h # c.h
FILES_OLDSTYLE    = oldst.c

FILES            = $(FILES_QUOTEFMT) $(FILES_NOQUOTEFMT) $(FILES_QUOTE_NOFMT)
FILES_USING_QUOTE = $(FILES_QUOTEFMT) $(FILES_NOQUOTEFMT)

TESTFILES = tst.f tst2.c tdollar.h estest.h eopindex.def \
	indent-html.def indent-latex.def indent-man.def tst.cit pubinc \
	fhtml.def oldst.c
SOURCES = $(FILES) $(TESTFILES)
DIST_COMMON = Makefile.in
DISTSUBDIRS = testref defn

runtests: Makefile
	rm -f *.3 *.2 *.tex *.html
	(export DOCTEXT_HEADING=PETSc; export DOCTEXT_PATH=$(DEFNDIR) ; \
		$(DOCTEXT) -defn indent-man.def -date "2/10/2000" $(FILES) )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -html -defn indent-html.def -date "2/10/2000" $(FILES) )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	   $(DOCTEXT) -latex -defn indent-latex.def -date "2/10/2000" $(FILES) )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -I pubinc -quotefmt -heading MTEST \
		-ext 2 -defn indent-man.def -date "2/10/2000" \
		$(FILES_USING_QUOTE) )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -I pubinc -outfile f.tex -quotefmt -latex \
		-heading MTEST \
		-defn indent-latex.def -date "2/10/2000" $(FILES_USING_QUOTE) )
	rm -f f1.cit
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -I pubinc -index f1.cit -outfile f.html -quotefmt -html \
		-heading MTEST \
		-mapref tst.cit -defn indent-html.def -date "2/10/2000" \
		$(FILES_USING_QUOTE) )
	(export DOCTEXT_PATH="defn" ; \
	    $(DOCTEXT)  -I pubinc -outfile f2.html -quotefmt -html \
		-heading MTEST \
		-mapref tst.cit -defn indent-html.def -date "2/10/2000" \
		$(FILES_USING_QUOTE) ; )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT)  -I pubinc -outfile f3.html -quotefmt -html \
		-heading MTEST \
		-mapref tst.cit -defn eopindex.def -defn indent-html.def \
		-date "2/10/2000" $(FILES_USING_QUOTE) )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -html -defn htmlargtbl.def -dosnl \
		-date "2/10/2000" tst1.c )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -html -outfile tstcolor.html -defn htmlargtbl.def \
		-defn htmlcolor.def -dosnl -date "2/10/2000" tst1.c )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -html -outfile tst3.html -defn htmlargtbl.def \
		-defn htmlcolor.def -date "2/10/2000" tst3.c )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -html -skipprefix C -dosnl -date "2/10/2000" tst.f )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -skipprefix C -date "2/10/2000" tst.f )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -ignore EXPORT_API -date "2/10/2000" tst2.c )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -html \
	        -defn fhtml.def -outfile tstreg.html tst1.c )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -date "2/10/2000" estest.h )
	rm -f es.cit
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -html -date "2/10/2000" -index es.cit estest.h )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -html tdollar.h )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -oldargstyle -html -defn htmlargtbl.def \
		-date "2/10/2000" $(FILES_OLDSTYLE) )
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	   $(DOCTEXT) -myst -defn $(FILES) )

# This test is not part of the default tests, in part because it generates
# large number of output files, and in part because the original reason
# for including it has been lost (but probably because an early version
# of doctext failed to handle some of the routine definitions).  The
# original test also included two additional arguments:
#   -mapref petsc.cit
#   -defn sec/eop.Vec
# (drawn from the PETSc source).  These are no longer available and
# thus not included in the test
special-test: mesh.c petschtml.def
	if ! -d man3-test ; then mkdir man3-test ; fi
	(export DOCTEXT_PATH=$(DEFNDIR) ; \
	    $(DOCTEXT) -html -mpath man3-test -heading PETSc \
		-defn petschtml.def \
		-locdir foobar/ -date 2/10/2000 mesh.c )

compare:
	-@for file in testref/* ; do \
	f2=`basename $$file` ; \
	if diff -rb $$f2 $$file >/dev/null 2>&1 ; then \
	    : ; \
	else \
	    echo "File $$file is different" ; \
	    diff -rb $$f2 $$file ; \
	fi ; \
	done

check-complete:
	@for file in *.2 *.3 *.tex *.html ; do \
	if test ! -s "$$file" ; then echo "make runtests first" ; exit 1; fi ;\
	if test ! -s testref/$$file ; then \
	    echo "testref/$$file is missing!" ; fi ; done

testing: runtests compare
check: runtests compare

dist:
	@if [ "X$(distdir)" == "X" ] ; then echo "Set distdir" ; exit 1 ; fi
	@if [ ! -d $(distdir) ] ; then mkdir $(distdir) ; fi
	if [ "X$(SOURCES)" != "X" ] ; then cp $(SOURCES) $(distdir) ; fi
	if [ "X$(HEADER_FILES)" != "X" ] ; then cp $(HEADER_FILES) $(distdir) ; fi
	if [ "X$(DIST_COMMON)" != "X" ] ; then cp $(DIST_COMMON) $(distdir) ; fi
	for dir in $(SUBDIRS) ; do \
		(cd $$dir && $(MAKE) distdir=../$(distdir)/$$dir dist ) ; done
	for dir in $(DISTSUBDIRS) ; do \
		cp -rp $$dir $(distdir) ; done
	if [ "X$(DOC_FILES)" != "X" ] ; then cp $(DOC_FILES) $(distdir) ; fi

clean:
	rm -f *.o *~ *.3 *.2 *.html *.tex f1.cit
	@if test -s man3-test/MeshView.html ; then rm -rf man3-test ; fi

Makefile: Makefile.in
	(cd ../../.. && export CONFIG_FILES=src/doctext/test/Makefile && \
	 ./config.status)
