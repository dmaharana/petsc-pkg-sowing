ALL: doctext/doctext.html doctext.ps
#    tohtml, bfort
ALLPS: doctext.ps
#    tohtml.ps bfort.ps
ALLWWW: doctext/doctext.html
#	tohtml/tohtml.html bfort/bfort.html
SHELL = /bin/sh
#LATEX = virtex \&latex209
#DEFAULTPATH=`kpsewhich -show-path=.tex`
#TEXPATH=/home/gropp/texhome/share/texmf/tex/latex209:$(DEFAULTPATH)
#TEXPATH=.:/home/gropp/texhome/share/texmf/tex/latex209
#TOHTML = tohtml
TOHTML = ../../src/tohtml/tohtml

.SUFFIXES: .html .ps .dvi .tex
# add -texindex $*.fn for large manuals
.tex.dvi:
	-/bin/rm -f $*.aux $*.blg $*.bbl $*.toc $*.lof $*.lot
	-latex $*
	-bibtex $*
	-latex $*
	-latex $*
#	- (TEXINPUTS=$(TEXPATH) ; export TEXINPUTS ; \
#		$(LATEX) $* )
#	-bibtex $*
#	- (TEXINPUTS=$(TEXPATH) ; export TEXINPUTS ; \
#		$(LATEX) $* )
#	- (TEXINPUTS=$(TEXPATH) ; export TEXINPUTS ; \
#		$(LATEX) $* )

.dvi.ps:
	dvips $*

tohtml.dvi: tohtml.tex tools.bib
doctext.dvi: doctext.tex swap.eps
bfort.dvi: bfort.tex tools.bib

swap.eps: swap.txt
	doctext -ext 3 swap.txt
	cat Swap.3 | ptroff -man -t > swap.eps
	doctext -latex -ext tex swap.txt

.tex.html:
	/bin/rm -f $*.aux
	${TOHTML} -default -mapref ../../demos/petsc.cit \
		-mapman ../../www/petsc.cit $*.tex
	${TOHTML} -default -mapref ../../demos/petsc.cit \
		-mapman ../../www/petsc.cit $*.tex
	/bin/mv latex.err $*.err

#
tohtml/tohtml.html: tohtml.tex
	/bin/rm -f tohtml.aux
	${TOHTML} -default -mapref ../../demos/petsc.cit \
		-mapman ../../www/petsc.cit tohtml.tex
	${TOHTML} -default -mapref ../../demos/petsc.cit \
		-mapman ../../www/petsc.cit tohtml.tex
	/bin/mv latex.err tohtml.err
#
doctext/doctext.html: doctext.tex
	/bin/rm -f doctext.aux
	${TOHTML} -default -mapref ../../demos/petsc.cit \
		-mapman ../../www/petsc.cit doctext.tex
	${TOHTML} -default -mapref ../../demos/petsc.cit \
		-mapman ../../www/petsc.cit doctext.tex
	/bin/mv latex.err doctext.err

clean:
	$(RM) *.fn *.aux *.blg *.toc *.lof *.lot *.dvi *.fns *.bbl *.log
compress:
	for file in *.ps ; gzip $$file ; done
