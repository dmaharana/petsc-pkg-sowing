#include "textout.h"
#include "instream.h"
#include "doc.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/*
   This file handles creating a keyword table.  This WILL change as
   we add tools for the keyword interface.

   Because of the nature of the processing, we need to remember the 
   name of the function being documented, and the filename where it
   is being written.  These will be provide by parser for the man pages 
   themselves.
 */
static OutStream *keywordfile = 0;

/* 
  Give the name of the file.  - stands for stdout.
 */
int KeywordOpen( const char *name )
{
    if (strcmp( name, "-" ) == 0) {
	keywordfile = new OutStreamFile( );
	return 0;
	}
    keywordfile = new OutStreamFile( name, "a" );
    if (!keywordfile->status) {
	fprintf( stderr, "Could not open keyword file %s\n", name );
	return 1;
	}

    return 0;
}

int KeywordOut( char *line )
{
    extern const char *GetCurrentRoutinename( void );
    extern const char *GetCurrentFileName( void );

    if (!keywordfile) return 0;

    keywordfile->PutToken( 0, GetCurrentRoutinename() );
    keywordfile->PutToken( 1, GetCurrentFileName() );
    keywordfile->PutToken( 1, line );
    
    return 0;
}
