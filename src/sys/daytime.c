#include "sowing.h"

#include <sys/types.h>       /*I <sys/types.h> I*/
#if defined(WIN32) || defined(HAVE_STRING_H)
#include <string.h>
#endif
/*
   Here's an unpleasent fact.  On Intel systems, include-ipsc/sys/types.h 
   contains "typedef long time_t" and
   include/time.h contains "typedef long int time_t".  We can fix this by 
   defining __TIME_T after types.h is included.
 */
#if defined(intelnx) && !defined(intelparagon) && !defined(__TIME_T)
#define __TIME_T
#endif
#include <time.h>
#if !defined(__MSDOS__) && !defined(WIN32)
#include <sys/time.h>        /*I <sys/time.h>  I*/
#else
struct timeval { long tv_sec, tv_usec; };
#endif

/*@
    SYGetDayTime - Get the time of day in timeval format.

    Output parameter:
.   tp - timeval (seconds since 00:00 GMT, January 1, 1970)

    Note:
    On system V systems, this has a resolution of seconds.
    Other systems (BSD-like, for example) may have finer
    resolution
@*/
void SYGetDayTime( struct timeval *tp )
{
#if (defined(intelnx) && !defined(paragon)) || defined(__MSDOS__) || defined(WIN32)
time_t tval;

tval        = time( (time_t *)0 );
tp->tv_sec  = tval;
tp->tv_usec = 0;
#else 
gettimeofday( tp, (struct timezone *)0 );
#endif
}

/*@
    SYGetDate - Gets the date and time as a string

    Output Parameter:
.   str - string to hold the result.  Must be long enough (> 26 characters)
@*/
void SYGetDate( str )
char *str;
{
time_t clock;

clock = time( (time_t *)0 );
strcpy( str, ctime( &clock ) );
/* Remove the trailing newline */
str[strlen(str)-1] = '\0';
}
