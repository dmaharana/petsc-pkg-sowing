#include "textout.h"
#include "doc.h"
#include "docutil.h"
#include <ctype.h>
#include <string.h>

char LeadingString[10] = "";

/*
   This file contains service routines to do things like read
   a description and skip to the Synopsis
 */

/*
   The description is of the form 
   [space]*<name>[space]*[-]*[text,including\n]\n\n
   That is, two consequtive newlines end the description.

   This can be used with a file or buffer output stream.

   The "name" can have blanks or commas in it; this allows the use
   of a single file to decribe a number of related commands.
 */

int DocReadName( InStream *ins, char *routinename, int maxlen )
{
    char ch;

    // [space]*
    while (!ins->GetChar( &ch ) && isspace(ch)) {
      if (ch == '\n') {
	SkipLeadingString( ins, &ch );
	ins->UngetChar( ch );
      }
    }
    ins->UngetChar( ch );

    // <name>
    while (!ins->GetChar( &ch ) && !isspace(ch) && ch != ',' && maxlen) {
	*routinename++ = ch; maxlen--;
	}
    *routinename = 0;
    return 0;

}

// If flag is true, output the - 
int DocReadDescription( InStream *ins, char *matchstring, 
			TextOut *textout, int flag, int *at_end )
{
    char ch;
    int  newline_cnt;
    int  state, i;
    int  b_state = strlen(matchstring) - 1;

    // [space]*
    while (!ins->GetChar( &ch ) && isspace(ch)) {
      if (ch == '\n') {
	SkipLeadingString( ins, &ch );
	ins->UngetChar( ch );
      }
    }
    ins->UngetChar( ch );

    // [-]*
    while (!ins->GetChar( &ch ) && ch == '-') ;
    // This needs to be \- for nroff, --- for LaTeX, and <BR> for HTML.
    if (flag) 
	textout->PutChar( '-' );
	
    // [space]*
    while (!ins->GetChar( &ch ) && isspace(ch)) {
      if (ch == '\n') {
	SkipLeadingString( ins, &ch );
	ins->UngetChar( ch );
      }
    }
    ins->UngetChar( ch );

    // Text, until two consequtive newlines or matchstring
    newline_cnt = 0;
    state = b_state;
    while (state >= 0 && !ins->GetChar( &ch )) {
	if (ch == matchstring[state]) {
	    state--;
	    continue;
	    }
	else {
	  if (state != b_state) {
	    // Must output the mismatched string
	    for (i=b_state; i>state; i--)
	      textout->PutChar( matchstring[i] );
	    state = b_state;
	  }
	}
	if (ch == '\n') {
	    newline_cnt++ ; 
	    if (newline_cnt == 1) {
	      SkipLeadingString( ins, &ch );
	      ins->UngetChar( ch );
	    }
	    ch = ' ';
	}
	else if (newline_cnt && isspace(ch)) {
	    // Allow spaces in the newline field; don't output.
	    continue; 
	}
	else
	    newline_cnt = 0;
	if (newline_cnt == 2) break;
	textout->PutChar( ch );
	}
    *at_end = (state < 0);
    return 0;
}

/*
   Skip to the START of a function synopsis.  This is just skipping to the
   end of the structured comment UNLESS a SYNOPSIS: is seen.
   This case is NOT handled yet (not even detected!)
 */
int DocSkipToFuncSynopsis( InStream *ins, char *matchstring )
{
    char ch;
    int state;
    int b_state = strlen(matchstring) - 1;

    state = b_state;
    while (state >= 0 && !ins->GetChar( &ch )) {
	if (ch == matchstring[state]) {
	    state--;
	    }
	else 
	    state = b_state;
	}

    // Skip to the first non-blank character 
    while (!ins->GetChar( &ch ) && isspace(ch)) ;
    ins->UngetChar( ch );

    return 0;
}

//
// Read the synopsis and send to the out stream.  Remove "register"
// if is appears.
// Also strip any ignore tokens.
//
int DocReadFuncSynopsis( InStream *ins, OutStream *outs )
{
    char ch;
    char token[256];
    int  nl_break;
    int  us_break;
    int maxlen = 255, nsp;

    // Must handle newline as non-space
    
    nl_break = ins->breaktable['\n'];
    us_break = ins->breaktable['_'];
    ins->SetBreakChar( '\n', BREAK_OTHER );
    ins->SetBreakChar( '_', BREAK_ALPHA );
    while (!ins->GetToken( maxlen, token, &nsp ) && token[0] != '{') {
      // Eventually we need to combine these into a single lookup list.
      if (strcmp( token, "register" ) == 0) {
	/* Skip blanks */
	while (!ins->GetChar( &ch ) && isspace(ch) ) ;
	ins->UngetChar( ch );
      }
      else if (IgnoreString && strcmp( token, IgnoreString ) == 0) {
	/* Skip blanks */
	while (!ins->GetChar( &ch ) && isspace(ch) ) ;
	ins->UngetChar( ch );
      }
      else {
	// Check for \n and do PutNewline instead.
	if (token[0] == '\n') {
	  outs->PutToken( nsp, NewlineString );
	}
	else
	  outs->PutToken( nsp, token );
      }
    }
    ins->SetBreakChar( '\n', nl_break );
    ins->SetBreakChar( '_', us_break );
    return 0;
}

