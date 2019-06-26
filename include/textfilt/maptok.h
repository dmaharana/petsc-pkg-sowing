#ifndef MAPTOK
#define MAPTOK

#include "outstream.h"
#include "instream.h"
#include "textout.h"
#include "srlist.h"

class OutStreamMap : public OutStream {

 protected:
    char breaktable[256];
    char *activetok, *position, *lctok;
    int  curlen, maxlen;
    char squote, equote;
    SrList *maptable;
    int print_matched;
    int ignore_case;
    void Setup( int );
    void FlushTokBuf( void );
    void toLower(char *);

 public:
    OutStreamMap( OutStream * );
    OutStreamMap( OutStream *, int );
    OutStreamMap( OutStream *, int, int );
    OutStreamMap( int );
    ~OutStreamMap();
    int ReadMap( InStream * );
    int ReadMap( InStream *, int );
    int ReadMap( InStream *, int, int );
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

 protected:
    char breaktable[256];
    char *activetok, *position,*lctok;
    int  curlen, maxlen;
    char squote, equote;
    SrList *maptable;
    int print_matched;
    int ignore_case;
    void Setup( int );
    void FlushTokBuf( void );
    void toLower(char *);

 public:
    int  debug;
    TextOutMap( TextOut *, int );
    TextOutMap( TextOut * );
    TextOutMap( );
    ~TextOutMap();
    int SetBreakChar( char, int );
    int PutChar( const char );
    int PutToken( int, const char * );
    int PutQuoted( int, const char * );
    int PutNewline( void );
    virtual int PutLink( const char *, SrEntry * );
    int SetRegisterValue( int, const char * );
    int ReadMap( InStream * );
    int ReadMap( InStream *, int );
    int ReadMap( InStream *, int, int );
    };

#endif
