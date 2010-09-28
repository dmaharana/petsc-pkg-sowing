#include <stdio.h>
#include <ctype.h>
#include "sowing.h"

#ifdef WIN32
#include <string.h>
#endif
/*
    This file contains some simple routines for providing line-oriented input
    (and output) for FILEs.

    If you have a complicated input syntax and you can use lex and yacc,
    you should consider doing so.  Since lex and yacc produce C programs,
    their output is (roughly) as portable as this code (but DO NOT EXPECT
    yacc and lex to be the same on all systems!)
 */

/*@C
     SYTxtGetLine - Gets a line from a file.

     Input Parameters:
.    fp  - File
.    buffer - holds line read
.    maxlen - maximum length of text read

     Returns:
     The number of characters read.  -1 indicates EOF.  The \n is
     included in the buffer.
@*/
int SYTxtGetLine( fp, buffer, maxlen )
FILE *fp;
char *buffer;
int  maxlen;
{
int  nchar, c;

for (nchar=0; nchar<maxlen; nchar++) {
    buffer[nchar] = c = getc( fp );
    if (c == EOF || c == '\n') break;
    }
if (c == '\n') nchar++;    
buffer[nchar] = 0;    
if (nchar == 0 && c == EOF) nchar = -1;
/* fputs( buffer, stderr ); fflush( stderr ); */
return nchar;
}

/*@C
    SYTxtSkipBlankLines - Return next non-blank line

     Input Parameters:
.    fp  - File
.    buffer - holds line read
.    maxlen - maximum length of text read

    Notes:
    This may be used instead of SYTxtGetLine if blank lines are to be
    ignored.
@*/    
int SYTxtSkipBlankLines( fp, buffer, maxlen )
FILE *fp;
char *buffer;
int  maxlen;
{
int actlen, i;

while ((actlen = SYTxtGetLine( fp, buffer, maxlen )) != EOF) {
    for (i=0; i<actlen; i++) if (buffer[i] != ' ' && buffer[i] != '\n') break;
    if (i < actlen) break;
    }
return actlen;
}


int SYTxtLineIsBlank( buffer, len )
char *buffer;
int  len;
{
int i;

for (i=0; i<len; i++)
    if (buffer[i] != ' ' && buffer[i] != '\n') return 0;
return 1;
}	

/* Find the next space delimited token; put the text into token.
   The number of leading spaces is kept in nsp */
/*@C
    SYTxtFindNextToken - Finds the next space delimited token

    Input Parameters:
.   fp - FILE pointer
.   maxtoken - maximum length of token

    Output Parameters:
.   token - pointer to space for token
.   nsp   - number of preceeding blanks (my be null)

    Returns:
    First character in token or -1 for EOF.
@*/
int SYTxtFindNextToken( fp, token, maxtoken, nsp )
FILE *fp;
char *token;
int  maxtoken, *nsp;
{
int fc, c, Nsp, nchar;

Nsp = SYTxtSkipWhite( fp );

fc = c = getc( fp );
if (fc == EOF) return fc;
*token++ = c;
nchar    = 1;
if (c != '\n') {
    while (nchar < maxtoken && (c = getc( fp )) != EOF) {
	if (c != '\n' && !isspace(c)) {
	    *token++ = c;
	    nchar++;
	    }
	else break;
	}
    ungetc( (char)c, fp );
    }
*token++ = '\0';
*nsp     = Nsp;
return fc;
}

/* Find the next space delimited token; put the text into token.
   The number of leading spaces is kept in nsp */
/*@C
    SYTxtGetChar - Get the next character

    Input Parameters:
.   fp - FILE pointer

    Returns:
    Character or -1 for EOF.
@*/
int SYTxtGetChar( fp )
FILE *fp;
{
return getc( fp );
}

/*@C
    SYTxtFindNextANToken - Finds the next alphanumeric token

    Input Parameters:
.   fp - FILE pointer
.   maxtoken - maximum length of token

    Output Parameters:
.   token - pointer to space for token
.   nsp   - number of preceeding blanks (my be null)

    Returns:
    First character in token or -1 for EOF.
@*/
int SYTxtFindNextANToken( fp, token, maxtoken, nsp )
FILE *fp;
char *token;
int  maxtoken, *nsp;
{
int fc, c, Nsp, nchar;

Nsp = SYTxtSkipWhite( fp );

fc = c = getc( fp );
if (fc == EOF) return fc;
*token++ = c;
nchar    = 1;
if (isalpha(c)) {
    while (nchar < maxtoken && (c = getc( fp )) != EOF) {
        if (isalnum(c)) {
	    *token++ = c;
            nchar++;
	    }
        else {
	    ungetc( (char)c, fp );
	    break;
	    }
        }
    }
else if (isdigit(c)) {
    while (nchar < maxtoken && (c = getc( fp )) != EOF) {
        if (isdigit(c)) {
	    *token++ = c;
            nchar++;
	    }
        else {
	    ungetc( (char)c, fp );
	    break;
	    }
        }
    }
*token++ = '\0';
*nsp     = Nsp;
return fc;
}

