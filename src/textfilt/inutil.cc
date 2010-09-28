#include "tfilter.h"

#include "instream.h"

/* 
   These are utilities that do NOT belong to a particular class.

   I'm not sure why I want to do this; in Java I'd have to put them in
   SOME class
 */

/*
   This is a simple finite-state machine for matching individual
   characters or a specified range.  Pattern is of the form

   abc[def]

   This matches abcd or abce or abcf.  The variable match is filled
   in with the matching pattern.  Non-matching characters are 
   discarded.
 */

#define MRESET 1
#define MSTATE 2
#define MCHAR  3
#define MRANGE 4

int FindPattern( InStream *ins, const char *pattern, char *match )
{
    int state = MRESET;
    const char *curpat, *p;
    char ch, *m;

    curpat = pattern;
    while (*curpat) {
	switch (state) {
	    case MRESET:
	    m	   = match;
	    curpat = pattern;
	    state  = MSTATE;
	    break;

	    case MCHAR: 
	    if (ins->GetChar( &ch )) return 1;
	    if (ch == *curpat) {
		*m++ = ch;
		curpat++;
		state  = MSTATE;
		}
	    else {
		state  = MRESET;
		}
	    break;

	    case MRANGE:
	    if (ins->GetChar( &ch )) return 1;
	    p = curpat+1;
	    while (*p && *p != ']' && *p != ch) p++;
	    if (*p != ']') {
		*m++   = ch;
		while (*p && *p != ']') p++;
		curpat = p + 1;
		state  = MSTATE;
		}
	    else
		state = MRESET;
	    
	    break;

	    case MSTATE:
	    if (*curpat == '[') state = MRANGE; else state = MCHAR;
	    break;
	    }
	}
    *m++ = 0;
    return 0;
    
}

int SkipWhite( InStream *ins )
{
	char ch;
	int  rc;
	while ( !(rc = ins->GetChar( &ch )) && (ch == ' ' || ch == '\t') ) ;
	ins->UngetChar( ch );
	return rc;
}

int SkipLine( InStream *ins )
{
	char ch;
	int  rc;
	while ( !(rc = ins->GetChar( &ch )) && ch != '\n') ;
	return rc;
}

int SkipOver( InStream *ins, const char *str )
{	
	char ch;
    while ( *str && !ins->GetChar( &ch ) && ch == *str++) ;
    return 0;
}
