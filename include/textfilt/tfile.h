#ifndef TFILE
#define TFILE

#include <time.h>   /*I <time.h> I*/

void SYLastChangeToFile( const char *, char *, struct tm * );
char *SYGetRealpath( const char *, char * );

#endif