/*@C
    SYTxtFindNextAToken - Finds the next alphabetic or numeric token

    Input Parameters:
.   fp - FILE pointer
.   maxtoken - maximum length of token

    Output Parameters:
.   token - pointer to space for token
.   nsp   - number of preceeding blanks (my be null)

    Returns:
    First character in token or -1 for EOF. 

    Notes:
    This differs from SYTxtFindNextANToken in that alpha-numeric tokens
    are not considered a single token.  Rather, a token consists EITHER
    of all alphabetic characters, all numeric characters, or a single other
    character.
@*/
int SYTxtFindNextAToken( fp, token, maxtoken, nsp )
FILE *fp;
char *token;
int  maxtoken, *nsp;
{
int fc, c, Nsp, nchar;

Nsp = SYTxtSkipWhite( fp );

fc = c = getc( fp );
if (fc == EOF) return fc;
*token++ = c;
nchar    = 1;
if (isalpha(c)) {
    while (nchar < maxtoken && (c = getc( fp )) != EOF) {
        if (isalnum(c)) {
	    *token++ = c;
            nchar++;
	    }
        else {
	    ungetc( (char)c, fp );
	    break;
	    }
        }
    }
else if (isdigit(c)) {
    while (nchar < maxtoken && (c = getc( fp )) != EOF) {
        if (isdigit(c)) {
	    *token++ = c;
            nchar++;
	    }
        else {
	    ungetc( (char)c, fp );
	    break;
	    }
        }
    }
*token++ = '\0';
*nsp     = Nsp;
return fc;
}

/*@C
    SYTxtSkipWhite - Skips white space but not newlines.

    Input Parameter:
.   fp - File pointer

    Returns:
    The number of spaces skiped    
@*/   
int SYTxtSkipWhite( fp )
FILE *fp;
{
int c;
int nsp;
nsp = 0;
while ((c = getc( fp )) != EOF) {
    if (!isspace(c) || c == '\n') break;
    nsp++;
    }
ungetc( (char)c, fp );
return nsp;
}

/*@C
  SYTxtSkipOver - Skips over optional text in a file

  Input Parameters:
. fp - File pointer
. str - String to skip over.  At first miss, stop skipping.

  Returns:
  1 if skipped entire string, 0 if encountered non-whitespace character.
@*/
int SYTxtSkipOver( fp, str )
FILE *fp;
char *str;
{
int c;

while ( *str && (c = getc( fp )) != EOF) {
    if (c != *str) { 
	ungetc( (char)c, fp );
	break; 
	}
    str++;
    }
return (*str == 0);
}

/*@C
    SYTxtDiscardToEndOfLine - Discards text until the end-of-line is read

    Input Parameter:
.   fp - File pointer    
@*/
void SYTxtDiscardToEndOfLine( fp )
FILE *fp;
{
int c;

while ((c = getc( fp )) != EOF && c != '\n') ;
}

/*@C
    SYTxtTrimLine - Copies a string  over itself, removing LEADING and 
                    TRAILING blanks.

    Input Parameters:
.   s - Pointer to string

    Returns:
    The final number of characters
@*/
int SYTxtTrimLine( s )
char *s;
{
char *s1 = s;
int  i;

while (*s1 && isspace(*s1)) s1++;
if (s1 != s) {
    for (i=0; s1[i]; i++) s[i] = s1[i];
    s[i] = '\0';
    }
i = strlen(s)-1;
while (i > 0 && isspace(s[i])) i--;
s[i+1] = '\0';
return i+1;
}

/*@C
    SYTxtUpperCase - Converts a string to upper case, in place.

    Input Parameter:
.   s - Pointer to string to convert
@*/
void SYTxtUpperCase( s )
char *s;
{
char c;
while (*s) {
    c = *s;
    if (isascii(c) && islower(c)) *s = toupper(c);
    s++;
    }
}

/*@C
    SYTxtLowerCase - Converts a string to lower case, in place.

    Input Parameter:
.   s - Pointer to string to convert
@*/
void SYTxtLowerCase( s )
char *s;
{
char c;
while (*s) {
    c = *s;
    if (isascii(c) && isupper(c)) *s = tolower(c);
    s++;
    }
}

