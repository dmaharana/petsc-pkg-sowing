/*
 * This file handles generating information about the include files
 * needed in order to use (not compile) a routine
 */

#include "doc.h"
#include "textout.h"
#include <string.h>
#include <stdio.h>
#include "incfiles.h"

/* Global state for these routines */
FILE         *incfd = 0;
OutStreamBuf *includes = 0;

#define MAX_INCLUDE_BUF 256

int SetIncludeFile( const char *path )
{
    if (path)
	incfd = fopen( path, "r" );
    else
	incfd = 0;
    includes = new OutStreamBuf( MAX_INCLUDE_BUF );    
    return includes->status;
}

int OutputIncludeInfo( TextOut *textout )
{
    int c;
    if (incfd) {
        fseek( incfd, 0L, 0 );
        while ((c = getc( incfd )) != EOF) 
	    textout->PutChar( c );
	}

    // Output the includes local to this file
    // Change into ...\n... into #include ...\n#include ...\n
    const char *p = includes->GetBuffer();
    const char *pend;
    char  tmpbuf[MAX_INCLUDE_BUF+10];

    strcpy( tmpbuf, "#include " );
    while (p && *p) {
      while (*p && *p == ' ') p++;
      pend = strchr( p, '\n' );
      if (!pend) {
	strcpy( tmpbuf+9, p );
	p = 0;
      }
      else {
	int plen = 1+(int)(pend - p);
	strncpy( tmpbuf+9, p, plen );
	tmpbuf[9 + plen] = 0;
	p = p + plen;
      }
      textout->PutToken( 0, tmpbuf );
    }

    return 0;
}

// Read a I ... I and save it
int SaveIncludeFile( InStream *ins, char *matchstring )
{
    char ch;
    int  rc = 0, state, base_state, i;

    state = strlen( matchstring ) - 1;
    base_state = state;
    while (state >= 0 && !ins->GetChar( &ch )) {
    	// Check for end of definition
    	if (ch == matchstring[state]) {
    	    state--;
    	    continue;
    	    }
    	else if (state != base_state) {
    	    // Was matching, but failed.  Output chars so far; reset state
    	    for (i=base_state; i>state; i--)
    	    	includes->PutChar( matchstring[i] );
    	    state = base_state;
    	    }
    	// Add to output
    	includes->PutChar( ch );
    	// Don't allow I...I to cross a newline
    	if (ch == '\n') return 1;
        }
   includes->PutToken( 0, NewlineString );
   return rc;
}
// Call this when starting a new input file
int ClearIncludeFile( )
{
    includes->Reset();
    return 0;
}
