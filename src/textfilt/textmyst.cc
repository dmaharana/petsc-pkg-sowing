#include "tfilter.h"

#include "textout.h"
#include "textpath.h"
#include <string.h>

/*
   This defines the MyST versions of the output.  Mostly, this setups up
   the general engine and defines the PutChar routine (which handles the
   special MyST cases)
 */

int TextOutMyST::Setup( )
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
    				     "TEXTFILTER_PATH", "myst.def", "r" );
    if (!ins->status)
        ReadCommands( ins );
    else
        rc = err->ErrMsg( ins->status, "Error reading myst.def in path "
	  TEXTFILTER_PATH " or  TEXTFILTER_PATH environment variable" );
    delete ins;
    return rc;
}

TextOutMyST::TextOutMyST( OutStream *outs )
{
    Setup( );
    out = outs;
}

TextOutMyST::TextOutMyST( )
{
    Setup( );
    out = 0;
}

/* This handles the special cases for MyST in non-raw mode */
int TextOutMyST::PutChar( const char ch )
{
    switch (ch) {
    default: out->PutChar( ch );
    }
    if (ch)
      UpdateNL( ch == '\n' );
    return 0;
}

int TextOutMyST::PutToken( int nsp, const char *token )
{
     out->PutToken( nsp, (char *)0 );
     if (token) {
       while (*token) {
	 PutChar( *token++ );
       }
     }
     return 0;
}

int TextOutMyST::PutTokenRaw(int nsp, const char *token)
{
    // Nothing special in this case
    int rc = PutToken(nsp, token);
    return rc;
}
