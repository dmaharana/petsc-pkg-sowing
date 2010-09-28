ALL: runtests
SHELL = /bin/sh

FILES =test.nam tstpgm.c tstpgma.c tstpgm.h allfmt.c
runtests: /home/gropp/bin/sun4/doctext
	/bin/rm -f *.3 *.2 *.tex *.html
	/home/gropp/bin/sun4/doctext -mpath testref $(FILES)
	/home/gropp/bin/sun4/doctext -mpath testref -html $(FILES)
	/home/gropp/bin/sun4/doctext -mpath testref -ext tex -latex $(FILES)
	/home/gropp/bin/sun4/doctext -mpath testref -I pubinc -index f1.cit -quotefmt -heading MTEST \
		-ext 2 $(FILES)
	/home/gropp/bin/sun4/doctext -mpath testref -I pubinc -outfile f.tex -quotefmt -latex -heading MTEST \
		$(FILES)
	/home/gropp/bin/sun4/doctext -mpath testref -I pubinc -outfile f.html -quotefmt -html -heading MTEST \
		tst.cit $(FILES)
