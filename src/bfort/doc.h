/*   Include file for the document preparation system.             */

#ifndef _DOCTEXT
#include <stdio.h>
/* #include <stdlib.h> */
#include <ctype.h>

#define MAX_FILE_SIZE    1024
#define MAX_ROUTINE_NAME 64
#define MAX_LINE         512

#define MATCH_CHARS "@DFHMNI"
#define MATCH_ALL_CHARS "@DFHIMN+"
#define ROUTINE     '@'
#define DESCRIPTION 'D'
#define FORTRAN     'F'
#define INTERNAL    '+'
#define HELP        'H'
#define MACRO       'M'
#define INCLUDE     'I'
#define TEXT        'N'

/* Special lead character types (only in the first column in a comment) */
#define ARGUMENT    '.'
#define VERBATIM    '$'


/* Redefinitions of routines to system/txt.c */
#define FindNextToken(fd,token,nsp) SYTxtFindNextToken(fd,token,1024,nsp)
#define SkipWhite( fd ) SYTxtSkipWhite( fd )
#define SkipLine( fd )  SYTxtDiscardToEndOfLine( fd )
#define SkipOver( fd, str ) SYTxtSkipOver( fd, str )
#define GetLine( fd, buf ) SYTxtGetLine( fd, buf, 1024 )
#define TrimLine( s )      SYTxtTrimLine( s )
#define UpperCase( s )     SYTxtUpperCase( s )
#define LowerCase( s )     SYTxtLowerCase( s )
#define LastChangeToFile(fname,date) \
    SYLastChangeToFile( fname, date, (void*)0 )

#ifdef FOO
/* This structure contains the output routines; to add a new output format,
   specify the routines for these output actions */
typedef struct {
    void    (*outbof)(),       /* Output beginning of file */
	    (*outchar)(),      /* Output a character */
            (*outraw)(),       /* Output a character without processing */
            (*outstring)(),    /* Output a string */
            (*outblank)(),     /* Output a blank */
            (*outlocation)(),  /* Output "location" information */
            (*outtitle)(),     /* Output "title" information */
            (*outsection)(),   /* Output a new section */
            (*outlinebreak)(), /* Output a mandatory line break */
            (*outargbegin)(),  /* Output start of an argument list */
            (*outargdefn)(),   /* Output an argument definition */
            (*outargend)(),    /* Output end of an argument definition */
            (*outverbatimbegin)(), /* Output begin of verbatim mode */
            (*outverbatimend)(),   /* Output end of verbatim mode */
            (*outendpage)(),   /* Output end of "page" */
            (*outendpar)(),    /* Output end of paragraph */
            (*outstartem)(),   /* Output begining of emphasis */
            (*outendem)(),     /* Output end of emphasis */
            (*outstarttt)(),   /* Output begining of tt */
            (*outendtt)(),     /* Output end of tt */
            (*outpicture)(),   /* Output a postscript picture */
            (*outstartcaption)(),    /* Output the beginning of a caption */
            (*outendcaption)();      /* Output the end of a caption */
    } DocOutput;

/* These provide access to the output routines if "outfcn" is set to
   a DocOutput structure */
#define OutputBOF( fout, dirname ) (*(outfcn)->outbof)( fout, dirname )
#define OutputChar( fout, c ) (*(outfcn)->outchar)( fout, c )
#define OutputRawChar( fout, c ) (*(outfcn)->outraw)( fout, c )
#define OutputString( fout, s ) (*(outfcn)->outstring)( fout, s )
#define OutputBlank( fout ) (*(outfcn)->outblank)( fout )
#define OutputLocation( fout, s ) (*(outfcn)->outlocation)( fout, s )
#define OutputTitle( fout, name, level, date, heading ) \
	(*(outfcn)->outtitle)( fout, name, level, date, heading )
#define OutputSection( fout, name ) (*(outfcn)->outsection)( fout, name )
#define OutputLineBreak( lastnl, fout ) \
	(*(outfcn)->outlinebreak)( lastnl, fout )
#define OutputArgBegin( fout ) (*(outfcn)->outargbegin)( fout )
#define OutputArgDefn( fin, fout ) (*(outfcn)->outargdefn)( fin, fout )
#define OutputArgEnd( fout ) (*(outfcn)->outargend)( fout )
#define OutputVerbatimBegin( fout ) (*(outfcn)->outverbatimbegin)( fout )
#define OutputVerbatimEnd( fout ) (*(outfcn)->outverbatimend)( fout )
#define OutputEndPage( fout )   (*(outfcn)->outendpage)( fout )
#define OutputEndPar( fout )    (*(outfcn)->outendpar)( fout )

#define OutputStartEm( fout )   (*(outfcn)->outstartem)( fout )
#define OutputEndEm( fout )     (*(outfcn)->outendem)( fout )
#define OutputStartTt( fout )   (*(outfcn)->outstarttt)( fout )
#define OutputEndTt( fout )     (*(outfcn)->outendtt)( fout )

#define OutputPicture( fout, name ) (*(outfcn)->outpicture)( fout, name )
#define OutputStartCaption( fout )  (*(outfcn)->outstartcaption)( fout )
#define OutputEndCaption( fout )    (*(outfcn)->outendcaption)( fout )


/* Known output formats */
extern DocOutput  *CreateOutputLaTeX( ), *CreateOutputMan(),
	          *CreateOutputHTML();

#endif

extern int DocGetChar ( FILE * );
extern void DocUnGetChar ( char, FILE * );
extern void DocSetPushback ( char * );
extern void ResetLineNo ( void );
extern int GetLineNo ( void );
extern char GetSubClass ( int * );
extern int GetIsX11Routine ( void );
extern void FindToken ( FILE *, char * );
extern int FoundLeader ( FILE *, char *, char * );
extern int MatchLeader ( FILE *, const char *, char * );
extern void CopyIncludeName ( FILE *, char * );
extern void ExpandFileName ( char *, int );
extern int GetToolsDirLength ( void );
extern int MatchTokens ( char *, char * );

#endif
