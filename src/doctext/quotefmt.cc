/*
 * This file defines a TextOut instance that does quote to font-change
 * formatting.  The quote format is '...' -> tt and `...` -> em
 *
 */
#include "docutil.h"
#include "quotefmt.h"
#include <string.h>

TextOutQFmt::TextOutQFmt( TextOut *in_outs )
{
    tt_char	 = '\'';
    em_char	 = '`';
    tt_cnt	 = 0;
    em_cnt	 = 0;
    last_tt	 = 0;
    last_em	 = 0;
    next	 = in_outs;
    
    lfont	 = 0;
    nl		 = 0;
    last_was_nl	 = 0;
    last_was_par = 0;
    debug_flag	 = 0;
    userops	 = 0;

    action_flag  = 1;
}

const char * TextOutQFmt::SetMode( const char *str )
{
  return next->SetMode( str );
}

int TextOutQFmt::PutChar( const char ch )
{
    if (action_flag) {
	if (last_tt) {
	    // Last character was tt_char.  Handle it first
	    last_tt = 0;
	    if (ch == tt_char) {
		return next->PutChar( ch );
	    }
	    else {
		if (tt_cnt++ == 0) 
		    next->PutOp( "tt" );
		else {
		    next->PutOp( "rm" );
		    tt_cnt = 0;
	        }
	    }
        }
	else if (ch == tt_char) {
	    last_tt = 1;
	    return 0;
	}
	if (last_em) {
	    // Last character was em_char.  Handle it first
	    last_em = 0;
	    if (ch == em_char) {
		return next->PutChar( ch );
	    }
	    else {
		if (em_cnt++ == 0) 
		    next->PutOp( "em" );
		else {
		    next->PutOp( "rm" );
		    em_cnt = 0;
	        }
	    }
        }
	else if (ch == em_char) {
	    last_em = 1;
	    return 0;
	}
    }
    // Allow PutChar( 0 ) to flush
    if (ch) 
        return next->PutChar( ch );
    return 0;
}

// Trap eop only for output of error messages
int TextOutQFmt::PutOp( const char *command )
{
    PutChar( 0 );
    if (strcmp( command, "eop" ) == 0) {
	 if (tt_cnt != 0)
	     fprintf( stderr, "Unclosed tt quote in %s\n", 
		      GetCurrentFileName() );
	 if (em_cnt != 0)
	     fprintf( stderr, "Unclosed em quote in %s\n",
		      GetCurrentFileName() );
	 tt_cnt = 0;
	 em_cnt = 0;
        }
    return next->PutOp( command );
}
int TextOutQFmt::PutOp( const char *command, char *s1, char *s2, int i1 )
{
  return next->PutOp( command, s1, s2, i1 );
}

int TextOutQFmt::PutOp( const char *command, char *s1, char *s2, char *s3, 
			char *s4 )
{
  return next->PutOp( command, s1, s2, s3, s4 );
}

int TextOutQFmt::PutOp( const char *command, char *s1 )
{
  return next->PutOp( command, s1 );
}

int TextOutQFmt::PutOp( const char *command, char *s1, int i1 )
{
  return next->PutOp( command, s1, i1 );
}

int TextOutQFmt::Flush()
{
    if (action_flag) {
	if (last_tt || last_em)
	    PutChar( 0 );
    }
    return next->Flush();
}

int TextOutQFmt::PutToken( int nsp, const char *token )
{
    int i;
    for (i=0; i<nsp; i++) PutChar( ' ' );
    while (*token) PutChar( *token++ );
	return 0;
}

int TextOutQFmt::SetAction( int flag )
{
    int old_flag = action_flag;

    action_flag = flag;
    return old_flag;
}
