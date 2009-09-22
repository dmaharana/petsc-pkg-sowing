/*
   This file contains routines to process \bibliography commands

   We really do this by looking for

   rootname.bbl

   and then processing that.  This allows people to include the
   bibliography lines directly in the file, since we add them to the
   list.

   Note that since we check for hyperlinks before checking the aux file
   entries, we will always direct the reader to a hyperlink before the
   conventional entry.

   Bib entries are of the form:

   \begin{thebibliography}{arg}
   \bibitem{cite-name}
   line
   \newblock line
   \newblock line

   ...

   \end{thebibliography}
 */

#include <stdio.h>
#include <ctype.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"

typedef struct _Bibentry {
    LINK     *Self;
    char     *name;  /* These could be gotten from Self, but this is easier */
    char     *fname; /* Filename that contains the reference */
    char     number[4];
    } Bibentry;

static SRList *biblist = 0;

static char  bibtoken[MAX_TOKEN];

/* Process things like \hskip 2in and \leftskip 2em */

static int bibcount = 1;

void TXbibitem( TeXEntry *e )
{
    char buf[10], dest[20], fulldest[256];

    TeXGetArg( fpin[curfile], bibtoken, MAX_TOKEN );

    sprintf( buf, "[%d] ", bibcount );
    sprintf( dest, "-Bib%d",  bibcount );
    sprintf( fulldest, "%s#-Bib%d",  outfile, bibcount );
    WriteJumpDestination( fpout, dest, buf );
    WriteBibtoauxfile( bibcount, fulldest, bibtoken );
    bibcount ++;
}

/* This routine is called to insert a definition read from the hux file into
   a search list for citations.  level can be ignored; the format of the
   entries is
   buffer is "filename#-namenumber citationname"

   For example "foo.html#-Bib3 p4-book"
 */
void InsertBib( int level, char *buffer )
{
    int d;
    LINK *l;
    Bibentry *e;
    char *namep;
    char  *p;

    if (!biblist) biblist = SRCreate();

/* Find the citation name */
    namep = buffer;
    while (*namep && *namep != ' ') namep++;
    while (*namep && *namep == ' ') namep++;
/* Remove the trailing newline */
    d = strlen(namep);
    if (namep[d-1] == '\n') namep[d-1] = 0;

    l       = SRInsert( biblist, namep, 0, &d );
    if (!l) return;
    e = NEW(Bibentry);   CHKPTR(e);
    l->priv = (void *)e;
    namep[-1] = 0;
    e->fname = (char *)MALLOC( strlen( buffer ) + 1 );  CHKPTR(e->fname);
    strcpy( e->fname, buffer );
    p = namep - 2;
    while (*p && isdigit(*p)) p--;
    p++;
    strncpy( e->number, p, 4 );
}

int BibLookup( char *name, char **url, char **text )
{
    LINK     *l;
    Bibentry *e;
    int      dummy;

    l = SRLookup( biblist, name, (char *)0, &dummy );
    if (!l) return 0;

    e = (Bibentry *)(l->priv);
    *url  = e->fname;
    *text = e->number;
    return 1;
}


void TXDoBibliography( TeXEntry *e )
{
    char fname[270];

    /* Eat the name of the database */
    TeXGetArg( fpin[curfile], fname, 270 );

    /* Get the name of the input file to use in searching for the bbl file */
    GetMainInputFileName( fname );
    RemoveExtension( fname );
    strcat( fname, ".bbl" );

    if (fpin[curfile+1]) fclose( fpin[curfile+1] );
    fpin[++curfile] = fopen( fname, "r" );
    if (!fpin[curfile]) {
	curfile--;
	fprintf( stderr, "(TXDoBibliography) Could not open file %s\n", fname );
	return;
    }
    InFName[curfile] = (char *)MALLOC( strlen(fname) + 1 );
    CHKPTR(InFName[curfile]);
    strcpy( InFName[curfile], fname );
    LineNo[curfile] = 0;
}
