/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * This file implements a simple if statement, using the LaTeX
 * \newif\ifname, \nametrue, \namefalse
 */

#include "sowing.h"
#include "tex.h"

static char iftoken[MAX_TOKEN];

void TXDoIf( TeXEntry * );
void TXDoIfFalse( TeXEntry * );
void TXDoIfTrue( TeXEntry * );
void TeXskipUntil( char *([]), int );

/* 
 *   Read a newif and define the name
 */
void TXNewif( TeXEntry *e )
{
    int ch, nsp;
    int *def;

    ch = TeXReadToken( iftoken, &nsp );
    if (ch == EOF) return;

    def = NEW(int); CHKPTR(def);
    *def = 1;
    TXInsertName( TeXlist, iftoken + 1, TXDoIf, 0, (void *)def );
    strcat( iftoken, "false" );
    TXInsertName( TeXlist, iftoken + 3, TXDoIfFalse, 0, (void *)def );
    iftoken[strlen(iftoken)-5] = '\0';
    strcat( iftoken, "true" );
    TXInsertName( TeXlist, iftoken + 3, TXDoIfTrue, 0, (void *)def );
}

/* A stack of if values.  For now, we can only handle true values */
#define MAX_IF_STACK 30
static int if_sp = -1;
static int truth[MAX_IF_STACK];

void TXDoIf( TeXEntry *e )
{
    int *def = (int *)(e->ctx);
    char *(names[2]);
    names[0] = "else";
    names[1] = "fi";
    if (*def) {
	/* True branch */
	/* Tell \else or \if to be silent */
	if (if_sp >= MAX_IF_STACK-1) 
	    fprintf( stderr, "Too many TeX ifs\n" );
	truth[++if_sp] = 1;
    }
    else {
	/* False branch */
	truth[++if_sp] = 0;
	/* Skip until we find a \else or \fi */
	TeXskipUntil( names, 2 );
    }
}

/* Set the if name to false */
void TXDoIfFalse( TeXEntry *e )
{
    int *def = (int *)(e->ctx);
    *def = 0;
}

/* Set the if name to true */
void TXDoIfTrue( TeXEntry *e )
{
    int *def = (int *)(e->ctx);
    *def = 1;
}

void TXElse( TeXEntry *e )
{
    char *(names[1]);
    /* If in the true branch, then skip, else ignore */
    if (truth[if_sp]) {
	/* Skip and ignore until \fi */
	names[0] = "fi";
	TeXskipUntil( names, 1 );
    }
}

void TXFi( TeXEntry *e )
{
    if (if_sp > 0)
	if_sp--;
}

void TeXskipUntil( char *(names[]), int n_names )
{
    int nsp, ch;

    PUSHCURTOK;
    while (1) {
	while ( (ch = TeXReadToken( curtok, &nsp )) == EOF) {
	    if (DebugCommands) 
		fprintf( stdout, "EOF in TeXskipUntil\n" );
	    TXPopFile();
	}
	if (ch == EOF) break;
	if (DebugCommands) {
	    fprintf( stdout, "Token is %s\n", curtok );
	}
	if (ch == CommentChar) {
	    /* Handle comments */
	    SCTxtDiscardToEndOfLine( fpin[curfile] );
	}
	else if (ch == CommandChar) {
	    int i;
	    for (i=0; i<n_names; i++) {
		if (strcmp( curtok+1, names[i] ) == 0) {
		    if (DebugCommands)
			fprintf( stdout, "Found terminator %s\n", names[i] );
		    POPCURTOK;
		    return;
		}
	    }
	}
    }
}

/* Add an easy way to process if commands that are defined in other packages 
   TXAddPredefIf( ifname, istrue )
 */
void TXAddPredefIf( const char *ifname, int isTrue )
{
    int *def;

    def = NEW(int); CHKPTR(def);
    *def = isTrue;
    TXInsertName( TeXlist, ifname, TXDoIf, 0, (void *)def );
}
