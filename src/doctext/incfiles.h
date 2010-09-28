#ifndef INCFILES
#define INCFILES

int SetIncludeFile( const char *path );
int OutputIncludeInfo( TextOut *textout );
int SaveIncludeFile( InStream *ins, char *matchstring );
int ClearIncludeFile( );

#endif
