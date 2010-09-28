#include <stdio.h>
#include <ctype.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"

/*
   This file contains some simple routines to manage math mode, particularly
   when using the TeX $...$ and $$ ... $$

   It really needs to be more aggressive.  It should process every command
   that it can (so that it handles user-defined commands), but remember if it
   encountered any that it could not process.  In that case, the option of
   processing the ORIGINAL TeX into a bitmap may be taken.

   In particular, this will allow us to process
        \begin{displaymath}...\end{displaymath}, since the effect of begin
   is to get us started, and the \end will invoke a replacement that 
   places \] (or $$) on the input.
 */

#define MATH_BUF_SIZE 4*4096
/* Forward refs */
void TXStripCmds ( char *, char * );
void TXAddToken ( char *, char **, int * );
void TXConvertMathString ( char **, char **, int *, int );

static int Debug_math_mode = 0;

/* Removes all \ from a string */
void TXStripCmds( char *str, char *strout )
{
    while (*str) {
	if (*str == CommandChar) str++;
	else *strout++ = *str++;
    }
    *strout = 0;
}

void TXAddToken( char *tok, char **outstrp, int *maxstrp )
{
    int maxstr = *maxstrp;
    char *outstr = *outstrp;

    /* if (1) printf( "Adding token %s\n", tok ); */
    *outstr++ = TOK_START;
    maxstr--;
    while (*tok && maxstr > 0) {
	*outstr++ = *tok++;
	maxstr--;
    }
    *outstr++ = TOK_END;
    maxstr--;

    *outstrp = outstr;
    *maxstrp = maxstr;
}

/*
 * Remove Convert \ in front of []{}(), remove {}, and process _^ for HTML
 * (if not HTML, won't be in this routine).  Note that it is recursive 
 * for things like a^{i_0}.
 *
 * instr is defined as EITHER null terminated or { ... }.  The flag
 * matched_brace, if true, terminates when either a matched closing brace
 * or a single character is seen (this matches TeX's argument processing when
 * this is used to process a math argument, such as to ^ or _).
 */
void TXConvertMathString( char **instrp, char **outstrp, int *maxstrp, 
			  int matched_brace )
{
    char *instr = *instrp, *outstr = *outstrp;
    int  maxstr = *maxstrp;
    int  bracecnt = 0;
    int  fontstate[100];
#define FONT_ITALICS 1
#define FONT_ROMAN   2
#define FONT_BOLD    3
#define FONT_FIXED   4

    if (matched_brace) {
	if (*instr != '{') {
	    /* Single character case */
	    /* The ^ or _ are erroneous, as are commands, at least for now */
	    *outstr++ = *instr++;
	    *outstr   = 0;
	    maxstr--;
	    *outstrp = outstr;
	    *maxstrp = maxstr;
	    *instrp  = instr;
	    return;
	}
	else {
	    instr++;
	    bracecnt = 1;
	}
    }
    while (*instr && maxstr) {
	if (*instr == CommandChar) {
	    instr++;
	    /* Process known commands: \{, \}, \_, \ldots \cdot \cdots*/
	    if (*instr == '{' || *instr == '}' || *instr == '_') {
		/* Accept next character (things like \{, \}, \_) */
		*outstr++ = *instr++;
		maxstr--;
	    }
	    else {
		char tmpname[100], *p = tmpname;
		while (isalpha(*instr)) *p++ = *instr++;
		*p++ = 0;
		if (strcmp( tmpname, "ldots" ) == 0) {
		    TXAddToken( "...", &outstr, &maxstr );
		}
		else if (strcmp( tmpname, "cdot" ) == 0) 
		    TXAddToken( "&#183;", &outstr, &maxstr );
		else if (strcmp( tmpname, "cdots" ) == 0) 
		    TXAddToken( "&#183;&#183;&#183;", &outstr, &maxstr );
		else if (strcmp( tmpname, "times" ) == 0) 
		    TXAddToken( "&#215;", &outstr, &maxstr );
		else if (strcmp( tmpname, "circ" ) == 0)
		    TXAddToken( "&#176;", &outstr, &maxstr );
		else if (strcmp( tmpname, "_" ) == 0) 
		    TXAddToken( "_", &outstr, &maxstr );
		/* or better is &sdot; once HTML 4 is accepted */
		else if (strcmp( tmpname, "sf" ) == 0) {
		    /* if (fontstate[bracecnt] == FONT_ITALICS)
		    TXAddToken( "</I>", &outstr, &maxstr ); */
		    fontstate[bracecnt] = FONT_ROMAN;
		}
		else if (strcmp( tmpname, "tt" ) == 0) {
		    fontstate[bracecnt] = FONT_FIXED;
		}
		else {
		    /* This shouldn't happen */
		    p = tmpname;
		    while (*p && maxstr > 0) {
			*outstr++ = *p++; 
			maxstr--;
		    }
		}
	    }
	}
	/* bare braces are ignored but counted */
	else if (*instr == '{') {
	    instr++;
	    bracecnt++;
	    fontstate[bracecnt] = fontstate[bracecnt-1];
	}
	else if (*instr == '}') {
	    instr++;
	    bracecnt--;
	    /* Handle pop from font (need a stack associated with the 
	       bracecount) */
	    if (fontstate[bracecnt] != fontstate[bracecnt+1]) {
		/* Not done yet */
		;
	    }
	    if (matched_brace && bracecnt == 0) break;
	}
	else if (*instr == SuperscriptChar) {
	    /* Superscript in HTML3 */
	    if (DebugCommands) {
		printf( "Processing supercript in math mode\n" );
	    }
	    TXAddToken( "<SUP>", &outstr, &maxstr );
	    instr++;
	    TXConvertMathString( &instr, &outstr, &maxstr, 1 );
	    TXAddToken( "</SUP>", &outstr, &maxstr );
	}
	else if (*instr == SubscriptChar) {
	    if (DebugCommands) {
		printf( "Processing subscript in math mode\n" );
	    }
	    /* Subscript in HTML3 */
	    TXAddToken( "<SUB>", &outstr, &maxstr );
	    instr++;
	    TXConvertMathString( &instr, &outstr, &maxstr, 1 );
	    TXAddToken( "</SUB>", &outstr, &maxstr );
	}
	else {
	    *outstr++ = *instr++;
	    maxstr--;
	}
    }
    *instrp  = instr;
    *outstrp = outstr;
    *outstr  = 0;
    *maxstrp = maxstr;
}

