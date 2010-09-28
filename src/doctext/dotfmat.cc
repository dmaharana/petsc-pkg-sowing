/* -*- Mode: C++; c-basic-offset:4 ; -*- */
/*
    This file contains code to process various formatting commands
    that are indicated by a "." in the first column

    Currently, we support
    . (. space)   for arguments
    + and -(space)for argument lists (beginning and end)
    .\t           for arguments (\t is tab)
    .N name       Insert the previously defined name block (*N name ... N*)
    .n            new line at here
    .vb           Begin verbatim (i.e., LaTeX verbatim or HTML PRE)
    .ve           End verbatim
    .p filename   Picture (PS files)
    .cb           Begin picture caption
    .ce           End picture caption                   
    .f            Execute user-defined function (e.g., .fin, .fout)

    Adding now
    .Es           Begin enumerate
    .Ee           End enumerate
    .i            item (like a \item in LaTeX)
    .Bqs          Begin block quote
    .Bqe          End block quote

    Soon to be added:
    .c citeref    Citation reference (eventually, we will be able to
                  convert this into a hot reference)
 */

#include "instream.h"
#include "textout.h"
#include "inutil.h"
#include "doc.h"
#include "docutil.h"
#include "dotfmt.h"
#include "keyword.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* Handle . name - description (argument definition) 
 * The old form was
 *     . name description
 * but this doesn't let you handle things like
 *     . -foo filename - description (for programs)
 * or
 *     . nx, ny - description (for routines)
 * so the new format is
 *     . (text) - description
 * or
 *     . any text <newline>
 *         description
 * where text is any text that does not START with a -.
 * You can escape the - with a \ .
 * In addition, an argument list can start with a + instead of a . and
 * end with a - instead of a dot; this allows the argument list as a whole
 * to be formatted.
 */  
int ProcessArgFmt( char arg_kind, InStream *ins, TextOut *textout, 
		   int *lastWasNl )
{
    char ch;
    int  at_first = 1;
    int  last_was_backslash;

    SkipWhite( ins );

    switch (arg_kind) {
    case ARGUMENT_BEGIN:
      InArgList = 1;
      textout->PutOp( "s_arg_list" );  
      /* No break; we fall through to the end */
    case ARGUMENT:
    case ARGUMENT_END:
      if (InArgList) 
	textout->PutOp( "s_arg_inlist" );
      else
	textout->PutOp( "s_arg" );
      break;
    }

    /* Get the name */
    last_was_backslash = 0;
    while ( !ins->GetChar( &ch ) ) {
	if (ch == '\n' || (ch == '-' && !at_first && !last_was_backslash)) { 
	  *lastWasNl = 1; break; }
	if (last_was_backslash && ch != '-') {
	  textout->PutChar( '\\' );
	}
	last_was_backslash = (ch == '\\');
	if (!last_was_backslash) {
	  // We don't output the - (separator)
	  textout->PutChar( ch );
	}
	at_first = 0;
    }
    if (InArgList) {
      textout->PutOp( "e_arg_inlist" );
      textout->PutOp( "s_defn_inlist" );
    }
    else {
      textout->PutOp( "e_arg" );
      textout->PutOp( "s_defn" );
    }
    SkipWhite( ins );
    while ( !ins->GetChar( &ch ) ) {
	if (ch != '\n') { 
	    textout->PutChar( ch );
	}
	else {
	    int ns = 0;
	    textout->PutNewline();
	    /* Check for a "continued" description, defined as:
	       next line is neither EMPTY nor starts with . or $ */
	    // Skip the prefix, peek at next character
	    SkipLeadingString( ins, &ch );
	    //ins->GetChar( &ch );
	    ins->UngetChar( ch );
	    /* Check for next character a formatting command or an
	       empty line */
	    // This needs to be changed to check in the list of chars
	    // Note that since this is the first character in the reserved
	    // position, we don't need to check for the full termination
	    // string
	    if (ch == ARGUMENT || ch == ARGUMENT_END || ch == VERBATIM || 
		ch == '\n' || ch == '@' || ch == 'M' || ch == 'D') {
	      // We must also unget the leading string
	      UngetLeadingString( ins );
	      *lastWasNl = 1; break;
	    }
	    /* Check for a blank line, looking for lines of all blanks
	       as well as those containing only a newline. */
	    if (ch == ' ' || ch == '\t') {
	      while (ch == ' ' || ch == '\t') {
		ns++;
		if (ins->GetChar( &ch )) break;
	      }
	      // Check for a blank line
	      if (ch == '\n') {
		*lastWasNl = 1; break;
	      }
	      // Really should check for <char> * / 
	      if (ch == '@' || ch == 'M' || ch == 'D') {
		char pat[4];
		pat[0] = ch;
		if (ins->GetChar( &pat[1] )) break;
		if (pat[1] == '*') {
		  if (ins->GetChar( &pat[2] )) break;
		  // We're going to unget these characters in any case
		  ins->UngetChar( pat[2] );
		  ins->UngetChar( pat[1] );
		  if (pat[2] == '/') {
		    *lastWasNl = 1; break;
		  }
		}
		else 
		  ins->UngetChar( pat[1] );
		}
	      // Unget the first non-blank character.
	      ins->UngetChar( ch );
	    }
	}
    }
    if (InArgList) {
      textout->PutOp( "e_defn_inlist" );
    }
    else {
      textout->PutOp( "e_defn" );
    }
    if (arg_kind == ARGUMENT_END) {
      textout->PutOp( "e_arg_list" );
      InArgList = 0;
    }
    return 0;
}


