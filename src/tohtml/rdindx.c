#include <stdio.h>
#include "sowing.h"
#include "tex.h"
#include "search.h"

/* 
    This file contains code for reading an index file and generating the index

    The format is
        name sectionname-that-contains
 */

typedef struct _Index {
    struct _Index *next, *prev;
    char     *name; 
    char     *containing_section;
    char     *fname;                 /* file name containing section */
    int      number;
    } Index;

static Index *fidx = 0, *fidxtail = 0, *cidx = 0, *cidxtail = 0,
             *idx  = 0, *idxtail = 0;
extern void SortIndex ( Index * );

void AddToIndex( name, section, sec_number, which )
char *name, *section;
int  sec_number, which;
{
    Index *new;

    new = NEW(Index);                                 CHKPTR(new);
    new->name = (char *)MALLOC( strlen(name) + 1 );   CHKPTR(new->name);
    new->containing_section = (char *)MALLOC( strlen(section) + 1 );
    CHKPTR(new->containing_section);
    strcpy( new->name, name );
    strcpy( new->containing_section, section );
    new->number = sec_number;
    new->next = 0;
    new->prev = 0;
    if (outfile) {
	new->fname = (char *)MALLOC( strlen(outfile) + 1 );  CHKPTR(new->fname);
	strcpy( new->fname, outfile );
    }
    else {
	new->fname = 0;
    }
    switch (which) {
    case 0: 
	if (fidxtail == 0) {
	    fidx = fidxtail = new;
	}
	else {
	    fidxtail->next = new;
	    new->prev      = fidxtail;
	    fidxtail       = new;
	}
	break;
    case 1: 
	if (cidxtail == 0) {
	    cidx = cidxtail = new;
	}
	else {
	    cidxtail->next = new;
	    new->prev      = cidxtail;
	    cidxtail       = new;
	}
	break;
    case 2: 
	if (idxtail == 0) {
	    idx = idxtail = new;
	}
	else {
	    idxtail->next = new;
	    new->prev      = idxtail;
	    idxtail       = new;
	}
	break;
    }
}

/* A simple bubble sort of the index */
void SortIndex( r )
Index *r;
{
    Index *r1;
    int   itmp;
    char  *ctmp;
    int   cmp;

    while (r) {
	r1 = r->next;
	while (r1) {
	    cmp = strcmp( r1->name, r->name );
	    if (cmp < 0 || (cmp == 0 && r1->number < r->number)) {
		/* Swap the contents */
		ctmp = r->name; r->name = r1->name; r1->name = ctmp;
		ctmp = r->containing_section; r->containing_section =
						  r1->containing_section; r1->containing_section = ctmp;
						  ctmp = r->fname; r->fname = r1->fname; r1->fname = ctmp;
						  itmp = r->number; r->number = r1->number; r1->number = itmp;
	    }
	    r1 = r1->next;
	}
	r  = r->next;
    }
}

/*
 * Removes empty entries and duplicates as well.
 */
void WriteIndex( fout, which )
FILE *fout;
int  which;
{
    Index *r;
    char  lfname[256];
    char *lastname = 0;
    int  lastnumber = 0;

    switch (which) {
    case 0: r = fidx; break;
    case 1: r = cidx; break;
    case 2: r =  idx; break;
    }
    SortIndex( r );
    TXbmenu( fout );
    while (r) {
	/* This had strlen(r->name) >= 0, which is ALWAYS true */
	if (r->name != 0 && strlen(r->name) > 0) {
	    if (lastname == 0 || 
		(r->name != 0 && strcmp( lastname, r->name ) != 0 &&
		 r->number != lastnumber)) {
		WriteBeginPointerMenu( fout );
		sprintf( lfname, "%s#Node", r->fname ? r->fname : "" );
		WritePointerText( fout, r->name, lfname, r->number );
		WriteEndOfPointer( fout );
		lastname   = r->name;
		lastnumber = r->number;
	    }
	}
	r = r->next;
    }
    TXemenu( fout );
}

