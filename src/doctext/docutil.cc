#include "textout.h"
#include "tfile.h"
#include "doc.h"
#include "docutil.h"
#include <ctype.h>
#include <string.h>

char LeadingString[10] = "";
int debugBreakStack = 0;

#define MAX_TOKEN_SIZE 255

static int  skipIgnore(char *token, InStream *ins);
static void skipBlanks(InStream *ins);

static int outToken(int nsp, char *token, TextOut *outs);
static void pushBreakchars(InStream *ins);
static void popBreakchars(InStream *ins);
int ReadAndOutputCalllist(InStream *ins, TextOut *outs);
int ReadAndOutputInteger(char fchar, InStream *ins, TextOut *outs,
			 char *nchar);
int ReadAndOutputQuotedString(InStream *ins, TextOut *outs,
			      char quotechar, char escchar);
int GetANTokenSkipComments(InStream *ins, TextOut *outs,
			   int maxlen, char *token, int *nsp);
int SkipPossibleComment(InStream *ins, TextOut *outs);

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

// If flag is true, output the "-"
int DocReadDescription( InStream *ins, char *matchstring,
			TextOut *textout, int flag, int *at_end )
{
    char ch;
    int  newline_cnt;
    int  state, i;
    int  b_state = (int)strlen(matchstring) - 1;

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
    int b_state = (int)strlen(matchstring) - 1;

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

int numcomma = -2;
//
// Read the synopsis and send to the out stream.  Remove "register"
// if is appears.
// Also strip any ignore tokens.
//
int DocReadFuncSynopsis( InStream *ins, TextOut /*OutStream */ *outs)
{
    char token[MAX_TOKEN_SIZE+1];
    int  maxlen = MAX_TOKEN_SIZE, nsp;
    int  findingForm = 1;
    int  exitOnSemicolon = 0;
    int  parenCount = 0;
    int  justentered = 0, voidfirst = 0;

    numcomma = -2;
    // Must handle newline as non-space
    pushBreakchars(ins);
    if (debugBreakStack) {
	fprintf(stderr, "Push in DocReadFuncSynopsis\n");
    }
    // Stop when we find *either* a { or a ;
    // The semicolon test lets us handle prototype definitions
    // Note that a semicolon should stop the scanning ONLY
    // when in pre-ansi, non-prototype form.
    while (!ins->GetToken( maxlen, token, &nsp ) && token[0] != '{') {
      //	   && ( !inProtoForm || token[0] != ';' ) {
      // Eventually we need to combine these into a single lookup list.
      // Handle the special case of int foo (...);
      if (exitOnSemicolon) {
	if (token[0] == ';') break;
	exitOnSemicolon = 0;
      }

      if (strcmp( token, "register" ) == 0) {
	  skipBlanks(ins);
      }
      else if (!skipIgnore(token, ins)) {
	// Check for parenthesis to determine whether we're in
	// prototype form
        if (!strcmp(token,"void") && justentered) voidfirst = 1; else justentered = 0;
	if (token[0] == '(') {if (++parenCount == 1) justentered = 1;if (numcomma == -2) numcomma = 0;}
	else if (token[0] == ')') {
	  if (parenCount == 1) {
            if (voidfirst) numcomma = -1;
	    if (findingForm) {
	      // if the NEXT char is a ;, we have a terminal prototype
	      exitOnSemicolon = 1;
	    }
	    findingForm = 0;
	  }
	  parenCount --;
	}
        if (parenCount == 1 && token[0] == ',') {numcomma++;}
	outToken(nsp, token, outs);
      }
    }
    popBreakchars(ins);
    if (debugBreakStack) {
	fprintf(stderr, "Pop in DocReadFuncSynopsis\n");
    }
    return 0;
}

/*
   Skip to the START of a macro synopsis.  This is just skipping to
   EITHER the end (no synopsis) or finding a line containing
     Synopsis:
   on it
 */
int DocSkipToMacroSynopsis( InStream *ins, char *matchstring )
{
    int state, sy_state;
    char ch;
    static char synopsis[10] = "Synopsis:";
    int b_state = (int)strlen(matchstring) - 1;

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

/* Read (and output) until we get two newlines */
int DocReadMacroSynopsis( InStream *ins, char *matchstring,
			  TextOut /*OutStream*/ *outs, int *at_end )
{
    char ch;
    int  newline_cnt, i;
    int  state, b_state = (int)strlen(matchstring) - 1;

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

//
// Read the definition and send to the out stream.
// Also strip any ignore tokens.
// This handles
// text { text including ; } name ;
// e.g.,
//   typedef enum { foo, bar=2, last } name;
// and
//   typedef struct { int foo; char bar; } name;
//
// Eventually, we'll want to include information on the contents of the
// definition.  At the very least, we'd like a list of the names (enum) or
// fields (struct).
// Enums aren't too hard, but struct definitions can contain things like
// int (*foo)( char *(name)([][]), double (*name2)( int (*)(int,int) ) );
// See bfort for some examples.
int DocReadTypeDefinition( InStream *ins, TextOut /*OutStream*/ *outs )
{
    char token[MAX_TOKEN_SIZE+1];
    int in_brace = 0;
    int maxlen = MAX_TOKEN_SIZE, nsp;
#if 0
    void *fieldlist=0;       // Fieldlist records the entries in the
                             // name.  NOT FULLY IMPLEMENTED
#endif

    // Must handle newline as non-space
    pushBreakchars(ins);
    if (debugBreakStack) {
	fprintf(stderr, "Push in DocReadTypeDefinition\n");
    }
    while (!ins->GetToken( maxlen, token, &nsp )) {
	// Eventually we need to combine these into a single lookup list.
	if (!skipIgnore(token, ins))
	    outToken(nsp, token, outs);

	// If that was the last character, exit
	if (in_brace == 0 && token[0] == ';') {
	    break;
	}
	if (token[0] == '{') in_brace++;
	else if (token[0] == '}') in_brace--;
    }
    outs->PutToken( nsp, NewlineString );
    popBreakchars(ins);
    if (debugBreakStack) {
	fprintf(stderr, "Pop in DocReadTypeDefinition\n");
    }
    return 0;
}

int DocReadDefineDefinition( InStream *ins, TextOut *outs )
{
    char token[MAX_TOKEN_SIZE+1];
    int maxlen = MAX_TOKEN_SIZE, nsp;
    int prevnewline = 0;
#if 0
    void *fieldlist=0;       // Fieldlist records the entries in the
                             // name.  NOT FULLY IMPLEMENTED
#endif

    // Must handle newline as non-space
    pushBreakchars(ins);
    if (debugBreakStack) {
	fprintf(stderr, "Push in DocReadDefineDefinition\n");
    }
    while (!ins->GetToken( maxlen, token, &nsp )) {
	// Check for \n and do PutNewline instead.
	int sawNewline = outToken(nsp, token, outs);
	if (sawNewline && prevnewline) break;
	prevnewline = sawNewline;
    }
    popBreakchars(ins);
    if (debugBreakStack) {
	fprintf(stderr, "Pop in DocReadDefineDefinition\n");
    }
    return 0;
}

// Read an enum definition, including a optional typedef and
// optional values for the names.  E.g.,
//   typedef enum { foo, bar=2, last } name;
// Must also output the text, and generate additional links
// Note that enum names can be alphanumeric, so we need to allow
// tokens that start with alpha but include numeric characters.
//
// Here is a grammar for the enum definitions. This addresses
// the use of macros for either names or values, which we have seen
// in use by PETSc.
//
// [typedef] enum [ATTR] { NAME [=VALUE] (,NAME [=VALUE])* } name [,name]*;
//
// where NAME and VALUE may either be a simple alpha-numeric token OR
// a macro reference:
//
// NAME and VALUE:
//    ANtoken [ ( TERM [,TERM]* ) ]
// where TERM is a token that is not "," or a quoted string
#if 1
int DocReadEnumDefinition(InStream *ins, TextOut *outs)
{
    char token[MAX_TOKEN_SIZE+1];
    int  maxlen = MAX_TOKEN_SIZE, nsp;
    int  isEnum = 1;      // Used to check if recognized as enum or typedef enum
    enum { get_name,
	   get_call,
	   get_val
    } state;

    // Must handle newline as non-space
    pushBreakchars(ins);
    if (debugBreakStack) {
	fprintf(stderr, "Push in DocReadEnumDefinition\n");
    }

    if (verbose) printf("Reading enum definition\n");
    // Skip any initial newlines
    while (!GetANTokenSkipComments(ins, outs, maxlen, token, &nsp)) {
	if (token[0] != '\n') break;
	if (verbose) printf("Read token :%s:\n",token);
    }

    // Look for a leading [typedef] enum.  Return the first token after
    // enum, or first token that doesn't match
    if (strcmp(token, "typedef") == 0) {
	if (verbose) printf("Saw typedef in enum doc\n");
	outs->PutToken(nsp, token);
	if (GetANTokenSkipComments(ins, outs, maxlen, token, &nsp)) {
	    isEnum = 0;
	}
    }
    if (strcmp(token, "enum") != 0) isEnum = 0;
    else {
	if (verbose) printf("Saw enum in enum doc\n");
	outs->PutToken(nsp, token);
	GetANTokenSkipComments(ins, outs, maxlen, token, &nsp);
    }

    // Look for {, skipping any comments (or attributes)
    if (token[0] != '{') {
        int  num_attempts = 0;
        char backup_token[sizeof(token)];

        // save token for the error message, since we are about to overwrite it
        memcpy(backup_token, token, sizeof(token));
        do {
            GetANTokenSkipComments(ins, outs, maxlen, token, &nsp);
            // a back door in case of malformed text
            if (++num_attempts > 100) {
                fprintf(stderr, "Expected { to start enum definition, saw %s\n",
                        backup_token);
                popBreakchars(ins);   // Restore break table
                return 1;
            }
        } while (token[0] != '{');
    }
    outs->PutToken(nsp, token);

    // process an enum list:
    //  NAME ( CALLLIST ) ( = EXPR )
    state = get_name;
    char *enumname = 0;
    while (!GetANTokenSkipComments(ins, outs, maxlen, token, &nsp)) {
	if (verbose) printf("processing token :%s:\n", token);
	// Ignore newlines
	if (token[0] == '\n') {
	    outToken(nsp, token, outs);
	    continue;
	}
	// Record the enumname if we've reached the end of the definition
	if (enumname && (token[0] == ',' || token[0] == '}')) {
	    IndexFileAdd(enumname, enumname, GetCurrentFileName(),
			 GetCurrentRoutinename());
	    delete(enumname);
	    enumname = 0;
	}

	if (token[0] == '}') {
	    outToken(nsp, token, outs);
	    break;
	}
	switch (state) {
	case get_name:
	    outToken(nsp, token, outs);
	    // Save a copy of the name
	    enumname = strdup(token);
	    state = get_call;
	    break;
	case get_call:
	    outToken(nsp, token, outs);
	    if (token[0] == ',') {
		state = get_name;
	    }
	    else if (token[0] == '=') {
		state = get_val;
	    }
	    else if (token[0] == '(') {
		// A complex name - the enumname is not valid
		if (enumname) { delete(enumname); enumname = 0; }
		ReadAndOutputCalllist(ins, outs);
		// stay in get call - look for , or =
	    }
	    else {
		fprintf(stderr, "Unexpected token in processing enum: %s\n",
			token);
		// how to break out?
	    }
	    break;
	case get_val:
	    // allow general expressions, including a call
	    outToken(nsp, token, outs);
	    if (token[0] == ',') {
		state = get_name;
	    }
	    else if (token[0] == '(') {
		ReadAndOutputCalllist(ins, outs);
	    }
	    else {
		// Allow almost anything - state unchanged
	    }
	    break;
	}
    }
    if (enumname) delete(enumname);

    // Still need to see if there is a name for the enum. Process
    // until the terminating ;
    while (!GetANTokenSkipComments(ins, outs, maxlen, token, &nsp)) {
	outToken(nsp, token, outs);
	if (token[0] == ';') break;
    }


    outs->PutToken(nsp, NewlineString);
    popBreakchars(ins);
    if (debugBreakStack) {
	fprintf(stderr, "Pop in DocReadEnumDefinition\n");
    }
    return 0;
}
#else
int DocReadEnumDefinition(InStream *ins, TextOut *outs)
{
    char ch;
    char token[MAX_TOKEN_SIZE+1];
    int  in_brace = 0;
    int  maxlen = MAX_TOKEN_SIZE, nsp;
    int  isEnum = 1;      // Used to check if recognized as enum or typedef enum
    enum { nxt_val,
	   has_name,
	   has_val,
	   nxt_comment,
	   in_c_cmt,
	   in_c_cmt_end
    } state, oldstate;

    // Must handle newline as non-space
    pushBreakchars(ins);

    if (verbose) printf("Reading enum definition\n");
    // Skip any initial newlines
    while (!ins->GetToken(maxlen, token, &nsp)) {
	if (token[0] != '\n') break;
	if (verbose) printf("Read token :%s:\n",token);
    }
    // Look for a leading [typedef] enum.  Return the first token after
    // enum, or first token that doesn't match
    if (strcmp(token, "typedef") == 0) {
	if (verbose) printf("Saw typedef in enum doc\n");
	outs->PutToken(nsp, token);
	if (ins->GetToken(maxlen, token, &nsp)) { isEnum = 0; }
    }
    if (strcmp(token, "enum") != 0) isEnum = 0;
    else {
	if (verbose) printf("Saw enum in enum doc\n");
	outs->PutToken(nsp, token);
	ins->GetToken(maxlen, token, &nsp);
    }

    // We don't start with GetANToken because we must first see an
    // open brace
    // Now have first token that is not typedef enum
    state = nxt_val;
    do {
	if (verbose) printf("processing token :%s:\n",token);

	// Eventually we need to combine these into a single lookup list.
	if (!skipIgnore(token, ins))
	    outToken(nsp, token, outs);

	// Must first check for processing inside of a C comment
	if (state == in_c_cmt) {
	    if (token[0] == '*') state = in_c_cmt_end;
	    continue;
	}
	else if (state == in_c_cmt_end) {
	    if (token[0] == '/') state = oldstate;
	    else state = in_c_cmt;
	    continue;
	}

	// If that was the last character, exit
	if (in_brace == 0 && token[0] == ';') {
	    break;
	}
	if (token[0] == '{') in_brace++;
	else if (token[0] == '}') in_brace--;
	else if (token[0] == ',') state=nxt_val;
	else if (token[0] == '/') {
	    if (state != nxt_comment) {
		oldstate = state;
		state=nxt_comment;
	    }
	    else {
		// saw a // - skip to end of line
		char c;
		while (!ins->GetChar(&c)) {
		    outs->PutChar(c);
		    if (c == '\n') break;
		}
		state = oldstate;
	    }
	}
	else if (in_brace == 1 &&
		 token[0] != '\n' && token[0] != ',' && isEnum) {
	    switch (state) {
	    case nxt_val:
		state=has_name;
		if (verbose) printf("enum name = %s\n", token);
		// possible enum name; format is name [=value], ...
		IndexFileAdd(token, token, GetCurrentFileName(),
			     GetCurrentRoutinename());
		break;
	    case has_name:
		if (token[0] == '=') state=has_val; else state=nxt_val;
		break;
	    case has_val:
		// Allow sign in enum
		if (token[0] == '+' || token[0] == '-') {
		    if (verbose) printf("enum sign is %s,", token);
		}
		else {
		    state=nxt_val;
		    if (verbose) printf("enum value is %s\n", token);
		}
		break;
	    case nxt_comment:
		if (token[0] == '*') {
		    // Found a C comment - read until closing */
		    state = in_c_cmt;
		}
		else  // Did not start a C comment
		    state = oldstate;
		break;
	    default: break;
	    }
	}
    } while (!ins->GetANToken(maxlen, token, &nsp));

    outs->PutToken(nsp, NewlineString);
    popBreakchars(ins);
    return 0;
}
#endif

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
#if 0
int DocSubOption( int *hasx, int *hasc )
{
    if (hasx) *hasx = HasX;
    if (hasc) *hasc = HasC;
    return 0;
}
#endif

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

// ---------------------------------------------------------------------------
// Skip a C comment, given that the leading / has been read.
// This routine reads the next character and either processes the comemnt
// (if the character is either * (/*) or / (//)) or ungets the character
// and returns.  The return values are:
//   0: Success and comment read
//   1: The leading / did not begin a comment
//   <0: Error, such as EOF on input.
// If outs is provided, output (not just skip) the comment.
int SkipPossibleComment(InStream *ins, TextOut *outs)
{
    char c, nchar;
    // Check next character
    if (ins->GetChar(&nchar)) return -1;

    if (nchar == '*') {
	if (outs) outs->PutChar(nchar);
	// delimited comment
	while (!ins->GetChar(&c)) {
	    if (outs) outs->PutChar(c);
	    if (c == '*') {
		if (ins->GetChar(&nchar)) return -11;
		if (nchar == '/') {
		    if (outs) outs->PutChar(nchar);
		    break;
		}
		ins->UngetChar(nchar);
	    }
	}
	return 0;
    }

    if (nchar == '/') {
	// comment to eol
	// NOTE: does not handle a \ followed by EOL.
	if (outs) outs->PutChar(nchar);
	while (!ins->GetChar(&c)) {
	    if (c == '\n') {
		ins->UngetChar(c);
		break;
	    }
	    if (outs) outs->PutChar(c);
	}
	return 0;
    }

    ins->UngetChar(nchar);
    return 1;  // Comment not found
}

// ---------------------------------------------------------------------------
// Get an AN token, skipping any preceeding comments
int GetANTokenSkipComments(InStream *ins, TextOut *outs,
			   int maxlen, char *token, int *nsp)
{
    int rct, rc;
    do {
	rct = ins->GetANToken(maxlen, token, nsp);
	if (rct) return rct;

	if (token[0] == '/') {
	    if (outs) {
		outs->PutToken(*nsp, token);
		*nsp = 0;
	    }
	    rc = SkipPossibleComment(ins, outs);
	    if (rc == 1) return 0;
	    if (rc < 0) return 1;  	    /* rc < 0 is error */
	    /* If rc == 0 saw a comment; need to read again */
	}
	else return 0;;
    } while(1);
}



// Read a quoted string.  The escape character (escchar) is used to
// include the quote or the escape character in the string.  E.g.,
//   "foo \"\"\\" is the string   foo""\ .
int ReadAndOutputQuotedString(InStream *ins, TextOut *outs,
			      char quotechar, char escchar)
{
    int inescape = 0;
    char c;

    while (!ins->GetChar(&c)) {
	if (inescape) {
	    inescape = 0;
	    outs->PutChar(c);
	}
	else if (c == escchar) {
	    inescape = 1;
	}
	else {
	    outs->PutChar(c);
	    if (c == quotechar) return 0;;
	}
    }
    // EoF or other problem before seeing end of string
    return 1;
}

#if 0
int ReadAndOutputExpr(InStream *ins, TextOut *outs)
{
    int c;

    return 1;
}
#endif

// Read just an integer.  nchar is set to the first char after the integer.
// This is a "peeked" value - the character still needs to be read.
int ReadAndOutputInteger(char fchar, InStream *ins, TextOut *outs,
			 char *nchar)
{
    char c;

    while (!ins->GetChar(&c)) {
	if (ins->breaktable[(unsigned char)c] == BREAK_DIGIT) {
	    outs->PutChar(c);
	}
	else {
	    *nchar = c;
	    ins->UngetChar(c);
	    return 0;
	}
    }
    return 1;
}

#if 0
// Read and output a number.  Both integer and floating point.
// The first digit has been read (in fchar).  This is used incase
// a later version of this will determine the value of the number

int ReadAndOutputNumber(char fchar, InStream *ins, TextOut *outs)
{
    int  err;
    char c, nchar;

    err = ReadAndOutputInteger(fchar, ins, outs, &nchar);
    if (err) return err;
    // Check for the case where nchar is . or e or E for the formats
    // INTEGER (. INTEGER) ([eE]([+-])INTEGER)
    if (nchar == '.') {
	ins->GetChar(&c);
	outs->PutChar(c);
	ins->GetChar(&nchar);
	if (ins->breaktable[(unsigned char)nchar] == BREAK_DIGIT) {
	    err = ReadAndOutputInteger(fchar, ins, outs, &nchar);
	    if (err) return err;
	}
    }
    if (nchar == 'e' || nchar == 'E') {
	ins->GetChar(&c);
	outs->PutChar(c);
	ins->GetChar(&c);
	if (c == '-' || c == '+') {
	    outs->PutChar(c);
	    ins->GetChar(&c);
	}
	if (ins->breaktable[(unsigned char)c] == BREAK_DIGIT) {
	    err = ReadAndOutputInteger(c, ins, outs, &nchar);
	}
	else err = 1;  // Require an exponent
    }
    return err;
}
#endif

// Read a function or macro invocation and output it.  The routine or macro
// name has been read. The approximate grammar is
//
// CALLLIST = \( EXPR (, EXPR)* \)
// EXPR = TERM ( OP EXPR )
// TERM = NAME ( CALLLIST ) | QUOTEDSTRING | NUMBER
// The opening "(" has been read
int ReadAndOutputCalllist(InStream *ins, TextOut *outs)
{
    char c;
    while (!ins->GetChar(&c)) {
	if (c == '(') {
	    outs->PutChar(c);
	    ReadAndOutputCalllist(ins, outs);
	}
	else if (c == ')') {
	    outs->PutChar(c);
	    return 0;
	}
	else if (c == ',') {
	    outs->PutChar(c);
	}
    }
    return 0;
}

#ifdef FOO
//
// The following isn't used yet.  It will read a single argument or
// field definition
//
// We'll start with an enum name.  That is simply
//   name [ = int ]
// We also asume that all setup (e.g., break characters) has already been
// done.
// We may also want a special outs that is effectively /dev/null (i.e.,
// don't write anything out).
// How do we want to return the names?
// Do we want a special structure with head and tail pointers?
//
// More generally, I'd like to parse the descriptions and store them so
// that they can be further processed, for example to check that all
// arguments and fields are described or to list all possible enum values.
// Once we have this information, doctext can also produce the Fortran
// and Lisp interfaces.
typedef struct _EnumEntry {
  struct _EnumEntry *next;
  char   *name;
  char   *value;
} EnumEntry;
typedef struct {
  EnumEntry *head, *tail;
} EnumList;

void ReadEnumName( InStream *ins, OutStream *outs, EnumList *enumlist )
{
    char enumname[MAX_TOKEN_SIZE+1];
    char enumint[MAX_TOKEN_SIZE+1];
    int maxlen = MAX_TOKEN_SIZE, nsp;
    EnumEntry *newentry = new(EnumEntry);

    // Get the name
    while (!ins->GetToken( maxlen, enumname, &nsp )) {
      // Convert newlines
      if (enumname[0] == '\n') {
	outs->PutToken( nsp, NewlineString );
      }
      else
	break;
    }

    // Read next token.  If not =, push back and return, else read an int.
    while (!ins->GetToken( maxlen, enumint, &nsp )) {
      // Convert newlines
      if (enumint[0] == '\n') {
	outs->PutToken( nsp, NewlineString );
      }
      else
	break;
    };

    if (enumint[0] == '=') {
      ins->GetToken( maxlen, enumint, &nsp );
    }
    else {
      while (nsp--) ins->UngetChar( ' ' );
      ins->UngetToken( enumint );
      enumint[0] = 0;   // Forget that token.
    }
    enum->name = strdup( enumname );
    enum->value = (enumint[0]) ? 0 : strdup( enumint );
    if (enumlist->tail) enumlist->tail->next = enum;
    else enumlist->tail = enumlist->head = enum;
    enum->next = 0;
}

#endif

// Manage the index and jump file
static FILE *idxfd=0, *jumpfd=0;
static const char *idxdir = 0;
// Set the index file.  If idxname is null, ignore (no error)
int IndexFileInit(const char *idxname, const char *idxdir_in)
{
    if (idxname && idxname[0]) {
	idxfd  = fopen(idxname, "a");
	idxdir = idxdir_in;
    }
    return 0;
}
// Add an index that matches "name" without label (output name) outname,
// and mapped to outfile#label (or outfile is label is null)
int IndexFileAdd(const char *name, const char *outname,
		 const char *outfilename, const char *label)
{
    const char *pp;
    if (!idxfd) return 0;  // Skip if no index file
    // Check for valid input
    if (!name || name[0] == 0 || !outname || outname[0] == 0) {
	fprintf(stderr, "Internal error (%s): Empty index entry or name\n",
		GetCurrentInputFileName());
	return -1;
    }
    pp = outfilename + strlen(outfilename) - 1;
    while (pp > outfilename && *pp != '/') pp--;
    if (*pp == '/') pp++;  // If there was no directory, leave the name alone
    // The output is
    //    type (manual) name-to-match name-to-replace directory location
    if (label)
	fprintf(idxfd, "man:+%s++%s++++man+%s/%s#%s\n",
		name, outname, idxdir, pp, label);
    else
	fprintf(idxfd, "man:+%s++%s++++man+%s/%s\n",
		name, outname, idxdir, pp);

    return 0;
}
void IndexFileEnd(void)
{
    if (idxfd) fclose(idxfd);
}
int JumpFileInit(const char *jumpfile)
{
    if (jumpfile && jumpfile[0]) {
	jumpfd = fopen( jumpfile, "a" );
    }
    return 0;
}
int JumpFileAdd(const char *infilename, const char *routine, int linenum)
{
    char rpath[1024];
    if (!jumpfd) return 0;
    /* If there is a jumpfile, add the line */
    /* Note that we want an ABSOLUTE path for the infilename */
    SYGetRealpath(infilename, rpath);
    fprintf(jumpfd, "%s:%s:%d\n", routine, rpath, linenum);
    return 0;
}
void JumpFileEnd(void)
{
    if (jumpfd) fclose(jumpfd);
}

// --------------------------------------------------------------------------
// Skip any ignore string (considered as an attribute in a function declaration,
// such as "EXPORT_API").  Return true if the ignore string was found.
// Skips any blanks after ignore string if found.
static int skipIgnore(char *token, InStream *ins)
{
    if (IgnoreString && strcmp(token, IgnoreString) == 0) {
	skipBlanks(ins);
	return 1;
    }
    return 0;
}
// Skip blanks
static void skipBlanks(InStream *ins)
{
    char ch;
    while (!ins->GetChar(&ch) && isspace(ch) ) ;
    ins->UngetChar(ch);
}

// Output token or correct newline.
// Return 1 if newline seen, 0 otherwise
static int outToken(int nsp, char *token, TextOut *outs)
{
    if (token[0] == '\n') {
	outs->PutToken(nsp, NewlineString);
	return 1;
    }
    outs->PutToken(nsp, token);
    return 0;
}

// --------------------------------------------------------------------------
// Update the input break table for newlines and underscores
static int nl_break;
static int us_break;
static int breaklevel = 0; // Use to insure not nested
static void pushBreakchars(InStream *ins)
{
    breaklevel++;
    if (breaklevel != 1) {
	fprintf(stderr, "Internal error (%s): Nested call to pushBreakchars\n",
		GetCurrentInputFileName());
	return;
    }
    nl_break = ins->breaktable[(unsigned char)'\n'];
    us_break = ins->breaktable[(unsigned char)'_'];
    ins->SetBreakChar( '\n', BREAK_OTHER );
    ins->SetBreakChar( '_', BREAK_ALPHA );
}
static void popBreakchars(InStream *ins)
{
    breaklevel--;
    if (breaklevel != 0) {
	fprintf(stderr, "Internal error (%s): Nested call to popBreakchars\n",
		GetCurrentInputFileName());
	return;
    }
    ins->SetBreakChar( '\n', nl_break );
    ins->SetBreakChar( '_', us_break );
}

// --------------------------------------------------------------------------
// These routines allow us to add index entries for values in an argument
// list that point back to the containing page.
static int qIndexArgs=0;
void indexArgsSet(int val)
{
    qIndexArgs = val;
}
// Return 1 if an index entry is added, 0 otherwise
int indexArgsPut(const char *argname)
{
    if (!qIndexArgs) return 0;
    // Use argname as the anchor name
    IndexFileAdd(argname, argname, GetCurrentFileName(), argname);
    // GetCurrentRoutinename());
    return 1;
}
