#ifndef _SOWINGFILE
#define _SOWINGFILE

extern void SYGetFullPath( char *, char *, int );
extern void SYGetRelativePath( char *, char *, int );
extern void SYGetUserName( char *, int );
extern void SYGetHostName( char *, int );
extern int SYGetFileFromPath( char *, char *, char *, char *, char );
extern int SYGetFileFromEnvironment( char *, char *, char *, char *, char );
extern FILE *SYOpenWritableFile( char *, char *, char *, char *, int );
extern void SYGetwd( char *, int );
extern char *SYGetRealpath( char *, char * );
extern void SYRemoveHomeDir( char * );
extern int SYIsDirectory( char * );
extern void SYMakeAllDirs( char *, int );
extern int SYIsMachineHost( char * );
extern int SYFileNewer( char *, char * );
extern int SYiFileExists( char *, char );
/* extern void SYLastChangeToFile( char *, char *, struct tm * ); */

#endif
