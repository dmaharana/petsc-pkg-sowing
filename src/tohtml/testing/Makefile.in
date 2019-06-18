ALL: inplace subfiles

TOHTML = ../tohtml

SOURCES = test1.tex test2.tex test3.tex test4.tex test5.tex test6.tex \
	test7.tex test8.tex test9.tex test10.tex test11.tex test12.tex \
	test13.tex test14.tex test15.tex test16.tex test17.tex test18.tex \
	test19.tex test20.tex test21.tex test22.tex test23.tex test24.tex \
	test25.tex test26.tex test27.tex test28.tex test29.tex \
	test5.cit test5m.cit testf1.pdf test10.dat test10.cit
DIST_COMMON = Makefile.in README
DISTSUBDIRS = testref

inplace: $(inplace_files) ../tohtml
subfiles: $(split_files)  ../tohtml

inplace_files = test1.html test2.html test3.html test4.html test6.html \
        test7.html test7a.html \
	test8.html test9.html test10.html test11.html test12.html test13.html \
	test14.html test15.html test16.html test17.html test18.html \
	test25.html test26.html test27.html test28.html test29.html
split_files = test1/test1.html test2/test2.html test3/test3.html \
	test4/test4.html test6/test6.html test7/test7.html test20/test20.html

# Inplace option
# For these, be careful to remove the aux files
# for test1, run twice because is uses a reference and needs the aux file
test1.html: test1.tex $(TOHTML)
	-@rm -f test1.hux test1.aux test1.out test1.ler
	$(TOHTML) test1.tex
	$(TOHTML) test1.tex
test2.html: test2.tex $(TOHTML)
	-@rm -f test2.hux test2.aux test2.out test2.ler
	$(TOHTML) test2.tex
test3.html: test3.tex $(TOHTML)
	-@rm -f test3.hux test3.aux test3.out test3.ler
	$(TOHTML) test3.tex
test4.html: test4.tex $(TOHTML)
	-@rm -f test4.hux test4.aux test4.out test4.ler
	$(TOHTML) test4.tex
test6.html: test6.tex $(TOHTML)
	-@rm -f test6.hux test6.aux test6.out test6.ler
	$(TOHTML) test6.tex
test7.html: test7.tex $(TOHTML)
	-@rm -f test7.hux test7.aux test7.out test7.ler
	$(TOHTML) test7.tex
test8.html: test8.tex $(TOHTML)
	-@rm -f test8.hux test8.aux test8.out test8.ler
	$(TOHTML) test8.tex
test9.html: test9.tex $(TOHTML)
	-@rm -f test9.hux test9.aux test9.out test9.ler
	$(TOHTML) test9.tex
test10.html: test10.tex test10.dat $(TOHTML)
	-@rm -f test10.hux test10.aux test10.out test10.ler
	$(TOHTML) -mapman test10.cit test10.tex
test11.html: test11.tex $(TOHTML)
	-@rm -f test11.hux test11.aux test11.out test11.ler
	$(TOHTML) test11.tex
test12.html: test12.tex $(TOHTML)
	-@rm -f test12.hux test12.aux test12.out test12.ler
	$(TOHTML) test12.tex
test13.html: test13.tex $(TOHTML)
	-@rm -f test13.hux test13.aux test13.out test13.ler
	$(TOHTML) test13.tex
test14.html: test14.tex $(TOHTML)
	-@rm -f test14.hux test14.aux test14.out test14.ler
	$(TOHTML) test14.tex
test15.html: test15.tex $(TOHTML)
	-@rm -f foo.ps foo.gif ../foobar.eps ../foobar.gif
	-@rm -f test15.hux test15.aux test15.out test15.ler
	@echo "%! PS" > foo.ps
	@echo "%! PS" > ../foobar.eps
	@sleep 1
	@echo "dummy gif" > foo.gif
	@echo "dummy gif2" > ../foobar.gif
	$(TOHTML) test15.tex
	@if [ ! -s foobar.gif ] ; then echo "Failed to move foobar.gif" ; fi
	@/bin/rm -f foo.ps foo.gif ../foobar.eps ../foobar.gif foobar.gif
test16.html: test16.tex $(TOHTML)
	-@rm -f test16.hux test16.aux test16.out test16.ler
	$(TOHTML) test16.tex
test17.html: test17.tex $(TOHTML)
	-@rm -f test17.hux test17.aux test17.out test17.ler
	$(TOHTML) test17.tex
test18.html: test18.tex $(TOHTML)
	-@rm -f test18.hux test18.aux test18.out test18.ler
	$(TOHTML) test18.tex
test25.html: test25.tex $(TOHTML)
	-@rm -f test25.hux test25.aux test25.out test25.ler
	rm -f testf1.gif testf1.ps
	$(TOHTML) test25.tex
	@if [ ! -s testf1.gif ] ; then \
	    echo "Did not create gif file" ; \
	fi
	rm -f testf1.gif testf1.ps
test26.html: test26.tex $(TOHTML)
	-@rm -f test26.hux test26.aux test26.out test26.ler
	$(TOHTML) test26.tex
test27.html: test27.tex $(TOHTML)
	-@rm -f test27.hux test27.aux test27.out test27.ler
	$(TOHTML) -cvtlatex -imgbase test27img test27.tex
test28.html: test28.tex $(TOHTML)
	-@rm -f test28.hux test28.aux test28.out test28.ler
	$(TOHTML) test28.tex
