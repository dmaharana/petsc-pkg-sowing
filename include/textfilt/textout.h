/*
 * This file defines the text output classes; these are simple subsets of
 * popular output formats needed by the various filters.
 */

#ifndef TEXTOUT
#define TEXTOUT

#include "outstream.h"
#include "instream.h"
#include "srlist.h"
#include "errhand.h"

#define TEXT_I_IGNORE -2

#define MAX_TEXT_REGNUM 10
#define MAX_TEXT_REGISTER 64

/* Base class */
class TextOut {

    int    MatchCommand( const char *, int, SrEntry ** );
    char * GetDQtext( char *, char *, int );
    int    OutString( const char * );
    int    OutStringUC( const char * );

    public:
    char      *lfont;
    char      font_str[32];
    const char *mode;
    char      mode_str[32];
    char      *nl;
    char      newline_str[32];
    int       last_was_nl;
    int       last_was_par;
    char      newline_onoutput[3];
    SrList    *userops;
    OutStream *out;
    int       debug_flag;
    TextOut   *next;

    /* Text Registers */
    char      cmdreg[MAX_TEXT_REGNUM][MAX_TEXT_REGISTER];

    // Basic control 
    virtual int SetDebug( int );
    virtual const char *SetMode( const char * );
    virtual int SetRegisterValue( int, const char * );
    virtual int SetOutstream( OutStream * );
    virtual ~TextOut();
    virtual int Flush();
    virtual void SetNewlineString( const char * );

    // Error handler.  Default is abort.  
    ErrHand  *err;

    // Get commands 
    virtual int ReadCommands( InStream * );

    // Common command generator 
    virtual int PutCommand( char *, char *, char *, char *, char *, int );

    // Basic text 
    virtual int PutChar( const char );
    virtual int PutToken( int, const char * );
    virtual int PutQuoted( int, const char * );
    virtual int UpdateNL( int );
    virtual int PutNewline( );

    /* Special output */
    virtual int PutOp( const char * );
    virtual int PutOp( const char *, char *, char *, int );
    virtual int PutOp( const char *, char *, char *, char *, char * );
    virtual int PutOp( const char *, char * );
    virtual int PutOp( const char *, char *, int );
    virtual int PutOpError( const char *, const char * );
    virtual int HasOp( const char * );

    /* Miscellaneous */
    virtual int Debug( int );
    };

class TextOutHTML : public TextOut {
    
    int Setup( );

    public:
    TextOutHTML( OutStream * );
    TextOutHTML( );
    int PutChar( const char );
    int PutToken( int, const char * );
    };

class TextOutTeX : public TextOut {
    
    int Setup( );

    public:
    TextOutTeX( OutStream * );
    TextOutTeX( );
    int PutChar( const char );
    int PutToken( int, const char * );
    };

class TextOutNroff : public TextOut {
    
    int Setup( );

    public:
    TextOutNroff( OutStream * );
    TextOutNroff( );
    int PutChar( const char );
    int PutToken( int, const char * );
    };

class TextOutStrm : public TextOut {
    public:
    TextOutStrm( OutStream * );
    ~TextOutStrm();
    };
    
#endif
