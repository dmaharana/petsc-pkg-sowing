#include "tfilter.h"

#include "textout.h"
#include "textpath.h"
#include <string.h>

/* 
   This defines the HTML versions of the output.  Mostly, this setups up
   the general engine and defines the PutChar routine (which handles the 
   special HTML cases)
 */

int TextOutHTML::Setup( )
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
    				     "TEXTFILTER_PATH", "html.def", "r" );
    if (!ins->status)
        ReadCommands( ins );
    else
        rc = err->ErrMsg( ins->status, "Error reading html.def in path "
	  TEXTFILTER_PATH " or  TEXTFILTER_PATH environment variable" );
    delete ins;
    return rc;
}

TextOutHTML::TextOutHTML( OutStream *outs )
{
    Setup( );
    out = outs;
}

TextOutHTML::TextOutHTML( )
{
    Setup( );
    out = 0;
}

/* This handles the special cases for HTML in non-raw mode */
int TextOutHTML::PutChar( const char ch )
{
    switch (ch) {
    case '<': out->PutQuoted( 0, "&lt;" ); break;
    case '>': out->PutQuoted( 0, "&gt;" ); break;
    case '&': out->PutQuoted( 0, "&amp;" ); break;
    default: out->PutChar( ch );
    }
    if (ch) 
      UpdateNL( ch == '\n' );
    return 0;
}

int TextOutHTML::PutToken( int nsp, const char *token )
{
     out->PutToken( nsp, (char *)0 );
     if (token) {
       while (*token) {
	 PutChar( *token++ );
       }
     }
     return 0;
}