/* 
   Skip math-mode text, using EITHER $, \], or \) as the ending token.
   Name is the OPENING token.

   We need a version that can use SimpleMath and output the stuff into
   a data area so that we can avoid processing as LaTeX when it is
   something as innocuous as Ax = b.
 */
void TeXskipMath( TeXEntry *e, char *name, int doout )
{
    int  nsp, ch;
    FILE *fout = fpout;
    int  (*oldtrans)( char *, int );

    /* First, remove any newlines */
    SCSkipNewlines( fpin[curfile] );

    oldtrans = SCSetTranslate( (int (*)(char *, int ))0 );

    if (name[0] != MathmodeChar && name[1] != '(' && name[1] != '[') {
	NumberedEnvironmentType = ENV_EQUATION;
	TeXSetEnvJump( "Equation" );
	TXcaptionHandling( e );
    }

    if (DebugCommands) 
	fprintf( stdout, "Processing environment %s\n", name );
    while (1) {
	while ( (ch = TeXReadToken( curtok, &nsp )) == EOF) {
	    if (DebugCommands) 
		fprintf( stdout, "EOF in TeXskipEnv\n" );
	    /* Special case for vtt */
	    if (curfile == 0 && strcmp( name, "_vtt" ) == 0) {
		break;
	    }
	    TXPopFile();
	}
	if (ch == EOF) break;
	if (name[0] == MathmodeChar && ch == MathmodeChar) break;
	if (ch == CommandChar) {
	    if (doout)
		TeXoutsp( fout, nsp );
	    ch = curtok[1];
	    if (name[1] == '[' && ch == ']') {
		break;
	    }
	    else if (name[1] == '(' && ch == ')') {
		break;
	    }
	    else if (strcmp( curtok, "\\label" ) == 0) {
		/* Need to process a label command */
		TXlabel( e );
	    }
	    else {
		if (doout) {
		    TeXoutstr( fout, curtok );
		}
	    }
	}
	else {
	    if (doout) {
		TeXoutsp( fout, nsp );
		TeXoutstr( fout, curtok );
	    }
        }
    }
    NumberedEnvironmentType = ENV_NONE;

/* Skip the newlines and the end of the environment */
    SCSetTranslate( oldtrans );
    SCSkipNewlines( fpin[curfile] );
    if (DebugCommands) 
	fprintf( stdout, "Done with environment %s\n", name );
}

