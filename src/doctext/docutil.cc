#include "textout.h"
#include "tfile.h"
#include "doc.h"
#include "docutil.h"
#include <ctype.h>
#include <string.h>

char LeadingString[10] = "";

#define MAX_TOKEN_SIZE 255

static int  skipIgnore(char *token, InStream *ins);
static void skipBlanks(InStream *ins);

static int outToken(int nsp, char *token, TextOut *outs);
static void pushBreakchars(InStream *ins);
static void popBreakchars(InStream *ins);

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
int DocReadFuncSynopsis( InStream *ins, TextOut /*OutStream */ *outs)
{
    char ch;
    char token[MAX_TOKEN_SIZE+1];
    int  maxlen = MAX_TOKEN_SIZE, nsp;
    int  findingForm = 1;
    int  exitOnSemicolon = 0;
    int  parenCount = 0;

    // Must handle newline as non-space
    pushBreakchars(ins);
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
	if (token[0] == '(') parenCount++;
	else if (token[0] == ')') {
	  if (parenCount == 1) {
	    if (findingForm) {
	      // if the NEXT char is a ;, we have a terminal prototype
	      exitOnSemicolon = 1;
	    }
	    findingForm = 0;
	  }
	  parenCount --;
	}
	outToken(nsp, token, outs);
      }
    }
    popBreakchars(ins);
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

/* Read (and output) until we get two newlines */
int DocReadMacroSynopsis( InStream *ins, char *matchstring,
			  TextOut /*OutStream*/ *outs, int *at_end )
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
    char ch;
    char token[MAX_TOKEN_SIZE+1];
    int in_brace = 0;
    int maxlen = MAX_TOKEN_SIZE, nsp;
    void *fieldlist=0;       // Fieldlist records the entries in the
                             // name.  NOT FULLY IMPLEMENTED

    // Must handle newline as non-space
    pushBreakchars(ins);
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
    return 0;
}

int DocReadDefineDefinition( InStream *ins, TextOut *outs )
{
    char ch;
    char token[MAX_TOKEN_SIZE+1];
    int in_brace = 0;
    int maxlen = MAX_TOKEN_SIZE, nsp;
    int prevnewline = 0;
    void *fieldlist=0;       // Fieldlist records the entries in the
                             // name.  NOT FULLY IMPLEMENTED

    // Must handle newline as non-space
    pushBreakchars(ins);
    while (!ins->GetToken( maxlen, token, &nsp )) {
	// Check for \n and do PutNewline instead.
	int sawNewline = outToken(nsp, token, outs);
	if (sawNewline && prevnewline) break;
	prevnewline = sawNewline;
    }
    popBreakchars(ins);
    return 0;
}

// Read an enum definition, including a optional typedef and
// optional values for the names.  E.g.,
//   typedef enum { foo, bar=2, last } name;
// Must also output the text, and generate additional links
int DocReadEnumDefinition(InStream *ins, TextOut *outs)
{
    char ch;
    char token[MAX_TOKEN_SIZE+1];
    int  in_brace = 0;
    int  maxlen = MAX_TOKEN_SIZE, nsp;
    int  isEnum = 1;      // Used to check if recognized as enum or typedef enum
    enum { nxt_val, has_name, has_val } state;

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

    // Now have first token that is not typedef enum
    state = nxt_val;
    do {
	if (verbose) printf("processing token :%s:\n",token);
	// Eventually we need to combine these into a single lookup list.
	if (!skipIgnore(token, ins))
	    outToken(nsp, token, outs);

	// If that was the last character, exit
	if (in_brace == 0 && token[0] == ';') {
	    break;
	}
	if (token[0] == '{') in_brace++;
	else if (token[0] == '}') in_brace--;
	else if (token[0] == ',') state=nxt_val;
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
		state=nxt_val;
		if (verbose) printf("enum value is %s\n", token);
		break;
	    }
	}
    } while (!ins->GetToken(maxlen, token, &nsp));

    outs->PutToken(nsp, NewlineString);
    popBreakchars(ins);
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
	fprintf(stderr, "Internal error: Empty index entry or name\n");
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
    char ch;
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
	fprintf(stderr, "Internal error: Nested call to pushBreakchars\n");
	return;
    }
    nl_break = ins->breaktable['\n'];
    us_break = ins->breaktable['_'];
    ins->SetBreakChar( '\n', BREAK_OTHER );
    ins->SetBreakChar( '_', BREAK_ALPHA );
}
static void popBreakchars(InStream *ins)
{
    breaklevel--;
    if (breaklevel != 0) {
	fprintf(stderr, "Internal error: Nested call to popBreakchars\n");
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
void indexArgsPut(const char *argname)
{
    if (!qIndexArgs) return;
    IndexFileAdd(argname, argname, GetCurrentFileName(),
		 GetCurrentRoutinename());
}
