# Generated automatically from Makefile.in by configure.
ALL: install.ps tohtml.ps bfort.ps doctext.ps
SHELL = /bin/sh
TOHTML = ../src/tohtml/tohtml
prefix = /nfs/mcs-homes05/gropp/sowing-proj/sowing
wwwdir = /homes/gropp/public_html/sowing

doc_POSTSCRIPT = install.ps tohtml.ps bfort.ps doctext.ps
doc_HTML = install/install.htm tohtml/tohtml.htm bfort/bfort.htm \
	doctext/doctext/doctext.htm

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

install/install.htm: install.tex config.txt
	${TOHTML} -default -dosnl install.tex
	${TOHTML} -default -dosnl install.tex
tohtml/tohtml.htm: tohtml.tex
	${TOHTML} -default -dosnl tohtml.tex
	${TOHTML} -default -dosnl tohtml.tex
bfort/bfort.htm: bfort.tex
	${TOHTML} -default -dosnl bfort.tex
	${TOHTML} -default -dosnl bfort.tex
doctext/doctext/doctext.htm: doctext/doctext.tex
	(cd doctext ; ${MAKE} doctext/doctext.htm )
config.txt: ../configure
	../configure --help >config.txt
clean:
	rm -f bfort.ps tohtml.ps install.ps doctext.ps \
		*.aux *.dvi *.toc *.log *.fn

install-web: ${doc_HTML}
	cp -rp install ${wwwdir}/sowinginstall
	cp -rp bfort ${wwwdir}/bfort
	cp -rp tohtml ${wwwdir}/tohtml
	cp -rp doctext/doctext ${wwwdir}/doctext
install-postscript: ${doc_POSTSCRIPT}
	cp -p install.ps ${wwwdir}/sowinginstall.ps
	cp -p bfort.ps ${wwwdir}/bfort.ps
	cp -p tohtml.ps ${wwwdir}/tohtml.ps
	cp -p doctext.ps ${wwwdir}doctext.ps

install: install-web install-postscript
