/* -*- Mode: C++; c-basic-offset:4 ; -*- */
/*
    This file contains code to process various formatting commands
    that are indicated by a "." in the first column

    Currently, we support
    . (. space)   For arguments
    .I            For arguments; add a index entry to the routine (for
                  all arguments in the same list, as in +I)
    + and -(space)for argument lists (beginning and end)
    +I            Like .I, but for the beginning of the list.  I is applied
                  to every argument
    .\t           For arguments (\t is tab)
    .N name       Insert the previously defined name block (*N name ... N*)
    .n            New line at here
    .vb           Begin verbatim (i.e., LaTeX verbatim or HTML PRE)
    .ve           End verbatim
    .p filename   Picture (PS files)
    .cb           Begin picture caption
    .ce           End picture caption
    .f            Execute user-defined function (e.g., .fin, .fout)
    .i            item (like a \item in LaTeX)

    Adding now
    .Es           Begin enumerate
    .Ee           End enumerate
    .Bqs          Begin block quote
    .Bqe          End block quote

    To be added:
    .c citeref    Citation reference (the goal is to convert this into a
                  link to the reference)
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

#define MAX_ARG_NAME 255
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
 *
 * This routine processes the entire entry - the name and the description
 *
 * ToDo: consider this option for simplifying the use of +...- in argument
 * lists. The variable OldArgList allows "." for multiple args, giving the
 * older behavior (though with some improved error checks)
 * LastCmdArg is a shorthand for LastCmdSeen==ARGUMENT
 *
 *  command    InArgList    Operations
 *  +          false        s_arg_list; s_arg_inlist; InArgList:=true
 *  +          true         s_arg_inlist; warning message
 *  space      false        if (OldArgList) s_arg
 *                          else
 *                             s_arg_list; s_arg_inlist; e_arg_list;
 *                             if (LastCmdArg) warning message
 *  space      true         s_arg_inlist
 *  -          false        s_arg_list ; s_arg_inlist; e_arg_list;
 *                            if (OldArgList) warning message
 *  -          true         s_arg_inlist; e_arg_list; InArgList:=false
 *
 */
