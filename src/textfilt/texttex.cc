#include "tfilter.h"

#include "textout.h"
#include "textpath.h"
#include <string.h>

/* 
   This defines the LaTeX versions of the output.  Mostly, this setups up
   the general engine and defines the PutChar routine (which handles the 
   special LaTeX cases)
 */

int TextOutTeX::Setup( )
{
    InStream *ins;
    int      rc = 0;

    err          = new ErrHandMsg( );
    lfont	 = 0;
    mode         = 0;
    nl           = 0;
    last_was_nl	 = 1;
    last_was_par = 1;
    debug_flag   = 0;
    next         = 0;
    strcpy( newline_onoutput, "\n" );
    userops	 = new SrList( 127 );
    ins		 = new InStreamFile( TEXTFILTER_PATH, 
    				     "TEXTFILTER_PATH", "latex.def", "r" );
    if (!ins->status) 				    
        ReadCommands( ins );
    else
    	rc = 1;
    delete ins;
    return err->ErrMsg( rc, "Could not open latex.def file in path "
	  TEXTFILTER_PATH " or  TEXTFILTER_PATH environment variable" );
}

TextOutTeX::TextOutTeX( OutStream *outs )
{
    Setup();
    out = outs;
}

TextOutTeX::TextOutTeX( )
{
    Setup();
    out = 0;
}

/* This handles the special cases for TeX in non-raw mode */
int TextOutTeX::PutChar( const char ch )
{
    int rc;

    if (ch)
      UpdateNL( ch == '\n' );
    if (mode && *mode) {
      // Verbatim mode
      if (debug_flag && ch) printf( "putting char %c in verbatim\n", ch );
      return out->PutChar( ch );
    }
    if (debug_flag && ch) printf( "putting char %c in TeX mode\n", ch );
    switch (ch) {
	case '\\': rc = out->PutQuoted( 0, "{\\tt \\char`\\\\}" ); break;
	case '<': rc = out->PutQuoted( 0, "$<$" ); break;
	case '>': rc = out->PutQuoted( 0, "$>$" ); break;
	case '=': rc = out->PutQuoted( 0, "$=$" ); break;
	case '^': rc = out->PutQuoted( 0, "{\\tt \\char`\\^}" ); break;
	case '_': rc = out->PutQuoted( 0, "{\\tt \\char`\\_}" ); break;
	case '#': rc = out->PutQuoted( 0, "{\\tt \\char`\\#}" ); break;
	case '&': rc = out->PutQuoted( 0, "\\&" ); break;
	case '$': rc = out->PutQuoted( 0, "\\$" ); break;
	case '%': rc = out->PutQuoted( 0, "\\%" ); break;
	case '|': rc = out->PutQuoted( 0, "{\\tt \\char`\\|}" ); break;
	case '{': rc = out->PutQuoted( 0, "\\{" ); break;
	case '}': rc = out->PutQuoted( 0, "\\}" ); break;
	default: rc = out->PutChar( ch );
    }
	if (rc) 
        	return err->ErrMsg( rc, "Error writing character" );
       	else return 0;
}

int TextOutTeX::PutToken( int nsp, const char *token )
{
    int rc;
    rc = out->PutToken( nsp, (char *)0 );
    if (!rc) err->ErrMsg( rc, "Error writing token" );
    while (*token) {
	 rc = PutChar( *token++ );
	 if (rc) { err->ErrMsg( rc, "Error writing character" ); break; }
	 }
     return rc;
}
