/*
 * InStream - Manages a input stream which could come from a file or memory
 * or another output stream ...
 */

#ifndef INSTREAM
#define INSTREAM

extern int InstreamDebugPaths( int );

/*
 * InStream is the base class.  The derived classes for different kinds
 * of InStreams 
 */
class InStream {
    public:
    char     breaktable[256];
    char     squote, equote;
    int      status; /* 0 for ok, errno for error, -1 for other error */

    InStream *next;       /* Next stream in chain */
    int ResetTables( void );
    virtual ~InStream();
    virtual int GetChar( char * );
    virtual int GetToken( int, char *, int * );
    virtual int GetLine( char *, int );
    virtual int UngetChar( char );
    virtual int UngetToken( char * );
    virtual int SkipLine( void );
    virtual int GetLoc( long * );
    virtual int SetLoc( long );
    virtual int GetSourceName( char *, int, int * );
            int SetBreakChar( char, int );
            int SetBreakChars( char *, int );
            int SetQuoteChars( char, char );
    virtual int GetLineNum( void );
    virtual int Close( void );
    };

/*
 * Derived classes.  
 */
#include <stdio.h>
class InStreamFile : public InStream {
    FILE *fp;          /* Actual file pointer */
    int  linecnt;      /* Current input line */
    int  colcnt;       /* Current position in line (column) */
    int  expand_tab;   /* whether to expand tabs to every 8 columns */
    int  nblanks;      /* number of pushed-back blanks */
    char *fname;       /* Name of file */
    int  didunget;

    public:
    ~InStreamFile();
    InStreamFile( const char *, const char *, const char *, const char * );
    InStreamFile( const char *, const char * );
    InStreamFile( );
    int GetChar( char * );
    int UngetChar( char );
    int UngetToken( char * ); 
    int GetLoc( long * );
    int SetLoc( long ); 
    int GetSourceName( char *, int, int * );
    int GetLineNum( void );
    int Close( void );

    void SetExpandTab( int flag ) { expand_tab = flag; }
    };

class InStreamBuf : public InStream {
    char *buffer, *position;
    int  curlen, maxlen;

    public:
    ~InStreamBuf();
    InStreamBuf( int, InStream * );
    int GetChar( char * );
    int UngetChar( char );
/*    int GetLoc( long * );
    int SetLoc( long ); */
    int Close( );
    };

class InStreamStack : public InStream {

    public:
    InStreamStack();
    ~InStreamStack();
    int GetChar( char * );
    int Push( InStream *ins );
    int Pop();
    };

/*
 * Breaktable information
 */
#define BREAK_OTHER 0
#define BREAK_SPACE 1
#define BREAK_ALPHA 2
#define BREAK_DIGIT 3
#endif
