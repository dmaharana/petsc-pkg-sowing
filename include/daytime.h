#ifndef _DAYTIME
#define _DAYTIME

#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif
#ifdef __STDC__
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

/* extern void SYGetDayTime ANSI_ARGS(( struct timeval * )); */
extern void SYGetDate ANSI_ARGS(( char * ));
#endif

