#ifndef DOTFMT
#define DOTFMT

int ProcessArgFmt( char, InStream *, TextOut *, int * );
int ProcessVerbatimFmt( InStream *, TextOut *, int * );
int ProcessNameFmt( InStream *, TextOut *, int * );
int ProcessVerbatimBlockFmt( InStream *, TextOut *, int * );
int ProcessCaption( InStream *, TextOut *, int * );
int ProcessPicture( InStream *, TextOut *, int * );
int ProcessKeywords( InStream *, TextOut *, int * );
int ProcessSeeAlso( InStream *, TextOut *, int * );
int ProcessDotFmt( char, InStream *, TextOut *, int * );
int ProcessUserCmd( InStream *, TextOut *, int * );

#endif
