#include <stdio.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"

/*
    This file provides simple handling of tabular environments
 */
/* extern int (*SCSetTranslate ANSI_ARGS(()))ANSI_ARGS(()); */

#define MAX_CELLS 20
typedef struct _TabularRow {
    struct _TabularRow *next;	
    char               **cell;
    } TabularRow;
    
struct {
    int ncols,                /* Number of columns */
        nrows,                /* Number of rows */
        coltype[MAX_CELLS],   /* Type of each column */
        colwid[MAX_CELLS];    /* Width of each column in characters */
    TabularRow *Rows;
    } Table;

static char curcell[MAX_TOKEN];
static char ltoken[MAX_TOKEN];
static int DebugTab = 0;

/*
  Get the {...} argument, look for l, r, c, p{...} commands
  Get lines, looking for & (separate cols) and \\ (separate rows)

  At \end{tabular}, format as "preformatted", using the colwid and coltype
  to format each cell.

  Use code similar to "skipenv", since we need to process commands that
  we may see.
 */    
void TeXtabular( e )
TeXEntry *e;
{
    int  i, j, nsp, ch, lastInArg;
    FILE *fout = fpout;
    int  ncell;
    TabularRow *row, *next;
    char *p;
    int  (*savef) ANSI_ARGS(( char *, int ));

    lastInArg = InArg;
    InArg     = 1;
    ncell = 0;
    TeXGetArg( fpin[curfile], ltoken, MAX_TOKEN );
    p = ltoken;
    while (*p) {
	if (*p == '{') while (*p && *p != '}') p++;
	else if (*p == 'l') ncell++;
	else if (*p == 'r') ncell++;
	else if (*p == 'c') ncell++;
	else if (*p == 'p') ncell++;
	p++;
    }
    Table.ncols = ncell;
    Table.Rows  = 0;
    for (i=0; i<ncell; i++) {
	Table.colwid[i] = 0;
    }

/* Needs to change for non-html output */
    savef = SCSetTranslate( (int (*)ANSI_ARGS(( char *, int )))0 ); 
                            /*  SCHTMLTranslateTables ); */

    p     = curcell;
    ncell = 0;
    curcell[0] = 0;
    row   = NEW(TabularRow);   CHKPTR(row);
    row->cell = (char **)MALLOC( Table.ncols * sizeof(char *) );
    row->next = 0;
    for (i=0; i<Table.ncols; i++) row->cell[i] = 0;
    while (1) {
	while ( (ch = 
		 SCTxtFindNextANToken( fpin[curfile], ltoken, MAX_TOKEN, &nsp )) == EOF)
	    TXPopFile();
	if (ch == -1) break;
	if (DebugTab) printf( "Tabular read %s\n", ltoken );
	/* Remove leading blanks */
	if (p == curcell) nsp = 0;
	if (ltoken[0] == '&') {
	    /* End of a cell */
	    if (ncell >= Table.ncols) {
		fprintf( ferr, "Too many cells in table\n" );
		break;
	    }
	    row->cell[ncell] = (char *)MALLOC( strlen(curcell) + 1 );
	    strcpy( row->cell[ncell], curcell );
	    if ((int)strlen(curcell) > Table.colwid[ncell]) 
		Table.colwid[ncell] = strlen(curcell);
	    ncell++;
	    curcell[0] = 0;
	    p          = curcell;
        }
	else if (ltoken[0] == '\\') {
	    /* TeX command */
	    for (i=0; i<nsp; i++) *p++ = ' ';
	    while ( (ch = 
		     SCTxtFindNextANToken( fpin[curfile], ltoken, MAX_TOKEN, &nsp )) 
		    == EOF) 
		TXPopFile();
	    if (nsp > 0) {
		/* Command was really \ ltoken */
		*p++ = ' ';
		i    = 0;
		while (ltoken[i]) { *p++ = ltoken[i]; i++; }
		*p = 0;
	    }
	    else if (strcmp( ltoken, "end" ) == 0) {
		TeXGetArg( fpin[curfile], ltoken, MAX_TOKEN );
		if (strcmp( ltoken, "tabular" ) != 0) {
		    fprintf( ferr, 
			     "%s does not match tabular\n", ltoken );
	        }
		else
		    break;
	    }
	    else if (ltoken[0] == '\\') {
		/* End of line.  First, do end-of-cell processing */
		if (ncell >= Table.ncols) {
		    fprintf( ferr, "Too many cells in table\n" );
		    break;
		}
		if (ncell + 1 != Table.ncols) {
		    fprintf( ferr, "Too few cells in table\n" );
		}
		/* Get last cell ... */
		row->cell[ncell] = (char *)MALLOC( strlen(curcell) + 1 );
		*p = 0;
		strcpy( row->cell[ncell], curcell );
		if ((int)strlen(curcell) > Table.colwid[ncell]) 
		    Table.colwid[ncell] = strlen(curcell);
		ncell = 0;
		p     = curcell;
		curcell[0] = 0;
		/* Now, create a new row */
		next       = NEW(TabularRow);
		next->cell = (char **)MALLOC( Table.ncols * sizeof(char *) );
		next->next = 0;
		for (i=0; i<Table.ncols; i++) next->cell[i] = 0;
		if (!Table.Rows) Table.Rows = row;
		row->next = next;
		row  = next;
	    }
	    else {
		/* Need to process TeX command */
		TeXProcessCommand( ltoken, fpin[curfile], fout );
	    }
        }
	else {
	    for (i=0; i<nsp; i++) *p++ = ' ';
	    *p = 0;
	    if (ch == '\n') {
		LineNo[curfile]++;
	    }
	    else if (ch == '{')
		TXbgroup( e );
	    else if (ch == '}')
		TXegroup( e );
	    else { 
		i    = 0;
		while (ltoken[i]) *p++ = ltoken[i++];
		*p = 0;
	    }
        }
    }
    SCSetTranslate( savef );

/* Now we can dump the rows... */
    InArg = lastInArg;
    TXpreformated( fout, 1 );
    row   = Table.Rows;
    while (row) {
	for (i=0; i<Table.ncols; i++) {
	    if (row->cell[i]) 
		WriteString( fout, row->cell[i] );
	    /* By using = in the length test, we get an extra space */
	    for (j=row->cell[i]?strlen(row->cell[i]):0; j<=Table.colwid[i]; j++) 
		fputc( ' ', fout );
	    if (row->cell[i]) {
		FREE( row->cell[i] );
	    }
	}
	fputc( '\n', fout );
	next = row->next;
	FREE( row->cell );
	FREE( row );
	row = next;
    }
    TXpreformated( fout, 0 );
}

/* We should really modify the output routine to handle the cases of
   (a) c, r, l type
   (b) p type
 */
