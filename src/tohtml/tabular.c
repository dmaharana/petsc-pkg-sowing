#include <stdio.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"

/*
    This file provides simple handling of tabular environments.

    There are two versions. The first, oldest version simply processes
    an entire tabular enviroment, attempting to build a single table 
    using a preformatted environment.  This was designed for HTML version 1,
    along with Microsoft RTF, neither of which provided any native support 
    for tables.

    In the second version (under construction), we allow the use of
    HTML table tags.  HTML tables aren't as flexible as TeX tables, so this
    may not always do what you want.  But for many tables, it is superior to 
    using GIF images created from the TeX.

    The algorithm (designed for LaTeX) is:

    Read tabular; get l, r, c, |, or p commands.  Generate a description of 
    the row (struct HTabRow { int ncols; align_t *align ; }), and add 
    the tabular environment to the environment stack.  The current column 
    number is set to 0 (0-origin indexing).

    Each alignment character (& by default) finishes the previous column
    and begins the next; the corresponding HTabRow is read to generate the
    relevant HTML.  A \cr or \\ command finishes a column and row, and begins 
    the next row.  An \end{tabular} ends finishes a column and row, and 
    pops the tabular environment.
 */
/* extern int (*SCSetTranslate ())(); */

#define MAX_CELLS 20
typedef enum { TAB_LEFT, TAB_RIGHT, TAB_CENTER, TAB_PARAGRAPH, TAB_VBAR } 
        align_t;

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

typedef struct {
    int ncols;
    int curcol;
    align_t coltype[MAX_CELLS];
} HTabularRow;

static char curcell[MAX_TOKEN];
static char ltoken[MAX_TOKEN];
static int DebugTab = 0;

/*
  Get the {...} argument, look for l, r, c, p{...} commands
  Get lines, looking for & (separate cols) and \\ (separate rows)
  */
static int Ncell = 0;

void TeXNewAlignCol( void );
void TeXPutAlign( void );
void TeXEndHalignRow( void );

void TeXGetTabularDefn( void )
{
    int         ncell;
    char        *p;
    HTabularRow *hrow;

    hrow = (HTabularRow *)MALLOC( sizeof(HTabularRow) );  CHKPTR(hrow);

    ncell = 0;
    TeXGetArg( fpin[curfile], ltoken, MAX_TOKEN );
    p = ltoken;
    while (*p) {
	switch (*p) {
	case 'l': hrow->coltype[ncell++] = TAB_LEFT; break;
	case 'r': hrow->coltype[ncell++] = TAB_RIGHT; break;
	case 'c': hrow->coltype[ncell++] = TAB_CENTER; break;
	case 'p': hrow->coltype[ncell++] = TAB_PARAGRAPH; 
	    /* Skip size argument for now for paragraph */
	    if (p[1] == '{') {
		p++;
		while (*p && *p != '}') p++;
	    }
	    break;
	case '|': break;   /* Ignore borders for now */
	default:
	    ;
	}
	p++;
    }

    if (lstack[lSp].env == TXTABULAR) {
	lstack[lSp].extra_data = hrow;
	hrow->curcol = 0;
	hrow->ncols  = ncell;
    }
    else {
	Ncell = ncell;
	FREE( hrow );
    }
}

/*
  At \end{tabular}, format as "preformatted", using the colwid and coltype
  to format each cell.

  Use code similar to "skipenv", since we need to process commands that
  we may see.
 */    
