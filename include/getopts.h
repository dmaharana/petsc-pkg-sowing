#ifndef _GETOPTS
#define _GETOPTS

#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif
#ifdef __STDC__
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

void SYArgSqueeze ANSI_ARGS(( int *, char ** ));
int SYArgFindName ANSI_ARGS(( int, char **, char * ));
int SYArgGetInt ANSI_ARGS(( int *, char **, int, char *, int * ));
int SYArgGetDouble ANSI_ARGS(( int *, char **, int, char *, double * ));
int SYArgGetString ANSI_ARGS(( int *, char **, int, char *, char *, int ));
int SYArgHasName ANSI_ARGS(( int *, char **, int, char * ));
int SYArgGetIntVec ANSI_ARGS(( int *, char **, int, char *, int, int * ));
int SYArgGetIntList ANSI_ARGS(( int *, char **, int, char *, int, int * ));

#endif

