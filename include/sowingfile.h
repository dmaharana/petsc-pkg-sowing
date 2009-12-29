#ifndef _SOWINGFILE
#define _SOWINGFILE

#ifdef HAVE_TIME_H
#include <time.h>
#endif

extern void SYGetFullPath( char *, char *, int );
extern void SYGetRelativePath( char *, char *, int );
extern void SYGetUserName( char *, int );
extern void SYGetHostName( char *, int );
extern int SYGetFileFromPath( char *, char *, char *, char *, char );
extern int SYGetFileFromEnvironment( char *, char *, char *, char *, char );
extern int SYFindFileWithExtension( char *, const char * );
extern FILE *SYOpenWritableFile( char *, char *, char *, char *, int );
extern void SYGetwd( char *, int );
extern char *SYGetRealpath( char *, char * );
extern void SYRemoveHomeDir( char * );
extern int SYIsDirectory( char * );
extern void SYMakeAllDirs( char *, int );
extern int SYIsMachineHost( char * );
extern int SYFileNewer( char *, char * );
extern int SYiFileExists( char *, char );
extern void SYLastChangeToFile( char *fname, char *date, struct tm *ltm );
/* extern void SYLastChangeToFile( char *, char *, struct tm * ); */
extern FILE *SYfopenLock( char *, char * );
extern void SYfcloseLock( FILE * );

#endif
