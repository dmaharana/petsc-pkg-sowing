# Generated automatically from Makefile.in by configure.
ALL: install.ps tohtml.ps bfort.ps doctext.ps

install.ps: install.tex
	latex install
	dvips install
tohtml.ps: tohtml.tex
	latex tohtml
	dvips tohtml
bfort.ps: bfort.tex
	latex bfort
	dvips bfort
doctext.ps: doctext/doctext.tex
	(cd doctext ; latex doctext ; dvips doctext ; mv doctext.ps .. )
