#ifndef _SOWINGFILE
#define _SOWINGFILE

#ifdef HAVE_TIME_H
#include <time.h>
#endif

extern void SYGetFullPath( const char *, char *, int );
extern void SYGetRelativePath( const char *, char *, int );
extern void SYGetUserName( char *, int );
extern void SYGetHostName( char *, int );
extern int SYGetFileFromPath( const char *, const char *, const char *,
			      char *, char );
extern int SYGetFileFromEnvironment( const char *, const char *, const char *,
				     char *, char );
extern int SYFindFileWithExtension( char *, const char * );
extern FILE *SYOpenWritableFile( const char *, const char *, const char *,
				 char *, int );
extern void SYGetwd( char *, int );
extern char *SYGetRealpath( const char *, char * );
extern void SYRemoveHomeDir( char * );
extern int SYIsDirectory( const char * );
extern void SYMakeAllDirs( const char *, int );
extern int SYIsMachineHost( const char * );
extern int SYFileNewer( char *, char * );
extern int SYiFileExists( const char *, char );
extern void SYLastChangeToFile( const char *fname, char *date, struct tm *ltm );
/* extern void SYLastChangeToFile( char *, char *, struct tm * ); */
extern FILE *SYfopenLock( char *, char * );
extern void SYfcloseLock( FILE * );
extern int SYGetFileFromPathEnv( const char *, const char *, const char *,
				 const char *, char *, char );

#endif