test29.html: test29.tex $(TOHTML)
	-@rm -f test29.hux test29.aux test29.out test29.ler
	$(TOHTML) test29.tex

# default
test1/test1.html: test1.tex $(TOHTML)
	-@rm -f test1.hux test1.aux test1.out test1.ler
	$(TOHTML) -default test1.tex
	$(TOHTML) -default test1.tex
test2/test2.html: test2.tex $(TOHTML)
	-@rm -f test2.hux test2.aux test2.out test2.ler
	$(TOHTML) -default test2.tex
	$(TOHTML) -default test2.tex
test3/test3.html: test3.tex $(TOHTML)
	-@rm -f test3.hux test3.aux test3.out test3.ler
	$(TOHTML) -default test3.tex
	$(TOHTML) -default test3.tex
test4/test4.html: test4.tex $(TOHTML)
	-@rm -f test4.hux test4.aux test4.out test4.ler
	$(TOHTML) -default test4.tex
	$(TOHTML) -default test4.tex
test6/test6.html: test6.tex $(TOHTML)
	-@rm -f test6.hux test6.aux test6.out test6.ler
	$(TOHTML) -default test6.tex
	$(TOHTML) -default test6.tex
test7/test7.html: test7.tex $(TOHTML)
	-@rm -f test7.hux test7.aux test7.out test7.ler
	$(TOHTML) -default test7.tex
	$(TOHTML) -default test7.tex
test11/test11.html: test11.tex $(TOHTML)
	-@rm -f test11.hux test11.aux test11.out test11.ler
	$(TOHTML) -default test11.tex
	$(TOHTML) -default test11.tex
test12/test12.html: test12.tex $(TOHTML)
	-@rm -f test12.hux test12.aux test12.out test12.ler
	$(TOHTML) -default test12.tex
	$(TOHTML) -default test12.tex
test7a.html: test7.tex $(TOHTML)
	-@rm -f test7.hux test7.aux test7.out test7.ler
	rm -f test7a.html
	$(TOHTML) -default -htables -split -1 test7.tex -o test7a.html
	$(TOHTML) -default -htables -split -1 test7.tex -o test7a.html
test20/test20.html: test20.tex $(TOHTML)
	-@rm -f test20.hux test20.aux test20.out test20.ler
	$(TOHTML) -cvtmath -split 1 test20.tex

# default with active token processing
test5.html: test5.tex test5.cit $(TOHTML)
	-@rm -f test5.hux test5.aux test5.out test5.ler
	$(TOHTML) -mapref test5.cit -mapman test5m.cit test5.tex

testing: $(inplace_files) $(split_files) compare

compare-old:
	-@for file in testref/* ; do \
	if test -d $$file ; then \
	    for f in $$file/* ; do \
		newf=`echo $$f | sed -e 's%^testref/%%g'` ; \
		xten=`echo $$newf | sed -e 's/.*\.//g'` ; \
		if test $$xten = "xbm" ; then continue ; fi ; \
		if diff -b $$newf $$f >/dev/null 2>&1 ; then \
		  : ; \
		else \
		   echo "File $$f is different" ; \
		   diff -b $$newf $$f ; \
                fi ; \
	    done ; \
	else \
	   f2=`basename $$file` ; \
	   if diff -rb $$f2 $$file >/dev/null 2>&1 ; then \
	       : ; \
	   else \
	       echo "File $$file is different" ; \
	       diff -rb $$f2 $$file ; \
	   fi ; \
	fi ; \
	done

compare: $(inplace_files) $(split_files)
	@for file in $(inplace_files) ; do \
	   if test ! -s testref/$$file ; then \
		echo "Missing reference file testref/$$file!" ; continue ; \
	   fi ; \
	   if diff -b $$file testref/$$file >/dev/null 2>&1 ; then \
		: ; \
	   else \
	       echo "File $$file differs from reference:" ;\
	       diff -b $$file testref/$$file ; \
	   fi ; \
	done
	@for file in $(split_files) ; do \
	    if test ! -s testref/$$file ; then \
		echo "Missing reference split file or directory testref/$$file!" ; \
	    else \
	        sdir=`dirname $$file` ; \
	        for sfile in testref/$$sdir/* ; do \
		    if test -d testref/$$sdir/$$sfile ; then continue ; fi ;\
		    if diff -b $$sfile $$sdir >/dev/null 2>&1 ; then \
			: ;\
		    else \
			srcfile=`basename $$sfile` ;\
			echo "File $$sdir/$$srcfile differs from reference:";\
			diff -b $$sdir/$$srcfile $$sfile ; \
		    fi ;\
		done ;\
	    fi ;\
	done

check-complete:
	@for file in test*.html ; do \
	@if test ! -s "$$file" ; then echo "make runtests first" ; exit 1; fi ;\
	@if test ! -s testref/$$file ; then \
	    echo "testref/$$file is missing!" ; fi ; done

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
	-rm -rf test[1-9] test1[0-9] test2[0-9]
	-rm -f test[1-9].html
	-rm -f latex.err *.hux img*.xbm img*.gif
	-rm -f up.gif previous.gif next.gif
	-rm -f test1[0-9].html test2[0-9].html test7a.html
	-rm -f test[0-9].htm test[1-2][0-9].htm
	-rm -f testf1.ps testf1.gif
	-rm -f inplace subfiles
	-rm -f *.ler *.aux *.out


Makefile: Makefile.in ../../../config.status
	(cd ../../.. && ./config.status --file=src/tohtml/testing/Makefile)

.PHONY: clean inplace subfiles compare compare2
