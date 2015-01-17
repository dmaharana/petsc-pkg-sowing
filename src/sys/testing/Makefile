ALL: all-redirect

SHELL       = /bin/sh

include ../../../Makerules
top_srcdir  = ../../..
srcdir      = .
VPATH=.:${srcdir}

INCLUDES = -I${top_srcdir}/include -I../../../include

rdconftest: rdconftest.o
	$(CC) -o rdconftest rdconftest.o ../../../lib/libsowing.a

all-redirect: testing

testing: rdconftest
	./rdconftest conf
clean:
	rm -f rdconftest *.o

doc_SOURCES =
manpages:

wwwpages:

source_names:
	@echo $(CSOURCES)

.PHONY: all-redirect testing manpages wwwpages source_names clean
