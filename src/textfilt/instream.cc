/* -*- Mode: C++; c-basic-offset:4 ; -*- */
#include "tfilter.h"

#include "instream.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static int show_paths = 0;

int InstreamDebugPaths( int flag )
{
  int old_flag = flag;
  show_paths = flag;
  return old_flag;
}

/*
 * Input streams
 * 
 * These are more powerful than the default C++ streams because they
 * allow for simple composition of streams and for dynamic definition
 * of tokens (replaces <ctype.h>).
 *
 * Notes on the break table.  By default, the entires are:
 * 0 (special, means singleton tokens)
 * 1 space (blank space; skipped and number of chars returned in nsp)
 * 2 alpha
 * 3 digit
 * 
 * Note that 0 and 1 are handled specially.
 * Also, user may define their own values; tokens will consist of all character
 * of the same class.
 */

/* Helpers for the breaktable/quote */
int InStream::ResetTables()
{
    int i;
    squote = 0;
    equote = 0;
    for (i=0; i<256; i++) {
	breaktable[i] = BREAK_OTHER;
	if (isascii(i)) {
	    if      (isalpha(i)) breaktable[i] = BREAK_ALPHA;
	    else if (isdigit(i)) breaktable[i] = BREAK_DIGIT;
	    else if (isspace(i)) breaktable[i] = BREAK_SPACE;
	    }
	}
    return 0;
}

/* 
   The base stream doesn't know how to do much; mostly, it just passes
   the buck
 */
int InStream::GetChar( char *c )
{
    if (next) 
	return next->GetChar( c );
    else
	return 1;
}

int InStream::GetToken( int maxlen, char *token, int *nsp )
{
    int Nsp;
    int char_class;
    int quote_cnt;
    char c;
    
    *nsp = 0;
    if (GetChar( &c )) return 1;
    /* Handle leading space first */
    if (breaktable[c] == BREAK_SPACE) {
	Nsp = 1;
	while (!GetChar( &c ) && breaktable[c] == 1) Nsp++;
	*nsp = Nsp;
	}
    /* Quoted strings are handled separately */
    if (c && c == squote) {
	quote_cnt = 1;
	*token++ = c; maxlen--;
	while (maxlen && quote_cnt > 0) {
	    if (GetChar( &c )) break;
	    if (c == squote) quote_cnt++;
	    else if (c == equote) quote_cnt--;
	    *token++ = c; maxlen--;
	    }
	*token = 0;
	/* Purge token if quote_cnt != 0? */
	if (quote_cnt) return 1;
	}
    else {
	char_class = breaktable[c];
	*token++ = c; maxlen--;
	if (char_class == BREAK_OTHER) {
	    *token = 0;
	    return 0;
	    }
	while (maxlen) {
	    if (GetChar( &c )) break;
	    if (breaktable[c] != char_class) {
		UngetChar( c );
		break;
		}
	    *token++ = c; maxlen--;
	    }
	*token = 0;
	}
    return 0;
}

int InStream::GetLine( char *line, int maxlen )
{
    char ch = 0;

    *line = 0;
    while (maxlen && !GetChar( &ch )) {
	*line++ = ch;
	maxlen--;
	if (ch == '\n') break;
	}
    *line = 0;
    return ch != '\n';
}

int InStream::UngetChar( char c )
{
    if (next) 
	return next->UngetChar( c );
    else
	return 1;
}

int InStream::UngetToken( char *token )
{
    int i;
    for (i=strlen(token)-1; i>=0; i--) 
      UngetChar( token[i] );
    return 0;
}

int InStream::GetLoc( long *position )
{
    if (next)
	return next->GetLoc( position );
    else
	return 1;
}

int InStream::SetLoc( long position )
{
    if (next)
	return next->SetLoc( position );
    else
	return 1;
}

int InStream::SetBreakChar( char c, int kind )
{
    breaktable[c] = kind;
	return 0;
}

int InStream::SetBreakChars( char *str, int kind )
{
    while (*str) 
	breaktable[*str++] = kind;
	return 0;
}

int InStream::SetQuoteChars( char schar, char echar )
{
    squote = schar;
    equote = echar;
	return 0;
}

int InStream::SkipLine( void )
{
    char ch;
    while (!GetChar( &ch )) 
	if (ch == '\n') break;
    return 0;
}

