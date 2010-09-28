#include "tfilter.h"

#include "maptok.h"
#include <ctype.h>
#include <string.h>

/*
 * This file contains routines to map names from string to another.
 * The mechanism is fairly general, and is implemented as an OutStream,
 * allowing it to be instered almost anywhere.
 *
 * The most convenient way to generating the mapping table is by 
 * reading a file; for this we use the command file-reading object.
 * Should probably remove Outstream version, or unify code
 */

/* Local info for replacement data */
typedef struct {
    char *repname;
    char *url;
    } MapData;

void OutStreamMap::Setup( int in_maxlen )
{
    int i;

    squote	  = 0;
    equote	  = 0;
    maxlen	  = in_maxlen;
    activetok	  = new char[maxlen];
    activetok[0]  = 0;
    curlen	  = 0;
    position	  = activetok;
    maptable	  = new SrList();
    print_matched = 0;  // For debugging

    for (i=0; i<255; i++) {
	breaktable[i] = BREAK_OTHER;
	if (isascii(i)) {
	    if      (isalpha(i)) breaktable[i] = BREAK_ALPHA;
	    else if (isdigit(i)) breaktable[i] = BREAK_ALPHA;
	    }
	}
    breaktable['_'] = BREAK_ALPHA;
}

OutStreamMap::OutStreamMap( OutStream *outs, int in_maxlen )
{
    Setup( in_maxlen );
    next      = outs;
}

OutStreamMap::OutStreamMap( OutStream *outs )
{
    Setup( 1024 );
    next = outs;
}
OutStreamMap::OutStreamMap( int in_maxlen )
{
    Setup( in_maxlen );
    next = 0;
}
OutStreamMap::OutStreamMap( OutStream *outs, int in_maxlen, int pflag )
{
  if (in_maxlen <= 0) in_maxlen = 1024;
  Setup( in_maxlen );
  next = outs;
  print_matched = pflag;
}
void OutStreamMap::FlushTokBuf( void )
{
  if (curlen) {
    SrEntry *entry;
    if (maptable->Lookup( activetok, &entry ) == 0) {
      PutLink( activetok, entry );
    }
    else {
      next->PutToken( 0, activetok );
    }
    curlen = 0;
  }
}

/* 
 * This routine works by putting a token into the internal buffer.
 * Once it knows that it can (found a delimiter), it tries to look it
 * up and replace it.
 */
int OutStreamMap::PutToken( int nsp, const char *token )
{
    SrEntry *entry;
    
    /* 
       First, tokenize the output and check each token.  Special case:
       if we reach the end of the string without a "break" character, we must
       delay processing until we get the break character, since the token may
       be incomplete (for example, MPI_Send may be delivered to this 
       routine as MPI, then _, then Send. 
     */

    /* Spaces are a delimiter.  Try to flush activetok, and rerun PutToken */
    if (nsp) {
      FlushTokBuf();
      next->PutToken( nsp, (char *)0 );
      return PutToken( 0, token );
    }

    /* Copy token to activetok while the breaktable entries are the same.
       Careful of the BREAK_OTHER and BREAK_SPACE case.
       We ONLY lookup tokens with break values of 2 (BREAK_ALPHA) */
    if (!token) return 0;
    while (*token) {
	/* This really needs to tokenize the string */
	if (breaktable[(int)*token] == BREAK_ALPHA) {
	    /* The character is a "alphanum" */
	    while (*token && breaktable[(int)*token] == BREAK_ALPHA) {
		activetok[curlen++] = *token++;
		if (curlen >= maxlen) {
		    activetok[maxlen-1] = 0;
		    fprintf( stderr, "Token too long (%s)=n", activetok );
		    return 1;
		    }
		}
	    activetok[curlen] = 0;
	    /* If we ended at a alnum-like token, don't output yet. */
	    if (*token == 0) break;
	    }
    else {
	/* If there is already something in the token buffer, drain it first
	 */
	if (curlen == 0) {
	    /* Skip over the non-alphanum characters */
	    while (*token && breaktable[(int)*token] != BREAK_ALPHA) {
		activetok[curlen++] = *token++;
		if (curlen >= maxlen) {
		    activetok[maxlen-1] = 0;
		    fprintf( stderr, "Token too long (%s)=n", activetok );
		    return 1;
		    }
		}
	    }
	activetok[curlen] = 0;
	}
    if (maptable->Lookup( activetok, &entry ) == 0) {
      // Allow debugging output of all matched tokens
      // Eventually, we should instead have a "token stream" operation;
      // then this filter is simply built on top of that token stream
      if (print_matched) {
	printf( "%s\n", activetok );
      }
      PutLink( activetok, entry );
	}
    else {
	next->PutToken( 0, activetok );
	}
    /* We've flushed the token */
    curlen = 0;
    }
    return 0;
}