/* Process a verbatim mode line (either $ ... or .<haven't decided>) */
/* Use s_doctext_verbatim and e_doctext_verbatim commands */
// While in verbatim mode, we may want to suppress any processing of
// quote characters.  We need to add that as an option to the
// quotefmt.cc routines.  The problem is that textout is the super class,
// so we don't have a control for the operation.
int ProcessVerbatimFmt( InStream *ins, TextOut *textout, int *lastWasNl )
{
    char ch;
    int quote_flag;

    /* Raw mode output. */
    if (ins->GetChar( &ch )) return 1;
    if (ch == ' ' || ch == '\t' || ch == '\n') {
	if (*lastWasNl) 
	    textout->PutOp( "s_doctext_verbatim" );
	}
    else {
	textout->PutChar( '.' );
	textout->PutChar( ch );
	}
    if (ch != '\n') {
	/* Preserve spacing */
	while (!ins->GetChar( &ch ) && isspace(ch)) {
	    /* Eventually, we want to handle tabs etc */
	    textout->PutOp( "blank" );
	    }
	ins->UngetChar( ch );
	/* SkipWhite( fin ); */
	while ( !ins->GetChar( &ch ) ) {
	    if (ch == '\n') { *lastWasNl = 1; break; }
	    textout->PutChar( ch );
	    }
	}
    /* At the end of the line, generate the verbatim-end command */
    textout->PutOp( "e_doctext_verbatim" );
    return 0;
}

/* Process a .N name include.  The N has been read, the name has not */
int ProcessNameFmt( InStream *ins, TextOut *textout, int *lastWasNl )
{
    char ch;
    char name[256], *p;

    /* Get the name */
    SkipWhite( ins );
    p = name;
    while ( !ins->GetChar( &ch ) && !isspace(ch)) *p++ = ch;
    *p = 0;

    if (!IncludeNameBlock( ins, name )) {
	fprintf( stderr, "Could not find include name (%s) in %s\n", name,
		 GetCurrentFileName() );
	return 1;
	}
    return 0;
}

/* Process a .vb ... .ve block */
// This really needs to be able to turn off quote-format processing.
int ProcessVerbatimBlockFmt( InStream *ins, TextOut *textout, int *lastWasNl )
{
  char ch;
  char buf[256];
  char *p;
  const char *savemode;
  
  /* Check for .vb */
  if (!ins->GetChar( &ch ) && ch != 'b') {
    fprintf( stderr, "Invalid verbatim command '.v%c' in %s\n", ch,
	     GetCurrentFileName() );
    return 1;
  }
  SkipLine( ins );
  textout->PutOp( "s_verbatim" );
  // We need to suppress character translation when in verbatim
  // mode, at least in LaTeX.
  savemode = textout->SetMode( "v" );
  while (!ins->GetLine( buf, 256 )) {
    // Remove leading string
    p = SkipOverLeadingString( buf );
    if (strncmp( p, ".ve", 3 ) == 0) break;
    textout->PutToken( 0, p );
  }
  (void)textout->SetMode( savemode );
  textout->PutOp( "e_verbatim" );
  textout->PutNewline();
  return 0;
}

/* Process a .cb ... .ce block */
int ProcessCaption( InStream *ins, TextOut *textout, int *lastWasNl )
{
    char ch;
 
    /* Check next character */
    if (ins->GetChar( &ch )) return 1;
    if (ch == 'b')      textout->PutOp( "s_caption" );
    else if (ch == 'e') textout->PutOp( "e_caption" );
    else {
	fprintf( stderr, "Invalid caption command '.c%c' in %s\n", ch,
		 GetCurrentFileName() );
	return 1;
	}
    return 0;
}

/* Process a .p name command */
int ProcessPicture( InStream *ins, TextOut *textout, int *lastWasNl )
{
    char buf[256];

    /* Get filename */
    SkipWhite( ins );

    if (ins->GetLine( buf, 256 )) {
	fprintf( stderr, "Could not read picture filename in %s\n",
		 GetCurrentFileName() );
	return 1;
	}
    /* Remove newline from end of buffer (should have a Chop function) */
    buf[strlen(buf)-1] = 0;
    return textout->PutOp( "picture", buf );
}