int InStream::GetLineNum( void )
{
    if (next)
	return next->GetLineNum();
    else 
	return 0;
}

int InStream::GetSourceName( char *filename, int maxlen, int *linecnt )
{
    if (next) 
	return next->GetSourceName( filename, maxlen, linecnt );
    else {
	filename[0] = 0;
	*linecnt    = 0;
	return 1;
	}
}

int InStream::Close( )
{
    if (next)
	return next->Close( );
    else
	return 1;
}

InStream::~InStream( )
{
    if (next)
	next->~InStream();
}

/*
 * First, the simple file stream (got to get to the file eventually)
 */

// This is the character used to separate directories/files.  / for Unix,
// \ for MSDOS
#if defined(__MSDOS__) || defined(WIN32)
#define DIR_SEP '\\'
#define PATH_SEP ' '
#else
#define DIR_SEP '/'
#define PATH_SEP ':'
#endif
/*
   Try to open envpath/name, then pathlist/name, then name.
   pathlist and envpath contain : separated directories
 */
InStreamFile::InStreamFile( const char *pathlist, const char *envpath, 
			    const char *name, const char *mode )
{
    char *pname = 0, *p;
    char *(plists[2]);
    char fullpath[1024], *fptr;
    int  i;

    plists[0] = 0;
    plists[1] = (char *)pathlist;
    if (envpath) 
	plists[0] = getenv( envpath );

    if (show_paths) {
      printf( "Looking in default path %s,\n\
environment variable %s (%s)\n\
for %s with mode %s\n", 
	      pathlist, envpath, plists[0] ? plists[0] : "<empty>", 
	      name, mode );
    }
    fp = 0;
    for (i=0; !fp && i<2; i++) {
	pname = plists[i];
	// Find directory 
	while (pname && *pname) {
	    p = pname;
	    // Create path
	    fptr = fullpath;
	    while (*p && *p != PATH_SEP) *fptr++ = *p++;
 	    if (*p == PATH_SEP) p++;
	    if (fptr == fullpath) continue;
	    if (fptr[-1] != DIR_SEP) *fptr++ = DIR_SEP; 
	    *fptr = 0;
	    strcat( fptr, name );
	    fp = fopen( fullpath, mode );
	    if (fp) {
	      if (show_paths) {
		printf( "Opened %s with mode %s\n", fullpath, mode );
	      }
	      fname = new char[strlen(fullpath)+1];
	      strcpy( fname, fullpath );
	      break;
	    }
	    // Advance to the next path
	    pname = p;
	    }
	}
    if (!fp) {
	/* Try to open JUST the name */
	fp = fopen( name, mode );
	if (fp) {
	  if (show_paths) {
	    printf( "Opened %s with mode %s\n", fullpath, mode );
	  }
	  fname = new char[strlen(name)+1];
	  strcpy( fname, name );
	}
    }
    if (!fp) {
      // Should be errno
      status = -1;
    }
    else 
      status = 0;
    next       = 0;
    linecnt    = 0;
    colcnt     = 0;
    expand_tab = 0;
    nblanks    = 0;
    didunget   = 0;
    ResetTables();
}

InStreamFile::InStreamFile( const char *path, const char *mode )
{
    fp	 = fopen( path, mode );
    if (!fp) 
	// Should be errno
	status = -1;
    else {
      if (show_paths) {
	printf( "Opened %s with mode %s\n", path, mode );
      }
      status = 0;
    }
    fname = new char[strlen(path)+1];
    strcpy( fname, path );
    linecnt    = 0;
    colcnt     = 0;
    expand_tab = 0;
    nblanks    = 0;
    didunget   = 0;
    next       = 0;
    ResetTables();
}

InStreamFile::InStreamFile( )
{
    fp	       = stdin;
    next       = 0;
    linecnt    = 0;
    colcnt     = 0;
    expand_tab = 0;
    nblanks    = 0;
    didunget   = 0;
    fname      = 0;
    ResetTables();
}

