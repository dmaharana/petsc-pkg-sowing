#include "tfilter.h"

#include <string.h>
#include <stdio.h>
#include "cmdline.h"


/* 
   This file contains routines for procssing the command line.
 */

CmdLine::CmdLine( int in_argc, char ** in_argv )
{
    cnt  = 1;
    argc = in_argc;
    argv = in_argv;
}

/*
   SqueezeArg - Remove all null arguments from an arg vector; 
   update the number of arguments.
 */
int CmdLine::SqueezeArg()
{
    int i, j;
    
    /* Compress out the eliminated args */
    j    = 0;
    i    = 0;
    while (j < argc) {
	while (argv[j] == 0 && j < argc) j++;
	if (j < argc) argv[i++] = argv[j++];
	}
    /* Back off the last value if it is null */
    if (!argv[i-1]) i--;
    argc = i;
    return 0;
}

/* Returns index of name */
int CmdLine::FindArg( const char *name )
{
    int  i;

    for (i=0; i<argc; i++) {
	if (strcmp( argv[i], name ) == 0) return i;
	}
    return -1;
}

int CmdLine::HasArg( const char *name )
{
    int idx;
    
    idx = FindArg( name );
    if (idx < 0) return 1;
    
    argv[idx]   = 0;
    SqueezeArg( );
    return 0;
}

int CmdLine::GetArg( const char *name, int *val )
{
    int idx;
    char *p;

    idx = FindArg( name );
    if (idx < 0) return 1;

    if (idx + 1 >= argc) {
	fprintf( stderr, "Missing value for argument\n" );
	return 1;
	}

    p = argv[idx+1];
    /* Check for hexidecimal value */
    if (((int)strlen(p) > 1) && p[0] == '0' && p[1] == 'x') {
	sscanf( p, "%i", val );
	}
    else {
	if ((int)strlen(p) > 1 && p[0] == '-' && p[1] >= 'A' && p[1] <= 'z') {
	    fprintf( stderr, "Missing value for argument\n" );	
	    return 1;
	    }	
	*val = atoi( p );
	}

    argv[idx]   = 0;
    argv[idx+1] = 0;
    SqueezeArg();
    return 0;
}

int CmdLine::GetArg( const char *name, char *string, int maxlen )
{
    int idx;

    idx = FindArg( name );
    if (idx < 0) return 1;

    if (idx + 1 >= argc) {
	fprintf( stderr, "Missing value for argument\n" );
	return 1;
	}

    strncpy( string, argv[idx+1], maxlen );
    argv[idx]   = 0;
    argv[idx+1] = 0;
    SqueezeArg();

    return 0;
}

/*@C
  GetArgPtr - Returns a pointer to the value of an argument

  Input Parameter:
. name - Name of argument

  Output Parameter:
. string - Pointer to argument value.  Null if 'name' not found.
@*/
int CmdLine::GetArgPtr( const char *name, const char **string )
{
    int idx;

    *string = 0;
    idx = FindArg( name );
    if (idx < 0) return 1;

    if (idx + 1 >= argc) {
	fprintf( stderr, "Missing value for argument\n" );
	return 1;
	}

    *string = argv[idx+1];
    argv[idx]   = 0;
    argv[idx+1] = 0;
    SqueezeArg();

    return 0;
}

int CmdLine::GetList( const char *name, int nitem, int *vals )
{
    int idx, i;

    idx = FindArg( name );
    if (idx < 0) return 1;

    /* Fail if there aren't enough values */
    if (idx + nitem + 1 > argc) {
	fprintf( stderr, "Not enough values for vector of integers\n" );
	return 1;
	}

    for (i=0; i<nitem; i++) {
	vals[i] = atoi( argv[idx+i+1] );
	argv[idx+i+1] = 0;
	}
    argv[idx]   = 0;
    SqueezeArg();
 
    return 0;
}

int CmdLine::GetList( const char *name, int nitem, char **vals )
{
	// Not implemented yet
	return 1;
}

int CmdLine::NextArg( const char **val )
{
    if (cnt >= argc) {
	*val = 0;
	return 1;
	}
    *val = argv[cnt++];
    return 0;
}
CmdLine::~CmdLine()
{
    argc = 0;
    argv = 0;
}

