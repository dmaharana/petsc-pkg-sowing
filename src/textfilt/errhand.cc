#include "tfilter.h"

#include "errhand.h"
#include <stdio.h>

int ErrHand::ErrMsg( int code, const char *msg )
{
  return code;
}
	
ErrHand::~ErrHand( )
{
}

ErrHandMsg::ErrHandMsg( const char *path )
{
	errout = fopen( path, "w" );
}
ErrHandMsg::ErrHandMsg()
{
	errout = stderr;
}
ErrHandMsg::~ErrHandMsg()
{
	if (errout && errout != stderr) fclose( errout );
}
int ErrHandMsg::ErrMsg( int code, const char *msg )
{
	if (code) {
	    if (errout) fprintf( errout, "Error: %s:%d\n", msg, code );
	    exit( 1 );
	    }
        return code;
}

ErrHandReturn::ErrHandReturn()
{
}
ErrHandReturn::~ErrHandReturn()
{
}
int ErrHandReturn::ErrMsg( int code, const char *msg )
{
	return code;
}