void TeXtabular( TeXEntry *e )
{
    int  i, j, nsp, ch, lastInArg;
    FILE *fout = fpout;
    int  ncell;
    TabularRow *row, *next;
    char *p;
    int  (*savef) ( char *, int );

    lastInArg = InArg;
    InArg     = 1;
    
    TeXGetTabularDefn();
    /* Horrible temp hack */
    ncell = Ncell;

    Table.ncols = ncell;
    Table.Rows  = 0;
    for (i=0; i<ncell; i++) {
	Table.colwid[i] = 0;
    }

/* Needs to change for non-html output */
    savef = SCSetTranslate( (int (*)( char *, int ))0 ); 
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

/* HTML Versions */
void TeXNewAlignCol( void )
{
    HTabularRow *hrow;
    char buf[256], *align_str;

    if (lstack[lSp].env == TXTABULAR) {
	hrow = (HTabularRow *)(lstack[lSp].extra_data);
	switch (hrow->coltype[hrow->curcol]) {
	case TAB_LEFT: align_str = "\"LEFT\""; break;
	case TAB_RIGHT: align_str = "\"RIGHT\""; break;
	default: align_str = "\"CENTER\""; break;
	}
	sprintf( buf, "<TD ALIGN=%s>", align_str );
	TeXoutcmd( fpout, buf );
	hrow->curcol++;
    }
}

void TeXPutAlign( void )
{
    HTabularRow *hrow;

    if (lstack[lSp].env == TXTABULAR) {
	hrow = (HTabularRow *)(lstack[lSp].extra_data);
	TeXoutcmd( fpout, "</TD>" );
	/* Only generate this if the next command is *not* multicolumn */
	TeXNewAlignCol( );
	hrow->curcol++;
    }
}

void TeXEndHalignRow( void )
{
    HTabularRow *hrow;

    if (lstack[lSp].env == TXTABULAR) {
	hrow = (HTabularRow *)(lstack[lSp].extra_data);
	TeXoutcmd( fpout, "</TD></TR>\n<TR>" );
	hrow->curcol = 0;
	TeXNewAlignCol( );
    }
}

void TeXBeginHalignTable( void )
{
    HTabularRow *hrow;
    hrow = (HTabularRow *)(lstack[lSp].extra_data);
    hrow->curcol = 0;
    TeXoutcmd( fpout, "<TABLE><TR>" );
    TeXNewAlignCol( );
}

void TeXEndHalignTable( void )
{
    TeXoutcmd( fpout, "</TD></TR></TABLE>\n" );
}

/* Must redefine \\ while in tabular */

/* We can handle multicol with the colspan="n" in the TD element */
/* The latex \multicolumn has this meaning:
      \multicolumn{#columns}{positioning}{text}
   where positioning is the ALIGN value: l, r, c.

   This isn't quite right, because we generate the <TD ALIGN=xxx> too early.
   
 */
void TXmulticolumn( TeXEntry *e )
{
    HTabularRow *hrow;
    char buf[256], *align_str;
    int colcount;
    char aligntype;

    if (lstack[lSp].env == TXTABULAR) {
	hrow = (HTabularRow *)(lstack[lSp].extra_data);
	PUSHCURTOK;
	if (TeXGetArg( fpin[curfile], curtok, MAX_TOKEN ) == -1) 
	    TeXAbort( "TXmulticolumn", e->name );
	/* Find column count */
	colcount = atoi( curtok );

	if (TeXGetArg( fpin[curfile], curtok, MAX_TOKEN ) == -1) 
	    TeXAbort( "TXmulticolumn", e->name );
	/* Find alignment type */
	switch (curtok[0]) {
	case 'l': align_str = "\"LEFT\"";   break;
	case 'r': align_str = "\"RIGHT\"";  break;
	case 'c': align_str = "\"CENTER\""; break;
	default:  
	  fprintf( ferr, "Unrecognized multcolumn alignment %c\n", curtok[0] );
	  align_str = "\"CENTER\""; break;
	}
	POPCURTOK;
	
	sprintf( buf, "<TD ALIGN=%s COLSPAN=%d>", align_str, colcount );
	TeXoutcmd( fpout, buf );
	hrow->curcol += colcount;
    }
}

void InitTabular( void )
{
    /* Don't uncomment this until multicolumn works correctly with the 
       other alignments */
    TXInsertName( TeXlist, "multicolumn", TXmulticolumn, 3, (void *)0 ); 
}
