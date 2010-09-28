#include "tfilter.h"

#include "outstream.h"
#include <memory.h>

int OutStream::PutToken( int nsp, const char *token )
{
    int i;
    if (debug_flag && token && *token) {
      printf( "(Generic)%s::PutToken %s\n", MyName(), token );
      printf( "debug_flag = %d\n", debug_flag );
    }
    for (i=0; i<nsp; i++) PutChar( ' ' );
    if (token)
	while (*token) PutChar( *token++ );
	return 0;
}

int OutStream::PutQuoted( int nsp, const char *token )
{
  if (debug_flag && token && *token) 
    printf( "(Generic)%s::PutQuoted %s\n", MyName(), token );
  return PutToken( nsp, token );
}

int OutStream::PutChar( const char ch )
{
    if (next)
	return next->PutChar( ch );
    else
	return 1;
}

/* Default flush is empty */
void OutStream::Flush()
{
  if (next)
    next->Flush();
}

void OutStream::SetBufMode( int flag )
{
  bufmode = flag;
  if (next) next->SetBufMode( flag );
}

int OutStream::PutInt( int i )
{
    char buf[20];
    sprintf( buf, "%d", i );
    return PutToken( 0, buf );
}

int OutStream::Close( )
{
    if (next)
	return next->Close();
    else 
	return 0;
}

OutStream::~OutStream()
{
    if (next) 
	next->~OutStream();
}

int OutStream::Debug( int flag )
{
  int old_flag = debug_flag;

  printf( "New debug_flag = %d\n", flag );
  debug_flag = flag;
  return old_flag;
}

const char *OutStream::MyName( void )
{
  return "OutStream";
}

/*
 * File versions
 */

OutStreamFile::OutStreamFile( const char *path, const char *mode )
{
    fp	 = fopen( path, mode );
    if (fp)
        status = 0;
    else
        status = -1;
    next       = 0;
    bufmode    = 0;
    debug_flag = 0;
}


OutStreamFile::OutStreamFile( )
{
    fp	       = stdout;
    status     = 0;
    next       = 0;
    bufmode    = 0;
    debug_flag = 0;
}

int OutStreamFile::PutChar( const char ch )
{
  // fprintf( stderr, "Output %c\n", ch );
  if (debug_flag && ch) printf( "OutStreamFile::PutChar of %c\n", ch );
  if (ch) {
    fputc( ch, fp );
  }
  if (bufmode) Flush();
  return 0;
}

void OutStreamFile::Flush( void )
{
  fflush( fp );
}

int OutStreamFile::Close( )
{
    fclose( fp );
    fp = 0;
    return 0;
}

OutStreamFile::~OutStreamFile()
{
     (void) Close();
}

const char *OutStreamFile::MyName( void )
{
  return "OutStreamFile";
}

/* 
 * Character buffer.  Still need to decide how to extract data.
 *                    This version will automatically extend the space,
 *                    doubling each time.
 */
OutStreamBuf::OutStreamBuf( int in_maxlen )
{
    buffer   = new char[in_maxlen];
    status   = (buffer == 0);
    maxlen   = in_maxlen;
    position = buffer;
    curlen   = 0;
    next     = 0;
    bufmode  = 0;
    debug_flag = 0;
}

int OutStreamBuf::PutChar( const char ch )
{
    if (!ch) return 0;
    if (curlen >= maxlen) {
    	// Extend the buffer
    	char *tmp = new char[2*maxlen];
    	if (!tmp) return 1;
    	memcpy( tmp, buffer, curlen );
    	position = tmp + curlen;
    	delete buffer;
    	buffer = tmp;
    	maxlen = 2 * maxlen;
	if (debug_flag) printf( "OutStreamBuf- doubling buffer\n" );
	// fprintf( stderr, "Doubling buffer\n" );
        }
    if (debug_flag && ch) printf( "OutStreamBuf::PutChar of %c\n", ch );
    *position++ = ch;
    curlen++;

    return 0;    
}

OutStreamBuf::~OutStreamBuf( )
{
    delete buffer;
}

char *OutStreamBuf::GetBuffer()
{
    // Add null at end of string
    *position = 0;
    return buffer;
}
void OutStreamBuf::Reset()
{
    // Discard contents of buffer
    position = buffer;
    curlen   = 0;
}

const char *OutStreamBuf::MyName( void )
{
  return "OutStreamBuf";
}