/* 
   Skip to the START of a function synopsis.  This is just skipping to 
   EITHER the end (no synopsis) or finding a line containing 
   Synopsis:
   on it 
 */
int DocSkipToMacroSynopsis( InStream *ins, char *matchstring )
{
    int state, sy_state;
    char ch;
    static char synopsis[10] = "Synopsis:";
    int b_state = strlen(matchstring) - 1;

    state    = b_state;
    sy_state = 1;
    while (state >= 0 && sy_state != 10 && !ins->GetChar( &ch )) {
	if (ch == matchstring[state]) {
	    state--;
	    }
	else {
	    state = b_state;
	    }
	if (ch == '\n') {
	    // reset state for synopsis
	    sy_state = 1;
	    }
	else {
	    switch (sy_state) {
		case 0: break;
		case 1:
			// Look for the first character
		if (!isspace(ch)) {
		    if (ch != 'S' && ch != 's') sy_state = 0;
		    else sy_state = 2;
		    }
		break;

	        default:
		if (ch == synopsis[sy_state-1]) sy_state++;
		else sy_state = 0;
		}
	    }    
	}

    // Skip to the first non-blank character 
    while (!ins->GetChar( &ch ) && isspace(ch)) ;
    ins->UngetChar( ch );

    return 0;
}

/* Read until we get two newlines */
int DocReadMacroSynopsis( InStream *ins, char *matchstring, OutStream *outs,
			  int *at_end )
{
    char ch;
    int  newline_cnt, i;
    int  state, b_state = strlen(matchstring) - 1;

    // Text, until two consequtive newlines OR end of comment
    newline_cnt = 1;
    SkipLeadingString( ins, &ch );
    ins->UngetChar( ch );

    state = b_state;
    while (state >= 0 && !ins->GetChar( &ch )) {
	if (ch == matchstring[state]) {
	    state--;
	    continue;
	    }
	else {
	  if (state != b_state) {
	    // Must output the mismatched string
	    for (i=b_state; i>state; i--)
	      outs->PutChar( matchstring[i] );
	    state = b_state;
	  }
	}

	if (ch == '\n') {
	    newline_cnt++ ; 
	    SkipLeadingString( ins, &ch );
	    ins->UngetChar( ch );
	    ch = '\n';
	}
	else if (newline_cnt && isspace(ch)) {
	    // Allow spaces in the newline field; don't output.
	    continue; 
	    }
	else {
	    newline_cnt = 0;
	}
	if (newline_cnt == 2) break;
	if (ch == '\n') 
	  outs->PutToken( 0, NewlineString );
	else
	  outs->PutChar( ch );
	}
    *at_end = state < 0;
    return 0;
}

// Read C and/or X
static int HasX = 0;
static int HasC = 0;
int DocGetSubOptions( InStream *ins )
{
    char ch;

    HasX = 0;
    HasC = 0;

    if (ins->GetChar( &ch )) return 1;
    if (ch == 'X') HasX = 1;
    else if (ch == 'C') HasC = 1;
    else {
	ins->UngetChar( ch );
	return 0;
	}

    if (ins->GetChar( &ch )) return 1;
    if (ch == 'X') HasX = 1;
    else if (ch == 'C') HasC = 1;
    else {
	ins->UngetChar( ch );
	return 0;
	}
    return 0;
}
int DocSubOption( int *hasx, int *hasc )
{
    if (hasx) *hasx = HasX;
    if (hasc) *hasc = HasC;
    return 0;
}

/*
   Match str1 to str2, ignoring case of str1 (str2 is uppercase)
   returns 0 for false, 1 for true
 */
int DocMatchTokens( const char *str1, const char *str2 )
{

    while (*str1) {
	if (toupper(*str1) != *str2) return 0;
	str1++;
	str2++;
        }
    return *str2 == 0;
}


// Skip any leading string
int SkipLeadingString( InStream *ins, char *ch )
{
  int rc;
  char *p = LeadingString;

  while (*p) {
    if ( (rc = ins->GetChar( ch )) ) return rc;
    if (*p != *ch) return 2;
    p++;
  }
  return ins->GetChar( ch );
}

char *SkipOverLeadingString( char *buf )
{
  char *p = LeadingString;

  while (*p && *buf == *p) { p++; buf++; }
  return buf;
}

void UngetLeadingString( InStream *ins )
{
  char *p = LeadingString + (strlen(LeadingString) - 1);
  while (p >= LeadingString) 
    ins->UngetChar( *p-- );
}
