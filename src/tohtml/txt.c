#include <stdio.h>

/*
    This file contains some simple routines for providing line-oriented input
    (and output) for FILEs.
 */

/* @
     SYTxtGetLine - Gets a line from a file.

     Input Parameters:
.    fp  - File
.    buffer - holds line read
.    maxlen - maximum length of text read

     Returns:
     The number of characters read.  -1 indicates EOF.  The \n is
     included in the buffer.
@ */
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
