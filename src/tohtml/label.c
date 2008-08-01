#include <stdio.h>
#include <ctype.h>
#include "tex.h"
#include "sowing.h"
#include "search.h"

/*
    This file contains routines for managing labels and references
 */
typedef struct {
    char *nodename;
    char *fname;
    int  seqnum;
    } LABEL;
    
SRList *labelctx = 0;


/* A label at node "seqnum", with buffer containing "name node-name".
   This is intended for use by the aux file.  The file containing
   the reference is "fname" */
void InsertLabel( int seqnum, char *buffer, char *fname )
{
    char *name, *p;
    static char entrylevel[] = "Document";
    int  dummy;
    LINK *l;
    LABEL *ll;

    if (!labelctx) labelctx = SRCreate();

    p    = buffer;
    while (isspace(*p)) p++;
    name = p;
    while (!isspace(*p)) p++;
    buffer = p+1;
    *p = 0;

/* 
   fprintf( stdout, "inserting |%s| into label list (node %s)\n", name, buffer );
   */
    l = SRInsert( labelctx, name, entrylevel, &dummy );
    ll = NEW(LABEL);
    l->priv = (void *)ll;
    ll->seqnum = seqnum;
    ll->nodename = MALLOC( strlen(buffer) + 1 );   CHKPTR(ll->nodename);
    strcpy( ll->nodename, buffer );
    ll->fname    = (char *)MALLOC( strlen(fname) + 1 );   CHKPTR(ll->fname);
    strcpy( ll->fname, fname );
}
	
char *LabelLookup( char *name, int *seqnum, char *fname )
{
    LINK *l;
    LABEL *ll;
    char  entryname[128];
    int   dummy;

    if (!labelctx) return 0;

    /* Set a default name */
    strcpy( entryname, "Document" );
    l = SRLookup( labelctx, name, entryname, &dummy );
    if (!l) return 0;
    ll = (LABEL *)l->priv;
    *seqnum = ll->seqnum;
    strcpy( fname, ll->fname );
    return ll->nodename;
}
	
/* Replace any tabs or newlines with a single blank */
void ReplaceWhite( char *str )
{
    while (*str) {
	if (isspace(*str)) *str = ' ';
	str++;
    }
}
