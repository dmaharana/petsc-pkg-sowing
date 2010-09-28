#ifndef _SOWINGTXT
#define _SOWINGTXT

#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif
#ifdef __STDC__
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

extern int SYTxtGetLine ANSI_ARGS(( FILE *, char *, int ));
extern int SYTxtSkipBlankLines ANSI_ARGS(( FILE *, char *, int ));
extern int SYTxtLineIsBlank ANSI_ARGS(( char *, int ));
extern int SYTxtFindNextToken ANSI_ARGS(( FILE *, char *, int, int * ));
extern int SYTxtGetChar ANSI_ARGS(( FILE * ));
extern int SYTxtFindNextANToken ANSI_ARGS(( FILE *, char *, int, int * ));
extern int SYTxtFindNextAToken ANSI_ARGS(( FILE *, char *, int, int * ));
extern int SYTxtSkipWhite ANSI_ARGS(( FILE * ));
extern int SYTxtSkipOver ANSI_ARGS(( FILE *, char * ));
extern void SYTxtDiscardToEndOfLine ANSI_ARGS(( FILE * ));
extern int SYTxtTrimLine ANSI_ARGS(( char * ));
extern void SYTxtUpperCase ANSI_ARGS(( char * ));
extern void SYTxtLowerCase ANSI_ARGS(( char * ));

#endif
