ALL: all-redirect
SHELL = /bin/sh
BFORT = ../bfort

TESTFILES  = tbase.c tchar.c tmpi.c tret.c tpetsc.c thandle.c thead.h

TESTCOUTPUT = tbasef.c tbasef2.c tbasef3.c tbasef4.c \
	tcharf.c tcharf2.c tmpif.c tretf.c tpetscf.c thandlef.c theadhf.c \
	tmp/tbasef.c tmp/tbasef2.c
TESTF90OUTPUT = tpetsc.f90 thandle.f90
OTHEROUTPUT = malformf.c tbaserm.c
# Permit easy testing of both the old and new version of bfort
# (The new version fixes some bugs and has a different and more consise way
# of handling the -anyname option)
ifeq "$(BFORTVERSION)" "new"
TESTDIR = testref
else
TESTDIR = testref-old
endif

SOURCES = $(TESTFILES) malform.c tcharinc tmpiinc
DIST_COMMON = Makefile.in
DISTSUBDIRS = testref testref-old

all-redirect: Makefile $(TESTCOUTPUT) $(TESTF90OUTPUT)

tbasef.c: tbase.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname tbase.c

tbasef2.c: tbase.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname -ferr -o tbasef2.c tbase.c

tbasef3.c: tbase.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -shortargname -ferr -o tbasef3.c tbase.c

tbasef4.c: tbase.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname -fcaps USECAPS -fuscore USEUSCORE \
		-fduscore USEDOUBLEUSCORE -ferr -o tbasef4.c tbase.c

# Test the -dir and -dir -o options.  Note that the output file *must be
# the same* as the cases without the -dir option
tmp/tbasef.c: tbase.c
	@if [ ! -d tmp ] ; then mkdir tmp ; fi
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -dir tmp -anyname tbase.c
tmp/tbasef2.c: tbase.c
	@if [ ! -d tmp ] ; then mkdir tmp ; fi
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -dir tmp -anyname -ferr tbase.c -o tbasef2.c

# Test different options for handling character data
tcharf.c: tchar.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname -I tcharinc tchar.c

tcharf2.c: tchar.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname -ferr -I tcharinc -o tcharf2.c tchar.c

# Test MPI handles and objects
tmpif.c: tmpi.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname -mpi -I tmpiinc -ferr tmpi.c

# Test routines that do not return int.  Must *not* use -ferr with this
# file.
tretf.c: tret.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname tret.c

# Test routines defined in C as macros.
theadhf.c: thead.h
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname -ferr thead.h

# Test special features used in PETSc (now the most important user of bfort)
ifeq "$(BFORTVERSION)" "new"
PETSCNEWOPTS=-ferrfuncret "PETSC_EXTERN void PETSC_STDCALL" -usenativeptr
else
PETSCNEWOPTS=
endif

tpetscf.c tpetsc.f90: tpetsc.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname -mpi \
	    -mapptr -mnative -ptrprefix Petsc \
	    -ptr64 PETSC_USE_POINTER_CONVERSION \
	    -fcaps PETSC_HAVE_FORTRAN_CAPS \
	    -fuscore PETSC_HAVE_FORTRAN_UNDERSCORE \
	    -ferr $(PETSCNEWOPTS) \
	    -f90modfile tpetsc.f90 -shortargname \
	    -f90mod_skip_header \
	 tpetsc.c

# Test how miscellaneous handles are processed
ifeq "$(BFORTVERSION)" "new"
HANDLENEWOPTS=-f90usemodule handledef -usenativeptr
else
HANDLENEWOPTS=
endif
thandlef.c thandle.f90: thandle.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -anyname \
	    -mapptr -mnative -ptrprefix Petsc \
	    -ptr64 PETSC_USE_POINTER_CONVERSION \
	    -f90modfile thandle.f90 $(HANDLENEWOPTS) \
	 thandle.c

# These targets test other options; typically used for coverage test
# malform.c is used to check that bfort can handle a malformed comment
manualtests:
	-export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -version
	-export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -help
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) -shortargname -ferr -debug -o tbaserm.c tbase.c
	rm -f tbaserm.c
	export BFORT_CONFIG_PATH=../../../share && \
	$(BFORT) malform.c
	rm -f malformf.c

#
# Options to test (not included above).  These are used by PETSc
# petsc/maint/bin/generatefortranstubs.py for what PETSc needs (and what
# it currently works around)
# -nomsgs
#
# Options to ignore
# -ansi -noprofile
#

testing: $(TESTCOUTPUT) $(TESTF90OUTPUT)
	-@for origfile in $(TESTCOUTPUT) ; do \
	   file=`basename $$origfile .c` ; \
	   if diff -b $$origfile $(TESTDIR)/$${file}.c 2>&1 >/dev/null ; then \
		: ; \
	   else \
		echo "File $$origfile and $(TESTDIR)/$${file}.c are different" ; \
		diff -b  $$origfile $(TESTDIR)/$${file}.c ; \
	   fi ; \
	done
	-@for origfile in $(TESTF90OUTPUT) ; do \
	    file=`basename $$origfile .f90` ; \
	    if diff -b $$origfile $(TESTDIR)/$${file}.f90 2>&1 >/dev/null ; then \
		: ; \
	    else \
		echo "File $$origfile and $(TESTDIR)/$${file}.f90 are different" ; \
		diff -b $$origfile $(TESTDIR)/$${file}.f90 ; \
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
	rm -f $(TESTCOUTPUT) $(TESTF90OUTPUT) $(OTHEROUTPUT)
	rm -rf tmp

Makefile: Makefile.in ../../../config.status
	(export CONFIG_FILES=src/bfort/testing2/Makefile && \
	cd ../../.. && ./config.status)

../bfort: ../bfort2.c
	(cd .. && $(MAKE) bfort)