int OutStreamMap::PutQuoted( int nsp, const char *token )
{
  //  if (debug_flag && token && *token)
  //    printf( "OutStreamMap::PutQuoted %s\n", token );
  FlushTokBuf();
  return next->PutQuoted( nsp, token );
}

int OutStreamMap::PutChar( const char c )
{
    char cc[2];
    cc[0] = c;
    cc[1] = 0;
    return PutToken( 0, cc );
}

#define MAX_NAME_LEN 128
#define MAX_REPNAME_LEN 128
#define MAX_URL_LEN 512
/*
   Read a map file from the instream
 */
int OutStreamMap::ReadMap( InStream *ins )
{
    char name[MAX_NAME_LEN], reptext[MAX_REPNAME_LEN], url[MAX_URL_LEN], *p;
    char ch, sepchar;
    SrEntry *entry;
    MapData *info;
    int     ln;

    while (!ins->GetChar( &ch )) {
	/* Skip comments */
	if (ch == '#') {
	    ins->SkipLine();
	    continue;
	    }
	/* Read file line.  One format is
	   tagtype:%cname%c%crepname%c%c%c%cskip%cURL
	   
	   where %c is any character, but the same character must be used
	   in all places (on a line-bu-line basis).
         */
	while (!ins->GetChar( &ch ) && ch != ':' && ch != '\n') ;
	ins->GetChar( &sepchar );
	p = name; ln = 0;
	while (!ins->GetChar( &ch ) && ch != sepchar && ++ln < MAX_NAME_LEN)
	  *p++ = ch;
	if (ch != sepchar) {
	  fprintf( stderr, "Name too long in map\n" );
	  return 1;
	}
	ins->GetChar( &ch );
	p = reptext; ln = 0;
	while (!ins->GetChar( &ch ) && ch != sepchar && 
	       ++ln < MAX_REPNAME_LEN) *p++ = ch;
	if (ch != sepchar) {
	  fprintf( stderr, "Replacement name too long in map\n" );
	  return 1;
	}
	while (!ins->GetChar( &ch ) && ch == sepchar) ;
	while (!ins->GetChar( &ch ) && ch != sepchar) ;
	p = url; ln = 0;
	while (!ins->GetChar( &ch ) && ch != '\n' && ++ln < MAX_URL_LEN) 
	  *p++ = ch;
	if (ch != '\n') {
	  fprintf( stderr, "URL too long in map\n" );
	  return 1;
	}
	/* Install this name */
	
	maptable->Insert( name, &entry );
	info = new MapData;
	info->repname = new char[strlen(reptext)+1];
	info->url     = new char[strlen(url)+1];
	strcpy( info->repname, reptext );
	strcpy( info->url, url );
	entry->extra_data = (void *)info;
	}
	return 0;
}

// This should really use a PutOp( "link", url, repname ) call to
// the appropriate textout handler.
int OutStreamMap::PutLink( const char *name, SrEntry *entry )
{
    MapData *info = (MapData *)entry->extra_data;
    next->PutToken( 0, "<A href=\"" );
    next->PutToken( 0, info->url );
    next->PutToken( 0, "\">" );
    next->PutToken( 0, info->repname );
    next->PutToken( 0, "</A>" );
    return 0;
}
OutStreamMap::~OutStreamMap()
{
    delete activetok;
    delete maptable;
    if (next)
	delete next;
}