int ProcessKeywords( InStream *ins, TextOut *textout, int *lastWasNl )
{
    char buf[1024];

    /* Skip over 'eywords' if present */
    SkipOver( ins, "eywords:" );
    
    /* Pass the keywords to the keyword routine */
    ins->GetLine( buf, 1024 );
    
    textout->PutOp( "section", (char*)"Keywords" );
    textout->PutToken( 0, buf );
    textout->PutOp( "linebreak" ); // lastwasNL?
    
    KeywordOut( buf );
    return 0;
}

int ProcessSeeAlso( InStream *ins, TextOut *textout, int *lastWasNl )
{
    char buf[1024];

    /* Skip over 'eealso' if present */
    SkipOver( ins, "eealso:" );
    
    /* Pass the tokens to the seealso routine */
    ins->GetLine( buf, 1024 );
    
    textout->PutOp( "section", (char *)"See Also" );
    textout->PutToken( 0, buf );
    textout->PutOp( "linebreak" ); // lastwasNL?
    return 0;
}

/* Process a .fname command */
int ProcessUserCmd( InStream *ins, TextOut *textout, int *lastWasNl )
{
    char buf[256];

    /* Get command name */
    SkipWhite( ins );

    if (ins->GetLine( buf, 256 )) {
	fprintf( stderr, "Could not read user command %s\n",
		 GetCurrentFileName() );
	return 1;
	}
    /* Remove newline from end of buffer (should have a Chop function) */
    buf[strlen(buf)-1] = 0;
    return textout->PutOp( buf );
}

/* Process a .Es .Ee block */
int ProcessEnumerate( InStream *ins, TextOut *textout, int *lastWasNl )
{
    char ch;
 
    /* Check next character */
    if (ins->GetChar( &ch )) return 1;
    if (ch == 'b')      textout->PutOp( "s_enumerate" );
    else if (ch == 'e') textout->PutOp( "e_enumerate" );
    else {
	fprintf( stderr, "Invalid enumerate command '.E%c' in %s\n", ch,
		 GetCurrentFileName() );
	return 1;
	}
    return 0;
}

int ProcessItemize( InStream *ins, TextOut *textout, int *lastWasNl )
{
    char buf[1024];

    textout->PutOp( "itemize_enum" );
#if 0
    /* Pass the tokens to the seealso routine */
    ins->GetLine( buf, 1024 );
    
    textout->PutOp( "section", (char *)"See Also" );
    textout->PutToken( 0, buf );
    textout->PutOp( "linebreak" ); // lastwasNL?
#endif
    return 0;
}

/* Process a .B<op>b ... .B<op>e block */
int ProcessBlockOp( InStream *ins, TextOut *textout, int *lastWasNl )
{
    char ch;
 
    /* Check next character */
    if (ins->GetChar( &ch )) return 1;
    if (ch != 'q') {
      fprintf( stderr, "Expected 'q' but saw %c in .B command in %s\n", ch,
	       GetCurrentFileName() );
      return 1;
    }
    if (ins->GetChar( &ch )) return 1;
    if (ch == 's')      textout->PutOp( "s_blockquote" );
    else if (ch == 'e') textout->PutOp( "e_blockquote" );
    else {
	fprintf( stderr, "Invalid block command '.Bq%c' in %s\n", ch,
		 GetCurrentFileName() );
	return 1;
	}
    return 0;
}

int ProcessDotFmt( char arg_kind, InStream *ins, TextOut *textout, 
		   int *lastWasNl )
{
    char ch;
    extern const char *GetCurrentRoutinename( void );
    extern const char *GetCurrentFileName( void );
    
    if (ins->GetChar( &ch )) return 1;
    switch (ch) {
	case '\t':
	case ' ': ins->UngetChar( ' ' );
		  return ProcessArgFmt( arg_kind, ins, textout, lastWasNl );
	case 'N': return ProcessNameFmt( ins, textout, lastWasNl );
        case 'n': textout->PutOp( "linebreak" );
	  break;
        case 'v': return ProcessVerbatimBlockFmt( ins, textout, lastWasNl );
        case 'p': return ProcessPicture( ins, textout, lastWasNl );
        case 'c': return ProcessCaption( ins, textout, lastWasNl );
        case 'k': return ProcessKeywords( ins, textout, lastWasNl );
        case 's': return ProcessSeeAlso( ins, textout, lastWasNl );
        case 'f': return ProcessUserCmd( ins, textout, lastWasNl );
        case 'E': return ProcessEnumerate( ins, textout, lastWasNl );
        case 'B': return ProcessBlockOp( ins, textout, lastWasNl );
        case 'i': return ProcessItemize( ins, textout, lastWasNl );
    
        default:
    	      fprintf( stderr, "Unknown dot command `%c' in file %s(%s)\n", 
		       ch, GetCurrentFileName(), GetCurrentRoutinename() );
    }
    return 0;    
}
