#ifndef MAPTOK
#define MAPTOK

#include "outstream.h"
#include "instream.h"
#include "textout.h"
#include "srlist.h"

class OutStreamMap : public OutStream {
    
    char breaktable[256];
    char *activetok, *position;
    int  curlen, maxlen;
    char squote, equote;
    SrList *maptable;
    void Setup( int );
    void FlushTokBuf( void );

    public:
    OutStreamMap( OutStream * );
    OutStreamMap( OutStream *, int );
    OutStreamMap( int );
    ~OutStreamMap();
    int ReadMap( InStream * );
    int PutToken( int, const char * );
    int PutQuoted( int, const char * );
    int PutChar( const char );
    int PutLink( const char *, SrEntry * );
    int SetBreakChar( char, int );
    int SetBreakChars( const char *, int );
    int SetQuoteChars( char, char );
    };

// We also need a TextOut version
class TextOutMap : public TextOut {
    
    char breaktable[256];
    char *activetok, *position;
    int  curlen, maxlen;
    char squote, equote;
    SrList *maptable;
    void Setup( int );
    void FlushTokBuf( void );

    public:
    int  debug;
    TextOutMap( TextOut * );
    TextOutMap( );
    ~TextOutMap();
    int PutChar( const char );
    int PutToken( int, const char * );
    int PutQuoted( int, const char * );
    int PutNewline( void );
    int PutLink( const char *, SrEntry * );
    int SetRegisterValue( int, const char * );
    int ReadMap( InStream * );
    };

#endif