int InStreamFile::GetChar( char *c )
{
    int ch;

    if (nblanks) {
	// We expanded a tab; return one of the blanks.
	*c = ' ';
	nblanks--;
	return 0;
    }
    ch = fgetc( fp );
    didunget = 0;
    if (ch == -1) {
	*c = 0;
	return 1;
	}
    /* Convert DOS to nonDOS */
    if (ch == '\r') {
	int ch2 = fgetc( fp );
	if (ch2 != '\n') {
	    ungetc( ch2, fp );
	}
	else 
	    ch = ch2;
    }

    if (ch == '\n') { linecnt++; colcnt = 0; }
    if (expand_tab && ch == '\t') {
      while (colcnt++ % 8) {
	  nblanks++;
      }
      ch = ' ';
    }
    *c = ch;
    colcnt++;
    return 0;
}

int InStreamFile::UngetChar( char c )
{
    // Only one character pushback is guaranteed.  Linux has a bug in 
    // that exceeding the pushback causes erroneous behavior without
    // an error being reported.
    if (didunget) {
	fprintf( stderr, 
		 "Warning: multiple unget in InStreamFile::UngetChar\n" );
    }
    ungetc( c, fp );
    didunget = 1;
    // This isn't quite correct unless the ch is the character we read.
    colcnt--;
    if (c == '\n') { linecnt--; colcnt = 0; }
    return 0;
}

int InStreamFile::UngetToken( char *token )
{
    ungetc( *token++, fp );
    if (!*token) return 1;
    return 0;
}

int InStreamFile::GetLoc( long *position )
{
    *position = ftell( fp );
    //printf( "Position = %ld\n", *position );
    return 0;
}

int InStreamFile::SetLoc( long position )
{
    fseek( fp, position, 0 );
    //printf( "Seeked to %ld\n", position );
    // linenum and colcnt are now broken!
    nblanks = 0;  // In case there were pushed-back blanks.
    return 0;
}

int InStreamFile::Close( )
{
    fclose( fp );
    return 0;
}

InStreamFile::~InStreamFile( )
{
    fclose( fp );
}

int InStreamFile::GetSourceName( char *filename, int maxlen, int *linenum )
{
    *linenum = linecnt;
    strncpy( filename, fname, maxlen );
    return 0;
}
int InStreamFile::GetLineNum( void )
{
	return linecnt;
}

/* 
 * Buffered stream.  When the stream is empty it tries to use the next
 * instream to get input.
 */
InStreamBuf::InStreamBuf( int in_maxlen, InStream *p )
{
    maxlen   = in_maxlen;
    buffer   = new char[maxlen];
    if (buffer) status = 0; 
    else        status = -1;   // ENOMEM?
    position = buffer;
    curlen   = 0;
    next     = p;
    ResetTables();
}

int InStreamBuf::GetChar( char *ch )
{
    if (curlen > 0) {
	*ch = *position--;
	curlen--;
	return 0;
	}
    else 
#if 0
{      int rc = next->GetChar( ch );
    fprintf( stderr, "Get char %c\n", *ch ); fflush( stderr );
    return rc;
}
#else
	return next->GetChar( ch );
#endif
}

int InStreamBuf::UngetChar( char ch )
{
    if (curlen < maxlen) {
	*++position = ch;
	curlen++;
	return 0;
	}
    else {
      // fprintf( stderr, "exceeded pushback buffer\n" );
	return 1;
	}
}

/* The gettoken base should just use getchar */
int InStreamBuf::Close( )
{
    if (buffer) delete buffer;
    return 0;
}

InStreamBuf::~InStreamBuf( )
{
    //if (next) delete next;
    if (buffer) delete buffer;
}

/*
 * InStreamStack stacks instreams, and pops them as EOFs are seen.
 */
InStreamStack::InStreamStack( )
{
}
InStreamStack::~InStreamStack( )
{
	InStream *nins;
	while (next) {
		nins = next->next;
		delete nins;
		next = nins;
	}
}
int InStreamStack::Push( InStream *ins )
{
	if (ins->next) {
	    // Can't stack instreams that have children
	    return 1;
	    }
	ins->next = next;
	next = ins;
	return 0;
}
int InStreamStack::Pop()
{
	InStream *nins;
	if (next) {
		nins = next->next;
		delete next;
		next = nins;
	}
	return 0;
}
int InStreamStack::GetChar( char *ch )
{
	int rc;
	InStream *nins;

	if (!next) return 1;
	rc = next->GetChar( ch );
	while (rc) {
	    // Really needs to check for EOF
	    nins = next->next;
	    delete next;
	    next = nins;
	    if (!next) break;
	    rc = next->GetChar( ch );
	}
	return rc;
}