/*
 * Process math mode from the input.  May use latexify to handle equations,
 * but can also try to keep things simple by converting into simpler
 * text.
 *
 * We should handle a few more commands.  For example, turning \ldots
 * into ... or \, into <nop> (\, is a spacing command).
 */
void TXProcessDollar( TeXEntry *e, int latexmath, int checkdollar )
{
    int ch, nsp;
    int displaymode = 0;
    int hasCommand = 0;
    int numCommands = 0;
    int numDoableCommands = 0;
    int hasBackwack = 0;
    int i;
    char *mathbuf;
    int  mathbufleft;
    char token[MAX_TOKEN];
    int num_subs = 0;
    int (*transold)( char *, int);

    mathbuf = (char *)MALLOC( MATH_BUF_SIZE );   CHKPTR(mathbuf);
    mathbufleft = MATH_BUF_SIZE - 1;
/* Try to find next MathmodeChar (usually $) */

    transold = SCSetTranslate( (int (*)(char *, int ))0 );
    NumberedEnvironmentType = ENV_EQUATION;
    TeXSetEnvJump( "Equation" );

    if (checkdollar) {
	ch = SCTxtFindNextANToken( fpin[curfile], token, MAX_TOKEN, &nsp );

	if (ch == MathmodeChar) {
	    displaymode = 1;
	    strcpy( mathbuf, "\\[" );
	    /* 4 because we're reserving 2 for the closing marker */
	    mathbufleft -= 4;
	}
	else {
	    SCPushToken( token );
	    strcpy( mathbuf, "\\(" );
	    /* 4 because we're reserving 2 for the closing marker */
	    mathbufleft -= 4;
	}
    }
    else {
	strcpy( mathbuf, "\\begin{displaymath}" );
	mathbufleft -= strlen(mathbuf);  /* This is enough for the end */
    }
    while (mathbufleft > 0) {
	ch = SCTxtFindNextANToken( fpin[curfile], token, MAX_TOKEN, &nsp );
	if (ch == EOF) {
	    TXPopFile();
	    continue;
	}
	if (ch == MathmodeChar) {
	    /* Check for end of math mode */
	    if (!displaymode) break;
	    else {
		ch = SCTxtGetChar( fpin[curfile] );
		if (ch == MathmodeChar) break;
		SCPushChar( ch );
		ch = MathmodeChar;
	    }
	}
	/* HTML 3 has sub and sup */
	if (ch == SuperscriptChar || ch == SubscriptChar) {
	    num_subs ++;
	    if (!HTMLv3) {
		hasCommand = 1;
		numCommands++;
	    }
	}

	/* If the previous character was a \ (command char), use 
  	   special processing for this character */
	if (hasBackwack) {
	    /* Check for \] or \) */
	    if (token[0] == ']' || token[0] == ')') break;
	    /* We'd like to change \{ \} to {} without requiring math mode;
	       ditto for \_ */
	    if (token[0] != '{' &&
		token[0] != '}' && token[0] != '_') {
		hasCommand = 1;
		numCommands++;
	    }
	    if (strcmp( token, "label" ) == 0) {
		/* Need to process a label command */
		/* Note that a label here, particularly if !displaymode, is a 
		   LATEX error */
		TXlabel( e );
		/* Add a dummy incase this is the only thing on a line */
		mathbufleft -= 13;
		if (mathbufleft < 0) break;
		strcat( mathbuf, "label{eq-foo}" );
		numDoableCommands++;
	    }
	    else if (strcmp( token, "end" ) == 0) {
		/* Look for displaymath */
		if (TeXGetArg( fpin[curfile], curtok, MAX_TOKEN ) == -1)
		    TeXAbort( "TXProcessDollar", 0 );
		else if (strcmp( curtok, "displaymath" ) == 0) {
		    numDoableCommands++;
		    break;
		}
		else {
		    /* Restore the token that we read */
		    SCPushToken( "}" );
		    SCPushToken( curtok );
		    SCPushToken( "{" );
		}
	    }
	    /* Here we could handle \ldots and some others ... */
	    if (strcmp( token, "ldots" ) == 0)
		numDoableCommands++;
	    else if (strcmp( token, "sf" ) == 0) 
		numDoableCommands++;
	    else if (strcmp( token, "tt" ) == 0) 
		numDoableCommands++;
	    else if (strcmp( token, "_" ) == 0) 
		numDoableCommands++;
	    else if (strcmp( token, "cdot" ) == 0) 
		numDoableCommands++;
	    else if (strcmp( token, "cdots" ) == 0) 
		numDoableCommands++;
	    else if (strcmp( token, "times" ) == 0) 
		numDoableCommands++;
	    else if (strcmp( token, "circ" ) == 0) 
		numDoableCommands++;
	    /*	    else if (strcmp( token, "{" ) == 0 ||
		     strcmp( token, "}" ) == 0) 
		     numDoableCommands++; */
	} /* Check for commandchar (hasBackwhack) */
	else if (ch == CommentChar) {
	    /* If the previous character was not a command char, then
	       this really is a comment, and we should skip to the
	       end of the line without processing it */
	    SCTxtDiscardToEndOfLine( fpin[curfile] );
	    LineNo[curfile]++;
	    continue;
	}

	/* Remember if the PREVIOUS character was a CommandChar */
	hasBackwack = (ch == CommandChar);
    
	/* copy to a buffer */
	/* decrement length left FIRST to avoid memory overwrites */
	mathbufleft -= nsp;
	mathbufleft -= strlen(token);
	if (mathbufleft < 0) break;

	for (i=0; i<nsp; i++) 
	    strcat( mathbuf, " " );
	strcat( mathbuf, token );
    }

    if (mathbufleft < 0) {
	fprintf( stderr, "Buffer overrun while processing math mode!\n" );
	mathbuf[MATH_BUF_SIZE-1] = 0;
    }

    SCSetTranslate( transold );
    NumberedEnvironmentType = ENV_NONE;

/* If hasCommand is false, we can skip the latexmath step... */
    if (latexmath && hasCommand && numCommands > numDoableCommands) {
	/* Eventually needs to handle LatexAgain ... */
	char bname[100]; 
	char fname[100];

	sprintf( bname, "img%d", imageno++ );
	strncpy( fname, bname, 100 );
	strncat( fname, ".", 100 );
	strncat( fname, ImageExt, 100 );
	if (!hasBackwack) strcat( mathbuf, "\\" );
	if (checkdollar) {
	    if (displaymode)
		strcat( mathbuf, "]" );
	    else 
		strcat( mathbuf, ")" );
	}
	else 
	    strcat( mathbuf, "end{displaymath}" );

	if (Debug_math_mode) {
	    printf( "Processing |%s| for math with latex\n", mathbuf );
	}

	if (LatexAgain || !FileExists( fname )) {
	    RunLatex( (char *)0, mathbuf, bname, (char *)0, ImageExt, 1 );
	}
	if (displaymode)
	    TXimage( e, fname );
	else  /* \( ... \)  */
	    TXInlineImage( e, fname );
    }
    else {
	/* Need to remove and \{  or \} that we let through... */
	char *strout;
	int  maxstr;

	if (Debug_math_mode) {
	    printf( "Processing |%s| for math inline\n", mathbuf );
	}
	maxstr = strlen(mathbuf + 2) + 1;

	/* add enough from a number of sub/super scripts */
	maxstr += (1+num_subs)*15;
	strout = (char *)MALLOC( maxstr );
	if (!strout) {
	    fprintf( stderr, "Out of memory while processing math mode!\n" );
	    return;
	}
	/* TXStripCmds( mathbuf + 2, strout ); */
	/* This routine also handles ^ and _ */
	{ 
	    char *mathbufp, *outstrp = strout;
	    mathbufp = mathbuf;
	    if (checkdollar) mathbufp += 2;
	    else mathbufp += strlen("\\begin{displaymath}");
	    TXConvertMathString( &mathbufp, &outstrp, &maxstr, 0 );
	}
	if (displaymode) {
	    TXmath( e );
	    TeXoutstr( fpout, strout );
	    TXmathend( e );
	}
	else {
	    TXinlinemath( e );
	    TeXoutstr( fpout, strout );
	    TXinlinemathend( e );
	}
	FREE( strout );
    }
    FREE( mathbuf );
}