//
// Textout version
//
void TextOutMap::Setup( int in_maxlen )
{
    int i;

    squote    = 0;
    equote    = 0;
    maxlen    = in_maxlen;
    activetok = new char[maxlen];
    activetok[0] = 0;
    curlen    = 0;
    position  = activetok;
    maptable  = new SrList();

    for (i=0; i<255; i++) {
	breaktable[i] = BREAK_OTHER;
	if (isascii(i)) {
	    if      (isalpha(i)) breaktable[i] = BREAK_ALPHA;
	    else if (isdigit(i)) breaktable[i] = BREAK_ALPHA;
	    }
	}
    breaktable['_'] = BREAK_ALPHA;

    // 
    err		  = new ErrHandMsg();
    lfont	  = 0;
    nl		  = 0;
    last_was_nl	  = 1;
    last_was_par  = 1;
    debug_flag	  = 0;
    next	  = 0;
    userops	  = 0;
    print_matched = 0;

    debug	  = 0;
}

TextOutMap::TextOutMap( TextOut *textout )
{
  Setup( 1024 );
  if (debug_flag) textout->Debug( debug_flag );
  next = textout;
}

TextOutMap::TextOutMap( )
{
  Setup( 1024 );
  next = 0;
}

TextOutMap::TextOutMap( TextOut *textout, int pflag )
{
  Setup( 1024 );
  if (debug_flag) textout->Debug( debug_flag );
  next = textout;
  print_matched = pflag;
}

int TextOutMap::PutChar( const char c )
{
    char cc[2];
    cc[0] = c;
    cc[1] = 0;
    return PutToken( 0, cc );
}

int TextOutMap::PutNewline( void )
{
  int rc;
  UpdateNL( 1 );
  if (*newline_onoutput) 
    rc = PutToken( 0, newline_onoutput );
  else
    rc = PutToken( 0, "\n" );
  return rc;
}

