/*
   This file contains routines to process TeX dimension commands
 */

#include <stdio.h>
#include <ctype.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"


static char  dimentoken[MAX_TOKEN];

/* Process things like \hskip 2in and \leftskip 2em 
 * Note that dimensions can be r-values as well as l-values, so 
 * try to catch those.
 */

/* Read a TeX dimension.  Return 1 on success, 0 on failure. */
int TXReadDimen( FILE *fin )
{
    int ch, nsp;
    char *p;

    do {
	ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
    } while (ch == '\n');
    if (ch == EOF) return 1;
    if (ch == '-') {
	ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
	if (ch == EOF) return 1;
    }
    if (ch == CommandChar) {
	/* Read macro name and accept that */
	TeXReadMacroName( dimentoken );
	return 0;
    }
/* Should be number; skip.  Note that it could contain a decimal point.
   This is rough; it skips decimal points and numbers until it finds something
   that might be a legal dimension name */
    while (1) {
	if (ch != '.' && !isdigit(ch)) break;
	ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
	if (ch == EOF) return 1;
    }
/* All dimensions can contain a "true" prefix, which in TeX terms means
   to NOT apply any magnification.  If you have to ask, you don't want to
   know. */
    p = dimentoken;
    if (strncmp( p, "true", 4 ) == 0) p += 4;
    if (!(strcmp( p, "in" ) == 0 ||
	  strcmp( p, "pt" ) == 0 ||
	  strcmp( p, "pc" ) == 0 ||
	  strcmp( p, "cm" ) == 0 || 
	  strcmp( p, "em" ) == 0 ||
	  strcmp( p, "ex" ) == 0 ||
	  strcmp( dimentoken, "fil" ) == 0 ||
	  strcmp( dimentoken, "fill" ) == 0 ||
	  strcmp( dimentoken, "filll" ) == 0)) {
	fprintf( ferr, 
	    "Unrecognized dimension %s at %s line %d (might be r-value)\n", 
		 dimentoken,
		 InFName[curfile] ? InFName[curfile] : "", LineNo[curfile] );
	/* Might be an r-value; push the token back */
	SCPushToken( dimentoken );
	return 1;
    }
    return 0;
}

void TXdimen( TeXEntry *e )
{
    int ch, nsp;

    ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
    if (ch == EOF) return;
    if (!strcmp( dimentoken, "=" ) == 0) {
	SCPushToken( dimentoken );
    }
    if (TXReadDimen( fpin[curfile] )) return;

    ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
    if (ch == EOF) return;

    if (!(strcmp( dimentoken, "plus" ) == 0 || 
	  strcmp( dimentoken, "minus" ) == 0)) {
	SCPushToken( dimentoken );
	return;
    }
    if (TXReadDimen( fpin[curfile] )) return;

    ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
    if (ch == EOF) return;

/* Do it again, in case there is a plus and a minus */
    if (!(strcmp( dimentoken, "plus" ) == 0 || 
	  strcmp( dimentoken, "minus" ) == 0)) {
	SCPushToken( dimentoken );
	return;
    }
    if (TXReadDimen( fpin[curfile] )) return;

    return;
}

/* Process things like \penalty 1000 */
void TXnumber( TeXEntry *e )
{
    int ch, nsp;

    ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
    if (ch == EOF) return;
}

/* Process things like \hangafter=1 */
void TXcounter( TeXEntry *e )
{
    int ch, nsp;
    ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
    if (ch == EOF) return;
    if (ch == '=') {
	ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
    }
}

/*
   Handle \advance\name [by] dimen
 */
void TXadvance( TeXEntry *e )
{
    int ch, nsp;

    ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
    if (ch == EOF) return;
    if (ch == '\\') {
	TeXReadMacroName( dimentoken );
	ch = SCTxtFindNextANToken( fpin[curfile], dimentoken, MAX_TOKEN, &nsp );
	if (strcmp( dimentoken, "by" ) != 0) 
	    SCPushToken( dimentoken );
	TXReadDimen( fpin[curfile] );
    }
}
