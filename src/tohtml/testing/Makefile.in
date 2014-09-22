ALL: inplace subfiles

TOHTML = ../tohtml


inplace: $(inplace_files) ../tohtml
subfiles: $(split_files)  ../tohtml

inplace_files = test1.html test2.html test3.html test4.html test6.html \
        test7.html test7a.html \
	test8.html test9.html test10.html test11.html test12.html test13.html \
	test14.html test15.html test16.html test17.html test18.html \
	test25.html test26.html test27.html test28.html
split_files = test1/test1.html test2/test2.html test3/test3.html \
	test4/test4.html test6/test6.html test7/test7.html test20/test20.html

# Inplace option
test1.html: test1.tex $(TOHTML)
	$(TOHTML) test1.tex
test2.html: test2.tex $(TOHTML)
	$(TOHTML) test2.tex
test3.html: test3.tex $(TOHTML)
	$(TOHTML) test3.tex
test4.html: test4.tex $(TOHTML)
	$(TOHTML) test4.tex
test6.html: test6.tex $(TOHTML)
	$(TOHTML) test6.tex
test7.html: test7.tex $(TOHTML)
	$(TOHTML) test7.tex
test8.html: test8.tex $(TOHTML)
	$(TOHTML) test8.tex
test9.html: test9.tex $(TOHTML)
	$(TOHTML) test9.tex
test10.html: test10.tex test10.dat $(TOHTML)
	$(TOHTML) -mapman test10.cit test10.tex
test11.html: test11.tex $(TOHTML)
	$(TOHTML) test11.tex
test12.html: test12.tex $(TOHTML)
	$(TOHTML) test12.tex
test13.html: test13.tex $(TOHTML)
	$(TOHTML) test13.tex
test14.html: test14.tex $(TOHTML)
	$(TOHTML) test14.tex
test15.html: test15.tex $(TOHTML)
	@/bin/rm -f foo.ps foo.gif ../foobar.eps ../foobar.gif
	@/bin/rm -f foo.ps foo.gif ../foobar.eps ../foobar.gif
	@echo "%! PS" > foo.ps
	@echo "%! PS" > ../foobar.eps
	@sleep 1
	@echo "dummy gif" > foo.gif
	@echo "dummy gif2" > ../foobar.gif
	$(TOHTML) test15.tex
	@if [ ! -s foobar.gif ] ; then echo "Failed to move foobar.gif" ; fi
	@/bin/rm -f foo.ps foo.gif ../foobar.eps ../foobar.gif foobar.gif
test16.html: test16.tex $(TOHTML)
	$(TOHTML) test16.tex
test17.html: test17.tex $(TOHTML)
	$(TOHTML) test17.tex
test18.html: test18.tex $(TOHTML)
	$(TOHTML) test18.tex
test25.html: test25.tex $(TOHTML)
	rm -f testf1.gif testf1.ps
	$(TOHTML) test25.tex
	if [ ! -s testf1.gif ] ; then \
	    echo "Did not create gif file" ; \
	fi
	rm -f testf1.gif testf1.ps
test26.html: test26.tex $(TOHTML)
	$(TOHTML) test26.tex
test27.html: test27.tex $(TOHTML)
	$(TOHTML) -cvtlatex test27.tex
test28.html: test28.tex $(TOHTML)
	$(TOHTML) test28.tex

# default
test1/test1.html: test1.tex $(TOHTML)
	$(TOHTML) -default test1.tex
	$(TOHTML) -default test1.tex
test2/test2.html: test2.tex $(TOHTML)
	$(TOHTML) -default test2.tex
	$(TOHTML) -default test2.tex
test3/test3.html: test3.tex $(TOHTML)
	$(TOHTML) -default test3.tex
	$(TOHTML) -default test3.tex
test4/test4.html: test4.tex $(TOHTML)
	$(TOHTML) -default test4.tex
	$(TOHTML) -default test4.tex
test6/test6.html: test6.tex $(TOHTML)
	$(TOHTML) -default test6.tex
	$(TOHTML) -default test6.tex
test7/test7.html: test7.tex $(TOHTML)
	$(TOHTML) -default test7.tex
	$(TOHTML) -default test7.tex
test11/test11.html: test11.tex $(TOHTML)
	$(TOHTML) -default test11.tex
	$(TOHTML) -default test11.tex
test12/test12.html: test12.tex $(TOHTML)
	$(TOHTML) -default test12.tex
	$(TOHTML) -default test12.tex
test7a.html: test7.tex $(TOHTML)
	rm -f test7a.html test7.html
	$(TOHTML) -default -htables -split -1 test7.tex -o test7a.html
test20/test20.html: test20.tex $(TOHTML)
	$(TOHTML) -cvtmath -split 1 test20.tex

# default with active token processing
test5.html: test5.tex test5.cit $(TOHTML)
	$(TOHTML) -mapref test5.cit -mapman test5m.cit test5.tex

testing: $(inplace_files) $(split_files) compare

compare:
	-@for file in testref/* ; do \
	if test $$file = "testref/CVS" ; then continue ; fi ; \
	if test -d $$file ; then \
	    for f in $$file/* ; do \
		if test $$f = "$$file/CVS" ; then continue ; fi ; \
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

compare2: $(inplace_files) $(split_files)
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