int ProcessArgFmt( char arg_kind, InStream *ins, TextOut *textout,
		   int *lastWasNl )
{
    char ch;
    int  at_first = 1;
    int  last_was_backslash=0;
    int  sawhyphen=0;
    char argname[MAX_ARG_NAME+1], *p=argname;

    SkipWhite( ins );

#if 0
    switch (arg_kind) {
    case ARGUMENT_BEGIN:
      InArgList = 1;
      textout->PutOp( "s_arg_list" );
      /* No break; we fall through to the end */
    case ARGUMENT:
    case ARGUMENT_END:
      if (InArgList)
	textout->PutOp( "s_arg_inlist" );
        CntArgList++;
      else
	textout->PutOp( "s_arg" );
      break;
    }
#else
    // Generate the initial commands before the argument name
    if (arg_kind == ARGUMENT_BEGIN) {
	if (InArgList) {
	    fprintf(stderr,
		    "WARNING (%s): '+' command seen within list of arguments for %s\n",
		    GetCurrentInputFileName(), GetCurrentRoutinename());
	}
	else {
	    textout->PutOp( "s_arg_list" );
	    InArgList =1 ;
	}
	textout->PutOp("s_arg_inlist");
        CntArgList++;
    }
    else if (arg_kind == ARGUMENT_END) {
	if (!InArgList) {
	    textout->PutOp( "s_arg_list" );
	    InArgList = 1;
	}
	textout->PutOp( "s_arg_inlist" );
        CntArgList++;
    }
    else if (arg_kind == ARGUMENT) {
	if (!InArgList && !OldArgList) {
	    if (LastCmdArg == ARGUMENT) {
		fprintf(stderr,
		"WARNING (%s): multiple '. ' commands without '+' to start list in %s\n",
			GetCurrentInputFileName(), GetCurrentRoutinename());
	    }
	    textout->PutOp( "s_arg_list" );
	    arg_kind = ARGUMENT_END;     // act as if this is a lone end-arg
	    InArgList = 1;
	}
	if (InArgList) {
	    textout->PutOp( "s_arg_inlist" );
            CntArgList++;
	}
	else {
	    textout->PutOp( "s_arg" );
	}
    }

#endif

    // Get the name. Detect the case of no hyphen - this is a name with
    // no description
    last_was_backslash = 0;
    sawhyphen          = 0;
    while ( !ins->GetChar( &ch ) ) {
	if (ch == '\n' || (ch == '-' && !at_first && !last_was_backslash)) {
	    *lastWasNl = 1;
	    if (ch == '-') sawhyphen = 1;
	    break;
	}
	if (last_was_backslash && ch != '-') {
	  textout->PutChar( '\\' );
	}
	last_was_backslash = (ch == '\\');
	// Keep a copy of the argument
	if (p - argname < MAX_ARG_NAME)
	    *p++ = ch;
	if (!last_was_backslash) {
	  // We don't output the backslash until we know if it is an escape
	  textout->PutChar( ch );
	}
	at_first = 0;
    }
    p--;                                     // Move p back to last character
    while (p > argname && isspace(*p)) p--;  // skip back over white space
    *++p = 0;                                // We always have room for the NULL
    // If this arg is indexed, and we are generating HTML, we
    // should generate and HTML anchor to use in the index.
    if (indexArgsPut(argname)) {
	textout->PutOp("anchor", argname);
    }

    // If we did not see a hyphen, we have a choice:
    //   There is no description - we need to end the item here
    //   There is a following description on the next line.  In that
    //   case, the first item must not be a command - . or + or -
    if (!sawhyphen) {
	char c1;
	// Peek at the next character.  Continuation lines must not start
	// with a command character or a blank line.
	ins->GetChar(&c1);
	ins->UngetChar(c1);
	if (c1 == ARGUMENT || c1 == ARGUMENT_END || c1 == ARGUMENT_BEGIN ||
	    c1 == '\n') {
	    if (warningNoArgDesc) {
		fprintf(stderr,
		    "WARNING (%s): argument with no description in %s\n",
		    GetCurrentInputFileName(), GetCurrentRoutinename());
	    }
	    if (InArgList) {
		textout->PutOp( "e_arg_inlist" );
	    }
	    else {
		textout->PutOp( "e_arg" );
	    }
	    if (c1 == '\n' && InArgList) arg_kind = ARGUMENT_END;

	    // Handle end-of-list
	    if (arg_kind == ARGUMENT_END) {
		textout->PutOp( "e_arg_list" );
		InArgList = 0;
		indexArgsSet(0);    // See handling at the end of the routine
	    }
	    return 0;
	}
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
      indexArgsSet(0);    // We will also need to set this to 0 for change
                          // in formatting for lists that don't start with
                          // the ARGUMENT_BEGIN string
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

    /* Raw mode output. */
    if (ins->GetChar( &ch )) return 1;
    /* FIXME: This is probably not exactly correct for NROFF output */
    if (outFormat != FMT_NROFF) {
	if (*lastWasNl)
	    textout->PutOp( "s_doctext_verbatim" );
	if (! (ch == ' ' || ch == '\t' || ch == '\n')) {
	    textout->PutChar( ch );
	}
    }
    else {
	/* This is a special case for nroff output */
	if (ch == ' ' || ch == '\t' || ch == '\n') {
	    if (*lastWasNl)
		textout->PutOp( "s_doctext_verbatim" );
	}
	else {
	    textout->PutChar( '.' );
	    textout->PutChar( ch );
	}
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
		 GetCurrentInputFileName() );
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
	     GetCurrentInputFileName() );
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
		 GetCurrentInputFileName() );
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
		 GetCurrentInputFileName() );
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
		 GetCurrentInputFileName() );
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
		 GetCurrentInputFileName() );
	return 1;
	}
    return 0;
}

int ProcessItemize( InStream *ins, TextOut *textout, int *lastWasNl )
{
    textout->PutOp( "itemize_enum" );
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
	       GetCurrentInputFileName() );
      return 1;
    }
    if (ins->GetChar( &ch )) return 1;
    if (ch == 's')      textout->PutOp( "s_blockquote" );
    else if (ch == 'e') textout->PutOp( "e_blockquote" );
    else {
	fprintf( stderr, "Invalid block command '.Bq%c' in %s\n", ch,
		 GetCurrentInputFileName() );
	return 1;
	}
    return 0;
}

// Process after a "." command
int ProcessDotFmt( char arg_kind, InStream *ins, TextOut *textout,
		   int *lastWasNl )
{
    char ch;

    if (ins->GetChar( &ch )) return 1;
    switch (ch) {
        case 'I':
	          indexArgsSet(1);
		  return ProcessArgFmt( arg_kind, ins, textout, lastWasNl );
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
		       ch, GetCurrentInputFileName(), GetCurrentRoutinename() );
    }
    return 0;
}
