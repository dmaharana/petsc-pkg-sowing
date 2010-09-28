#include <stdio.h>
#include <ctype.h>
#include "sowing.h"
#include "tex.h"

/*
    This file contains routines for re-scanning data.

    To accommodate special characters on input, a function, SCTranslate,
    is called ON ORIGINAL INPUT ONLY.

    For obscure reasons, the first version of this pushed back a single token;
    this allowed the code to strcpy the token in place.  This (will soon) be
    replaced with a more conventional (and correct) push-back mechanism that
    pushes back in inverse order.

    Another complication is that TeX doesn't REALLY have a comment character.
    It actually has an ACTIVE character that causes a flush to EOL.  This
    character is active almost everywhere, but not after a \ or in certain
    commands (eg, \verb or \code).  So we really don't want to have a 
    commentchar for TeX.
 */

#define MAX_LBUFFER 5000 
static char lbuffer[MAX_LBUFFER+2];
static int  lp  = -1;
static int  DebugScan = 0;
static char commentchar = 0;    /* Causes a flush to EOL if defined */
static int (*SCTranslate) ( char *, int ) = 0;
static int isatletter = 0;
/* #define OUTFILE fpout */
#define OUTFILE stdout

void OverlapCopy( char *, int );

void SCSetDebug( int flag )
{
    DebugScan     = flag;
}

int (*SCSetTranslate( int (*f)( char *, int ) ))( char *, int )
{
    int (*old) ( char *, int );

    old = SCTranslate;
    SCTranslate = f;
    return old;
}

/* 
   This is a TeX/LaTeX thing.  TeX allows completely arbitrary changes of 
   character types; for now, we only support changing the '@' character 
   because this is a common operation
 */
void SCSetAtLetter( int flag )
{
    isatletter = flag;
}

/* 
   We really want to use the same mechanism that TeX does; this
   requires having a translation table for characters to types
 */
static int chartype[256];

/* Initialize the chartype array */
/* Currently unused ?? */
/* Should change to use TeX's numbers */
#define TEX_LETTER 1
#define TEX_NUMBER 2

#define TEX_OTHER  12
#define TEX_ALIGN 4
#define TEX_ACTIVE 13
#define TEX_COMMENT 14
#define TEX_SUPERSCRIPT 7
#define TEX_SUBSCRIPT 8

void SCInitChartype( void )
{
    int i;
    unsigned char c;
    for (i=0; i<255; i++) {
	c = (unsigned char)i;
	if (isdigit(c))     chartype[i] = TEX_NUMBER;
	else if(isalpha(c)) chartype[i] = TEX_LETTER;
	else                chartype[i] = TEX_OTHER;
    }
/* May need to change */
    chartype[TOK_START] = TOK_START;
    chartype[TOK_END]   = TOK_END;
}

/* This is a version of SYTxtFindNextAToken that first looks in lbuffer */
/* Note that we have stricter rules.  Tokens are either ALL digits, 
   ALL letters, or single characters.

   Depending on the value of isatletter, the character '@' make be considered
   a letter 

   A second complication is that if an 'output' token is pushed back, we
   need to return it.  They are marked as TOK_START ... TOK_END .
  */
