ALL: all-redirect
SHELL = /bin/sh
BFORT = ../bfort

TESTCOUTPUT = n1f.c n2f.c tstpgmaf.c tstpgma2f.c tstpgmbf.c \
	tstcf.c tst2cf.c tst3cf.c
TESTF90OUTPUT = tstpgmaf90.f90 tstpgmbf90.f90
TESTOUTPUT = $(TESTCOUTPUT) $(TESTF90OUTPUT)

SOURCES = n1.c n2.c tstpgma.c tstpgma2.c tstpgmb.c tstc.c pubinc malform.c
DIST_COMMON = Makefile.in
DISTSUBDIRS = testref

all-redirect: $(TESTOUTPUT)

# Note that we set the BFORT_CONFIG_PATH to permit running the tests before
# installing the program

n1f.c: n1.c $(BFORT)
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -ferr -mnative -anyname -I pubinc -mpi n1.c

n2f.c: n2.c $(BFORT)
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -ferr -mapptr -mnative -anyname -I pubinc -mpi n2.c

# These are tests that include string handling
tst2c.c: tstc.c
	ln -s tstc.c tst2c.c
tst3c.c: tstc.c
	ln -s tstc.c tst3c.c
tstcf.c: tstc.c $(BFORT)
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -ferr -anyname tstc.c
tst2cf.c: tst2c.c $(BFORT)
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -ferr -fnostring -anyname tst2c.c
tst3cf.c: tst3c.c $(BFORT)
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname tst3c.c


tstpgmaf90.f90 tstpgmaf.c: tstpgma.c $(BFORT)
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -mnative -nomsgs -anyname -mapptr -mpi -ferr \
	         -ptrprefix Petsc -ptr64 HAVE_64BIGS -fcaps HAVE_FORTRAN_CAPS \
		 -fuscore HAVE_FORTRAN_UNDERSCORE \
		 -f90modfile tstpgmaf90.f90 tstpgma.c

tstpgmbf90.f90 tstpgmbf.c: tstpgmb.c  $(BFORT)
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -mnative -nomsgs -anyname -mapptr -mpi -ferr \
	         -ptrprefix Petsc -ptr64 HAVE_64BIGS -fcaps HAVE_FORTRAN_CAPS \
		 -fuscore HAVE_FORTRAN_UNDERSCORE -shortargname \
		 -f90modfile tstpgmbf90.f90 tstpgmb.c

tstpgma2f.c: tstpgma2.c $(BFORT)
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -mnative -nomsgs -noprofile -anyname -mapptr -mpi \
		 -ferr -ptrprefix Petsc \
		 -ptr64 PETSC_USE_POINTER_CONVERSION \
		 -fcaps PETSC_HAVE_FORTRAN_CAPS \
		 -fuscore PETSC_HAVE_FORTRAN_UNDERSCORE \
		 -f90mod_skip_header tstpgma2.c

testing: $(TESTOUTPUT)
	-@for file in $(TESTCOUTPUT) ; do \
	   file=`basename $$file .c` ; \
	   if diff -b $$file.c testref/$${file}.c 2>&1 >/dev/null ; then \
		: ; \
	   else \
		echo "File $$file is different" ; \
		diff -b  $$file.c testref/$${file}.c ; \
	   fi ; \
	done
	-@for file in $(TESTF90OUTPUT) ; do \
	    file=`basename $$file .f90` ; \
	    if diff -b $$file.f90 testref/$${file}.f90 2>&1 >/dev/null ; then \
		: ; \
	    else \
		echo "File $$file is different" ; \
		diff -b $$file.f90 testref/$${file}.f90 ; \
	    fi ; \
	done

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
	rm -f $(TESTOUTPUT) config.log

Makefile: Makefile.in ../../../config.status
	(export CONFIG_FILES=src/bfort/testing/Makefile && \
	cd ../../.. && ./config.status)

../bfort: ../bfort.c
	(cd .. && $(MAKE) bfort)
#
# malform.c is used to check that bfort can handle a malformed comment
