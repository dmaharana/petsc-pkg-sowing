#include "tfilter.h"

#include "textout.h"
#include "textpath.h"
#include <ctype.h>
#include <string.h>

/* 
   This defines the Nroff versions of the output.  Mostly, this setups up
   the general engine and defines the PutChar routine (which handles the 
   special Nroff cases)
 */

int TextOutNroff::Setup( )
{
    InStream *ins;
    int      rc = 0;

    err          = new ErrHandMsg();
    lfont	 = 0;
    nl           = 0;
    last_was_nl	 = 1;
    last_was_par = 1;
    debug_flag   = 0;
    next         = 0;
    strcpy( newline_onoutput, "\n" );
    userops	 = new SrList( 127 );
    ins		 = new InStreamFile( TEXTFILTER_PATH, 
				     "TEXTFILTER_PATH", "nroff.def", "r" );
    if (!ins->status)
        ReadCommands( ins );
    else 
        rc = err->ErrMsg( ins->status, "Error reading nroff.def in path " 
	  TEXTFILTER_PATH " or  TEXTFILTER_PATH environment variable" );
    delete ins;
    return rc;
}

TextOutNroff::TextOutNroff( OutStream *outs )
{
    Setup( );
    out = outs;
}

TextOutNroff::TextOutNroff( )
{
    Setup( );
    out = 0;
}

/* This handles the special cases for Nroff in non-raw mode */
/* We may need some special state for EOL/Space processing */
/* We do.  If we generate a newline, we must eat spaces */
int TextOutNroff::PutChar( const char ch )
{
    int lnl = 0;
    if (last_was_nl && isspace(ch)) return 0;
    switch (ch) {
        case '\\': out->PutQuoted( 0, "\\\\" ); break;
        case '\n': lnl = 1; out->PutChar( ch ); break;
        case '.' : 
	  if (last_was_nl) {
	    // This is a mess.  We must change the command character
	    // and then change it back.
	    //out->PutToken( 0, ".cc ,\n.\n,cc .\n" );
	    // The prefered solution is to use \&
	    out->PutToken( 0, "\\&.\n" );
	    lnl = 1;
	  }
	  else 
	    out->PutChar( ch );
	  break;
	default: out->PutChar( ch ); 
	}
    if (ch) 
      UpdateNL( lnl );
    return 0;
}

int TextOutNroff::PutToken( int nsp, const char *token )
{
     out->PutToken( nsp, (char *)0 );
     while (*token) {
	 PutChar( *token++ );
	 }
     return 0;
}