int SCTxtFindNextANToken( FILE *fp, char *token, int maxtoken, int *nsp )
{
    int fc, c, Nsp;
    char *tf = token;

    c = SCTxtGetChar( fp );
    if (c == EOF) return -1;

    Nsp = 0;
    while (isspace(c) && c != '\n') {
	Nsp++;
	c = SCTxtGetChar( fp );
	if (c == EOF) return c;
    }

/* Replace comment character with newline after flushing input; note that
   we NEVER push comment characters back onto the input, so this will ALWAYS
   read from the file */
    if (c == commentchar) {
	if (DebugCommands) fprintf( stdout, "Discarding to end of line\n" );
	SYTxtDiscardToEndOfLine( fp );
	LineNo[curfile]++;
	c = '\n';
    }

    *token++ = fc = c;
    maxtoken--;
    if (isalpha(c) || (isatletter && c == '@')) {
	c = SCTxtGetChar( fp );
	while(maxtoken && (isalpha(c) || (isatletter && c == '@'))) {
	    *token++ = c;
	    maxtoken--;
	    c = SCTxtGetChar( fp );
	    if (c == EOF) break;
	}
	SCPushChar( c );
    }
    else if (isdigit(c)) {
	c = SCTxtGetChar( fp );
	while(maxtoken && isdigit(c)) {
	    *token++ = c;
	    maxtoken--;
	    c = SCTxtGetChar( fp );
	    if (c == EOF) break;
	}
	SCPushChar( c );
    }
    else if (c == TOK_START) {
	c = SCTxtGetChar( fp );
	while (maxtoken && c != TOK_END) {
	    *token++ = c;
	    maxtoken--;
	    c = SCTxtGetChar( fp );
	    if (c == EOF) break;
	}
	/* Add the end-of-token */
	if (maxtoken) 
	    *token++ = c;
    }

    *token = 0;
    *nsp   = Nsp;
    if (c != EOF && SCTranslate)
	c = (*SCTranslate)( token, maxtoken );
    if (DebugScan) {
	fprintf( OUTFILE, "Returning |%s|\n", tf );
    }
    return fc;
}

/* Return the next character */
int SCTxtGetChar( FILE *fp )
{
    int fc, c;
    char token[10];

    if (lp == -1) {
	c = SYTxtGetChar( fp );
	if (c != EOF && SCTranslate) {
	    token[0] = c;
	    token[1] = 0;
	    c = SCTranslate( token, 10 );
	    /* If there are more characters, return them to the pile */
	    if (token[1]) 
		SCPushToken( token + 1 );
	}
	if (c == commentchar) {
	    if (DebugCommands) 
		fprintf( stdout, "Discarding to end of line\n" );
	    SYTxtDiscardToEndOfLine( fp );
	    c = '\n';
        }
	if (DebugScan) {
	    if (c == EOF) 
		fprintf( OUTFILE, "Returning EOF\n" );
	    else {
		if (isprint(c)) {
		    fprintf( OUTFILE, "Returning char %c\n", c );
		}
		else if (iscntrl(c)) {
		    if (c == '\r') 
			fprintf( OUTFILE, "Returning char \\r\n" );
		    else if (c == '\n')
			fprintf( OUTFILE, "Returning char \\n\n" );
		    else
			fprintf( OUTFILE, "Returning char ^%c\n", c + 0100 );
		}
		else 
		    fprintf( OUTFILE, "Returning char 0x%x\n", c );
	    }
	}
	if (c == '\n') LineNo[curfile]++;
	return c;
    }

/*  Note that a comment char can NEVER be pushed back, so we don't need
    to check for it here */
    fc = c = lbuffer[lp--];
    if (DebugScan) 
	fprintf( OUTFILE, "Returning pushed-back char %c\n", c );
    return fc;
}

/* This does a move of s by n to the right */
void OverlapCopy( char *s, int n )
{
    int len = strlen(s), i;

    for (i=1; i<=len; i++) 
	s[len+n-i] = s[len-i];
    s[len+n] = 0;
}

/* 
   Append is like push, except that the token is added at the BACK of the list.
   This is appropriate for use by TeXoutstr when it needs to write 
   to the input buffer
 */
void SCAppendToken( char *token )
{
    int len, i, j;
    if (DebugScan)
	fprintf( OUTFILE, "Appending %s [pos=%d]\n", token, lp ); 

    len = strlen(token);
    if (lp + len >= MAX_LBUFFER) {
	fprintf( OUTFILE, "Push-back buffer limit exceeded!\n" );
	exit(1);
    }
    OverlapCopy( lbuffer, len );

    lp += len;
    j  = 0;
    for (i=len-1; i>=0; i--)
	lbuffer[j++] = token[i];
}

/* 
 */