void TextOutMap::FlushTokBuf( void )
{
  if (curlen) {
    SrEntry *entry;
    if (maptable->Lookup( activetok, &entry ) == 0) {
      PutLink( activetok, entry );
    }
    else {
      next->PutToken( 0, activetok );
    }
    curlen = 0;
  }
}
int TextOutMap::PutToken( int nsp, const char *token )
{
    SrEntry *entry;

    if (debug && token) printf( "Mapping token |%s|\n", token );
    /* 
       First, tokenize the output and check each token.  Special case:
       if we reach the end of the string without a "break" character, we must
       delay processing until we get the break character, since the token may
       be incomplete (for example, MPI_Send may be delivered to this 
       routine as MPI, then _, then Send. 
     */

    /* Spaces are a delimiter.  Try to flush activetok, and rerun PutToken */
    if (nsp) {
      FlushTokBuf();
      next->PutToken( nsp, (char *)0 );
      return PutToken( 0, token );
    }

    /* Copy token to activetok while the breaktable entries are the same.
       Careful of the BREAK_OTHER and BREAK_SPACE case.
       We ONLY lookup tokens with break values of 2 (BREAK_ALPHA) */
    if (!token) return 0;
    // We can flush by sending a null token.
    if (token[0] == 0) {
      if (debug) {
        printf( "Looking up %s ...", activetok );
      }
      if (activetok[0] == 0) return 0;
      if (maptable->Lookup( activetok, &entry ) == 0) {
        if (debug) printf( "Found entry\n" );
        PutLink( activetok, entry );
      }
      else {
	if (debug) printf( "Did not find entry\n" );
	next->PutToken( 0, activetok );
      }
      /* We've flushed the token */
      curlen = 0;
      activetok[0] = 0;
    }

    while (*token) {
	/* This really needs to tokenize the string */
	if (breaktable[(int)*token] == BREAK_ALPHA) {
	    /* The character is a "alphanum" */
	    while (*token && breaktable[(int)*token] == BREAK_ALPHA) {
		activetok[curlen++] = *token++;
		if (curlen >= maxlen) {
		    activetok[maxlen-1] = 0;
		    fprintf( stderr, "Token too long (%s)=n", activetok );
		    return 1;
		    }
		}
	    activetok[curlen] = 0;
	    /* If we ended at a alnum-like token, don't output yet. */
	    if (*token == 0) break;
	    }
	else {
	/* If there is already something in the token buffer, drain it first
	 */
	  if (curlen == 0) {
	    /* Skip over the non-alphanum characters */
	    while (*token && breaktable[(int)*token] != BREAK_ALPHA) {
		activetok[curlen++] = *token++;
		if (curlen >= maxlen) {
		    activetok[maxlen-1] = 0;
		    fprintf( stderr, "Token too long (%s)=n", activetok );
		    return 1;
		    }
		}
	    }
	activetok[curlen] = 0;
	}
    if (debug) {
        printf( "Looking up %s ...", activetok );
	}
    if (maptable->Lookup( activetok, &entry ) == 0) {
        if (debug) printf( "Found entry\n" );
	else if (print_matched) printf( "%s\n", activetok );
        PutLink( activetok, entry );
	}
    else {
        if (debug) printf( "Did not find entry\n" );
	next->PutToken( 0, activetok );
	}
    /* We've flushed the token */
    curlen = 0;
    activetok[0] = 0;
    }
    return 0;
}
int TextOutMap::ReadMap( InStream *ins )
{
    char name[MAX_NAME_LEN], reptext[MAX_REPNAME_LEN], url[MAX_URL_LEN], *p;
    char ch, sepchar;
    SrEntry *entry;
    MapData *info;
    int     ln;

    if (debug) printf( "Reading mappings\n" );
    while (!ins->GetChar( &ch )) {
	/* Skip comments */
	if (ch == '#') {
	    ins->SkipLine();
	    continue;
	    }
	/* Read file line.  One format is
	   tagtype:%cname%c%crepname%c%c%c%cskip%cURL
	   
	   where %c is any character, but the same character must be used
	   in all places (on a line-bu-line basis).
         */
	while (!ins->GetChar( &ch ) && ch != ':' && ch != '\n') ;
	ins->GetChar( &sepchar );
	p = name; ln = 0;
	while (!ins->GetChar( &ch ) && ch != sepchar && ++ln < MAX_NAME_LEN) 
	  *p++ = ch;
	if (ch != sepchar) {
	  fprintf( stderr, "Name too long in map\n" );
	  return 1;
	}
	*p = 0;
	ins->GetChar( &ch );
	p = reptext; ln = 0;
	while (!ins->GetChar( &ch ) && ch != sepchar && 
	       ++ln < MAX_REPNAME_LEN) *p++ = ch;
	if (ch != sepchar) {
	  fprintf( stderr, "Replacement name too long in map\n" );
	  return 1;
	}
	*p = 0;
	while (!ins->GetChar( &ch ) && ch == sepchar) ;
	while (!ins->GetChar( &ch ) && ch != sepchar) ;
	p = url; ln = 0;
	while (!ins->GetChar( &ch ) && ch != '\n' && ++ln < MAX_URL_LEN) 
	  *p++ = ch;
	if (ch != '\n') {
	  fprintf( stderr, "URL too long in map\n" );
	  return 1;
	}
	*p = 0;
	
	/* Install this name */
	
	maptable->Insert( name, &entry );
	info = new MapData;
	info->repname = new char[strlen(reptext)+1];
	info->url     = new char[strlen(url)+1];
	strcpy( info->repname, reptext );
	strcpy( info->url, url );
	entry->extra_data = (void *)info;
	if (debug) {
	    printf( "Installing %s with url=%s, text=%s\n", 
		    name, url, reptext );
	}
	}
    return 0;
}
int TextOutMap::PutLink( const char *name, SrEntry *entry )
{
    MapData *info = (MapData *)entry->extra_data;
    // We may need to suppress the output message when no link command
    // is specified, since that is a common case
    if (next->userops->Lookup( "link", 0 )) {
      next->PutToken( 0, "<A href=\"" );
      next->PutToken( 0, info->url );
      next->PutToken( 0, "\">" );
      next->PutToken( 0, info->repname );
      next->PutToken( 0, "</A>" );
    }
    else {
      next->PutOp( "link", info->url, info->repname, 0 );
    }
    return 0;
}
int TextOutMap::PutQuoted( int nsp, const char *token )
{
  //  if (debug_flag && token && *token)
  //    printf( "OutStreamMap::PutQuoted %s\n", token );
  FlushTokBuf();
  return next->PutQuoted( nsp, token );
}

TextOutMap::~TextOutMap()
{
    delete activetok;
    delete maptable;
    if (next)
	delete next;
}
int TextOutMap::SetRegisterValue( int regnum, const char * val )
{
  if (next)
    next->SetRegisterValue( regnum, val );
  return 0;
}