void SCPushToken( char *token )
{
    int len, i;

    if (DebugScan)
	fprintf( OUTFILE, "Pushing back %s [pos=%d]\n", token, lp ); 

    len = strlen(token);
    for (i=len-1; i>=0 && lp < MAX_LBUFFER; i--)
	lbuffer[++lp] = token[i];
    if (i >= 0) {
	fprintf( OUTFILE, "Push-back buffer limit exceeded!\n" );
	exit(1);
    }
    lbuffer[lp+1] = 0;
}    

/* This pushes a token, surrounded by TOK_START ... TOK_END */
void SCPushCommand( char *token )
{
    int len, i;

    if (DebugScan)
	fprintf( OUTFILE, "Pushing back %s [pos=%d]\n", token, lp ); 

    len = strlen(token);
    lbuffer[++lp] = TOK_END;
    for (i=len-1; i>=0 && lp < MAX_LBUFFER; i--)
	lbuffer[++lp] = token[i];
    lbuffer[++lp] = TOK_START;
    if (i >= 0) {
	fprintf( OUTFILE, "Push-back buffer limit exceeded!\n" );
	exit(1);
    }
    lbuffer[lp+1] = 0;
}    

void SCPushChar( char ch )
{
    if (DebugScan)
	fprintf( OUTFILE, "Pushing back %c\n", ch ); 
/* If we have started reading, then we push this character back where it
   can be rescanned (at the current read location */
    if (lp == MAX_LBUFFER) {
	fprintf( OUTFILE, "Push-back buffer limit exceeded!\n" );
	exit(1);
    }
    lbuffer[++lp] = ch;
    lbuffer[lp+1] = 0;
}    

void SCSetCommentChar( char c )
{
    if (DebugCommands) fprintf( stdout, "Setting comment char %c\n", 
				(c > 20) ? c : 'x');
    commentchar = c;
}	


/* Skip whitespace and newlines until a non-newline is found. */
void SCSkipNewlines( FILE *fp )
{
    int c;

/*  Note that a comment char can NEVER be pushed back, so we don't need
    to check for it here */
    while (lp >= 0) {
	c = lbuffer[lp--];
	if (!isspace(c)) {
	    lp++;
	    return;
	}
    }

/* If we reach here, we have exhausted the push-back buffer */

/* Read new characters from file */
    while (1) {
	c = SYTxtGetChar( fp );
	if (c == EOF) return;
	if (c == commentchar) {
	    if (DebugCommands) 
		fprintf( stdout, "Discarding to end of line\n" );
	    SYTxtDiscardToEndOfLine( fp );
	    c = '\n';
	}
	if (c == '\n') LineNo[curfile]++;
	if (!isspace(c)) {
	    SCPushChar( c );
	    return;
	}
    }
}

char SCGetCommentChar( void )
{
    return commentchar;
}

/* Skip whitespace to the first newline and then skip spaces and newlines 
   until a nonblank is found.  Push back the spaces before the first nonblank 
   character */
void SCSkipNewlines2( FILE *fp )
{
    int c;
    int space_count;

/*  Note that a comment char can NEVER be pushed back, so we don't need
    to check for it here */
    while (lp >= 0) {
	c = lbuffer[lp--];
	if (!isspace(c)) {
	    lp++;
	    return;
	}
    }

/* If we reach here, we have exhausted the push-back buffer */

/* Read new characters from file */
    space_count = 0;
    while (1) {
	c = SYTxtGetChar( fp );
	if (c == EOF) return;
	if (c == commentchar) {
	    if (DebugCommands) 
		fprintf( stdout, "Discarding to end of line\n" );
	    SYTxtDiscardToEndOfLine( fp );
	    c = '\n';
	}
	if (c == '\n') {
	    space_count = -1;   /* Because we increment space_count below */
	    LineNo[curfile]++;
	}
	if (!isspace(c)) {
	    SCPushChar( c );
	    while (space_count-- > 0) SCPushChar( (int)' ' );
	    return;
	}
	space_count++;
    }
}

/* This is a version of SYxxx that first draws from the pushback buffer */
void SCTxtDiscardToEndOfLine( FILE *fp )
{
    int c;
    while (lp >= 0) {
	c = lbuffer[lp--];
	if (c == '\n') return;
    }
    SYTxtDiscardToEndOfLine( fp );
}
